cmake_minimum_required(VERSION 3.16)
project(dataforge LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(dataforge
    src/main.cpp
    src/kvstore.cpp
    src/journal.cpp
    src/http_server.cpp
    src/wal.cpp
)

target_include_directories(dataforge PRIVATE ${CMAKE_SOURCE_DIR}/src)