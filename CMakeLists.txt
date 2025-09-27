cmake_minimum_required(VERSION 3.10)
project(MultiClientChat)

set(CMAKE_CXX_STANDARD 11)

add_executable(server server.cpp)
add_executable(client client.cpp)

find_package(Threads REQUIRED)
target_link_libraries(server PRIVATE Threads::Threads)
target_link_libraries(client PRIVATE Threads::Threads)
