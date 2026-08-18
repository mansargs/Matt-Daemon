#include "Daemon.hpp"
#include "Server.hpp"
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <iostream>
#include <filesystem>


namespace fs = std::filesystem;;

int main() {
    uid_t eff_user = geteuid();
    if (eff_user != 0) {
        std::cerr << "Permission denied: root privileges required" << std::endl;
        return EPERM;
    }
    std::string log_dir = "/var/log/matt_daemon/";
    if (!fs::exists(log_dir)) {
        try {
            fs::create_directories(log_dir);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Failed to create log folder: " << e.what() << '\n';
            return 1;
        }
    }
    Server server;
    Daemon daemon;
    server.run();
    daemon.start();
};