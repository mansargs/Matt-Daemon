#include "Client.hpp"
#include <cstddef>
#include <optional>
#include <unistd.h>

Client::Client(int fd) : _fd(fd) {}

Client::~Client() {
    if (_fd >= 0)
        close(_fd);
}

int Client::getFd() const {
    return _fd;
}

void Client::append(const char* data, size_t size) {
    _buffer.append(data, size);
}

std::optional<std::string> Client::getMessage() {
    std::size_t pos = _buffer.find('\n');
    if (pos == std::string::npos)
        return std::nullopt;
    std::string msg = _buffer.substr(0, pos);
    if (msg.empty())
        return std::nullopt;
    _buffer.erase(0, pos + 1);
    return std::optional<std::string>(msg);
}

void Client::clearBuffer() {
    _buffer.clear();
}