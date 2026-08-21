#include "Tintin_reporter.hpp"
#include <iomanip>
#include <ctime>
#include <ios>
#include <iostream>
#include <sstream>

Tintin_reporter::Tintin_reporter() : _log_file("/var/log/matt_daemon/matt_daemon.log", std::ios::app){
    _reporters[LogType::Info] = "INFO";
    _reporters[LogType::Log] = "LOG";
    _reporters[LogType::Error] = "ERROR";
}

std::string Tintin_reporter::getTimeStamp() const {
    std::time_t now = std::time(nullptr);
    std::tm* time = std::localtime(&now);
    std::ostringstream timestamp;
    timestamp << "["
              << std::setfill('0') << std::setw(2) << time->tm_mday
              << "/"
              << std::setfill('0') << std::setw(2) << (time->tm_mon + 1)
              << "/"
              << (time->tm_year + 1900)
              << "-"
              << std::setfill('0') << std::setw(2) << time->tm_hour
              << ":"
              << std::setfill('0') << std::setw(2) << time->tm_min
              << ":"
              << std::setfill('0') << std::setw(2) << time->tm_sec
              << "]";
    return timestamp.str();
}


Tintin_reporter& Tintin_reporter::getInstance() {
    static Tintin_reporter instance;
    return instance;
}

void Tintin_reporter::log(LogType type, std::string_view msg) {
    if (_log_file) {
        _log_file << getTimeStamp() << " [" << _reporters[type] << "] : " << msg << '\n';
        _log_file.flush();
        return;
    }
    std::cerr << getTimeStamp() << " [" << _reporters[type] << "] - " << msg << '\n';
}