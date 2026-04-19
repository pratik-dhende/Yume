cmake_minimum_required(VERSION 3.20)

project(Sandbox)

add_executable(Sandbox Sandbox.cpp)

target_link_libraries(Sandbox PRIVATE Yume)

set_target_properties(Sandbox PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${PROJECT_NAME})