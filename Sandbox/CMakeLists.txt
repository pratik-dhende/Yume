cmake_minimum_required(VERSION 3.20)

project(Sandbox)

add_executable(Sandbox main.cpp)

target_link_libraries(Sandbox PRIVATE Yume)