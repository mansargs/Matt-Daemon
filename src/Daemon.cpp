#include "Daemon.hpp"
#include "Tintin_reporter.hpp"
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/file.h>
#include <sys/stat.h>

const char* Daemon::LOCK_PATH = "/var/lock/matt_daemon.lock";
volatile sig_atomic_t Daemon::_running = 0;
volatile sig_atomic_t Daemon::_signal = 0;

Daemon::Daemon() : _lockFd(-1) {}

Daemon::~Daemon() {
    if (_lockFd != -1) {
        close(_lockFd);
        _lockFd = -1;
    }
}

bool Daemon::createLock() {
    _lockFd = open(LOCK_PATH, O_RDWR | O_CREAT, 0644);
    if (_lockFd == -1) {
        std::cerr << "Can't open :" << LOCK_PATH << std::endl;
        return false;
    }
    if (flock(_lockFd, LOCK_EX | LOCK_NB) == -1) {
        std::cerr << "Can't open :" << LOCK_PATH << std::endl;
        close(_lockFd);
        _lockFd = -1;
        return false;
    }
    return true;
}

bool Daemon::createSession() {
    if (setsid() == -1) {
        std::cerr << "Session creation failed\n";
        return false;
    }
    return true;
}

bool Daemon::redirectStandardFiles() {
    int fd = open("/dev/null", O_RDWR);
    if (fd < 0)
        return false;
    if (dup2(fd, STDIN_FILENO) == -1 || dup2(fd, STDOUT_FILENO)
        || dup2(fd, STDERR_FILENO)) {
            if (fd > STDERR_FILENO)
                close(fd);
        return false;
    }
    if (fd > STDERR_FILENO)
        close(fd);
    return true;
}

bool Daemon::createFork() {
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "fork() failed" << std::endl;
        return false;
    }
    if (pid > 0)
        _exit(EXIT_SUCCESS);
    return true;
}

void Daemon::stop() {
    _running = 0;
}

bool Daemon::start() {
    Tintin_reporter& logger = Tintin_reporter::getInstance();
    logger.log(LogType::Info, "Entering daemon mode.");
    if (!createLock())
        return false;
    if (!createFork())
        return false;
    if (!createSession())
        return false;
    if (!createFork())
        return false;
     if (chdir("/") == -1) {
        std::cerr << "chdir() failed" << std::endl;
        return false;
    }
    umask(0);
    if (!redirectStandardFiles())
        return false;
    if (!setupSignals())
        return false;
    _running = 1;
   logger.log(
        LogType::Info,
        "Matt_daemon: started. PID: " +
        std::to_string(static_cast<long long>(getpid()))
    );
    return true; 
}

void Daemon::signalHandler(int signal) {
    _signal = signal;
    _running = 0;
}

bool Daemon::isRunning() {
    return _running == 1;
}

bool Daemon::setupSignals() {
    struct sigaction sa{};

    sa.sa_handler = Daemon::signalHandler;
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