cmake_minimum_required(VERSION 3.20)

project(Matt_daemon
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(Matt_daemon
    src/main.cpp
    src/Tintin_reporter.cpp
    src/Client.cpp
    src/Daemon.cpp
    src/Server.cpp
    src/SignalHandler.cpp
)

target_include_directories(Matt_daemon PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)