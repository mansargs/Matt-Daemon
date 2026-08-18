#include "Server.hpp"

#include "Daemon.hpp"
#include "Tintin_reporter.hpp"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

Server::Server()
    : _serverFd(-1)
{
    _serverFd = socket(AF_INET, SOCK_STREAM, 0);

    if (_serverFd == -1)
        throw std::runtime_error("Server creation failed");
}

Server::~Server()
{
    for (Client& client : _clients)
    {
        // Client destructor closes its own fd.
    }

    if (_serverFd != -1)
        close(_serverFd);
}

bool Server::setOptions()
{
    int opt = 1;

    if (setsockopt(
            _serverFd,
            SOL_SOCKET,
            SO_REUSEADDR,
            &opt,
            sizeof(opt)) == -1)
    {
        return false;
    }

    return true;
}

bool Server::makeAsync()
{
    int flags = fcntl(_serverFd, F_GETFL, 0);

    if (flags == -1)
        return false;

    if (fcntl(_serverFd, F_SETFL, flags | O_NONBLOCK) == -1)
        return false;

    return true;
}

bool Server::bindSocket()
{
    sockaddr_in address{};

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(kPort);

    if (::bind(
            _serverFd,
            reinterpret_cast<sockaddr*>(&address),
            sizeof(address)) == -1)
    {
        return false;
    }

    return true;
}

bool Server::listenSocket()
{
    if (::listen(_serverFd, kBacklog) == -1)
        return false;

    return true;
}

bool Server::start()
{
    if (!setOptions())
        return false;

    if (!makeAsync())
        return false;

    if (!bindSocket())
        return false;

    if (!listenSocket())
        return false;

    return true;
}

void Server::setupPollFds(std::vector<pollfd>& fds) const
{
    pollfd serverPoll{};

    serverPoll.fd = _serverFd;
    serverPoll.events = POLLIN;

    fds.push_back(serverPoll);

    for (const Client& client : _clients)
    {
        pollfd clientPoll{};

        clientPoll.fd = client.getFd();
        clientPoll.events = POLLIN;

        fds.push_back(clientPoll);
    }
}

bool Server::waitForEvents(std::vector<pollfd>& fds)
{
    int ret = poll(
        fds.data(),
        static_cast<nfds_t>(fds.size()),
        -1
    );

    if (ret == -1)
    {
        /*
         * A signal can interrupt poll().
         *
         * The signal handler sets Daemon::_running to 0,
         * so we should leave the event loop.
         */
        if (errno == EINTR)
            return Daemon::isRunning();

        return false;
    }

    return true;
}

void Server::handleServerEvent(const pollfd& pfd)
{
    if (pfd.revents & POLLIN)
        acceptClient();
}

bool Server::acceptClient()
{
    Tintin_reporter& logger = Tintin_reporter::getInstance();

    /*
     * The project allows only 3 simultaneous clients.
     */
    if (_clients.size() >= kMaxClients)
    {
        /*
         * There is no room for another client.
         *
         * We still accept the connection and immediately close it.
         */
        int clientFd = accept(_serverFd, nullptr, nullptr);

        if (clientFd != -1)
            close(clientFd);

        return false;
    }

    int clientFd = accept(_serverFd, nullptr, nullptr);

    if (clientFd == -1)
    {
        /*
         * Because the server socket is non-blocking,
         * this can happen normally.
         */
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return true;

        logger.log(
            LogType::Error,
            "Matt_daemon: accept() failed."
        );

        return false;
    }

    /*
     * Make the client socket non-blocking too.
     */
    int flags = fcntl(clientFd, F_GETFL, 0);

    if (flags == -1)
    {
        close(clientFd);
        return false;
    }

    if (fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        close(clientFd);
        return false;
    }

    _clients.emplace_back(clientFd);

    logger.log(
        LogType::Info,
        "Matt_daemon: New client connected."
    );

    return true;
}

void Server::handleClientEvents(const std::vector<pollfd>& fds)
{
    /*
     * fds[0] is the server socket.
     *
     * Therefore:
     *
     * fds[1] -> _clients[0]
     * fds[2] -> _clients[1]
     * fds[3] -> _clients[2]
     */
    for (std::size_t i = 1; i < fds.size(); ++i)
    {
        std::size_t clientIndex = i - 1;

        if (fds[i].revents & POLLIN)
        {
            if (!receiveClientData(clientIndex))
                continue;
        }

        /*
         * Client closed the connection or an error occurred.
         */
        if (fds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
        {
            removeClient(clientIndex);
        }
    }
}

bool Server::receiveClientData(std::size_t clientIndex)
{
    if (clientIndex >= _clients.size())
        return false;

    Client& client = _clients[clientIndex];

    char buffer[1024];

    ssize_t bytes = recv(
        client.getFd(),
        buffer,
        sizeof(buffer),
        0
    );

    if (bytes > 0)
    {
        client.append(buffer, static_cast<std::size_t>(bytes));

        processClientMessages(client);

        return true;
    }

    /*
     * recv() returning 0 means the client closed
     * its connection.
     */
    if (bytes == 0)
    {
        removeClient(clientIndex);
        return false;
    }

    /*
     * Non-blocking socket can return EAGAIN/EWOULDBLOCK.
     */
    if (errno == EAGAIN || errno == EWOULDBLOCK)
        return true;

    Tintin_reporter& logger = Tintin_reporter::getInstance();

    logger.log(
        LogType::Error,
        "Matt_daemon: recv() failed."
    );

    removeClient(clientIndex);

    return false;
}

void Server::processClientMessages(Client& client)
{
    Tintin_reporter& logger = Tintin_reporter::getInstance();

    while (true)
    {
        std::optional<std::string> message = client.getMessage();

        if (!message.has_value())
            break;

        /*
         * Remove a possible '\r'.
         *
         * This is useful when clients send "\r\n".
         */
        if (!message->empty() && message->back() == '\r')
            message->pop_back();

        if (*message == "quit")
        {
            logger.log(
                LogType::Info,
                "Matt_daemon: Request quit."
            );

            /*
             * Request daemon shutdown.
             *
             * The signal handler is not involved here because
             * this is a normal client request.
             */
            Daemon::stop();
            return;
        }

        logger.log(
            LogType::Log,
            "Matt_daemon: User input: " + *message
        );
    }
}

void Server::removeClient(std::size_t index)
{
    if (index >= _clients.size())
        return;

    Tintin_reporter& logger = Tintin_reporter::getInstance();

    logger.log(
        LogType::Info,
        "Matt_daemon: Client disconnected."
    );

    /*
     * Client's destructor closes the socket.
     */
    _clients.erase(_clients.begin() + index);
}

void Server::run() {
    Tintin_reporter& logger = Tintin_reporter::getInstance();
    logger.log(
        LogType::Info,
        "Matt_daemon: Creating server."
    );
    if (!start()) {
        logger.log(
            LogType::Error,
            "Matt_daemon: Server creation failed."
        );

        return;
    }
    logger.log(
        LogType::Info,
        "Matt_daemon: Server created."
    );
    while (Daemon::isRunning()) {
        std::vector<pollfd> fds;
        setupPollFds(fds);
        if (!waitForEvents(fds))
            break;
        if (!Daemon::isRunning())
            break;
        handleServerEvent(fds[0]);
        if (!Daemon::isRunning())
            break;
        handleClientEvents(fds);
    }

    logger.log(
        LogType::Info,
        "Matt_daemon: Server stopped."
    );
}