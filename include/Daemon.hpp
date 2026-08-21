#pragma once

#include "SignalHandler.hpp"

class Daemon {
    private:
        static const char* LOCK_PATH;
        int  _lockFd;
        static bool _running;
        SignalHandler _signals;

       
        bool createFork();
        bool createSession();
        bool redirectStandardFiles();
        int  daemonize();
    public:
        Daemon();
        ~Daemon();

        bool start();
        bool createLock();

        static void stop();
        static bool isRunning();
};