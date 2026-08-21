#pragma once

#include <cstddef>
#include <cstdint>
#include <poll.h>
#include <vector>

#include "Client.hpp"

class Server {
    private:
        static constexpr uint16_t kPort = 4242;
        static constexpr std::size_t kMaxClients = 3;
        static constexpr int kBacklog = 128;
    
        int _serverFd;
        std::vector<Client> _clients;
    
        bool setOptions();
        bool makeAsync();
        bool bindSocket();
        bool listenSocket();
        bool acceptClient();
    
        void setupPollFds(std::vector<pollfd>& fds) const;
        bool waitForEvents(std::vector<pollfd>& fds);
    
        void handleServerEvent(const pollfd& pfd);
        void handleClientEvents(const std::vector<pollfd>& fds);
    
        bool receiveClientData(std::size_t clientIndex);
        void processClientMessages(Client& client);
    
        void removeClient(std::size_t index);
        bool start();
    public:
        Server();
        ~Server();
    
        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;
    

        bool run();
};