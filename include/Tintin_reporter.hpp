#pragma once

#include <cstdint>
#include <fstream>
#include <unordered_map>
#include <string_view>

enum class LogType : uint8_t {
    Info,
    Log,
    Error
};

class Tintin_reporter {
    private:
        std::ofstream _log_file;
        std::unordered_map<LogType, std::string_view> _reporters;    

        Tintin_reporter();
        std::string getTimeStamp() const;
    public:
        static Tintin_reporter& getInstance();
        void log(LogType type, std::string_view msg);
};