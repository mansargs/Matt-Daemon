#pragma once

#include <string>
#include <optional>

class Client {
    private:
        int _fd;
        std::string _buffer;
    public:
       explicit Client(int fd);
       ~Client();

       int getFd() const;

       void append(const char* data, std::size_t size);
       std::optional<std::string> getMessage();
       void clearBuffer();
};