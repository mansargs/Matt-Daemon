#include "SignalHandler.hpp"
#include <csignal>

volatile sig_atomic_t SignalHandler::_signal = 0;
std::unordered_map<int, std::string> SignalHandler::dictionary;

SignalHandler::SignalHandler() {
    dictionary[SIGINT] = "SIGINT";
    dictionary[SIGTERM] = "SIGTERM";
    dictionary[SIGHUP] = "SIGHUP";
    dictionary[SIGQUIT] = "SIGQUIT";
}

void SignalHandler::signalHandler(int signal) {
    _signal = signal;
}

bool SignalHandler::setupSignals() {
    struct sigaction sa{};

    sa.sa_handler = SignalHandler::signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGTERM, &sa, nullptr) == -1)
        return false;
    if (sigaction(SIGINT, &sa, nullptr) == -1)
        return false;
    if (sigaction(SIGHUP, &sa, nullptr) == -1)
        return false;
    if (sigaction(SIGQUIT, &sa, nullptr) == -1)
        return false;
    return true;
}

std::string SignalHandler::getSignalName() {
    auto it = dictionary.find(static_cast<int>(_signal));
    if (it != dictionary.end())
        return it->second;
    return std::string("UNKNOWN");
}

volatile sig_atomic_t SignalHandler::getSignal() {
    return _signal;
}