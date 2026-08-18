#pragma once

#include <csignal>

class Daemon {
    private:
        static const char* LOCK_PATH;

        int  _lockFd;  
        static volatile sig_atomic_t _running;
        static volatile sig_atomic_t _signal;

        bool createLock();
        bool createFork();
        bool createSession();
        bool redirectStandardFiles();
        int daemonize();
        bool setupSignals();

        static void signalHandler(int signal);
    public:
        Daemon();
        ~Daemon();

        bool start();
        static void stop();

        static bool isRunning();
};