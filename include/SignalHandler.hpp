#pragma once

#include <unordered_map>
#include <string>
#include <csignal>

class SignalHandler  {
    private:
        static volatile sig_atomic_t _signal;

        static std::unordered_map<int, std::string> dictionary;
        static void signalHandler(int signal);
    public:
        SignalHandler();

        bool setupSignals();
        static std::string getSignalName();
        static volatile sig_atomic_t getSignal();
};