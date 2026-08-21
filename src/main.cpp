#include "Server.hpp"
#include <exception>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;;

int main() {
    try {
        Server server;
        uid_t eff_user = geteuid();
        if (eff_user != 0) {
            std::cerr << "Permission denied: root privileges required" << std::endl;
            return 1;
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
        return server.run() == 0;
    } catch (const std::exception& e) {
        return 1;
    }
    
};