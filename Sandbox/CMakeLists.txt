cmake_minimum_required(VERSION 3.20)

project(Sandbox)

add_executable(Sandbox Sandbox.cpp)

target_link_libraries(Sandbox PRIVATE Yume)

find_program(SLANGC_EXECUTABLE slangc)
if(NOT SLANGC_EXECUTABLE)
  message(FATAL_ERROR "Could not find slangc")
endif()

include(${CMAKE_SOURCE_DIR}/Yume/cmake/CompileShaders.cmake)
file(GLOB SANDBOX_SHADERS "${CMAKE_CURRENT_LIST_DIR}/shaders/*.slang")

add_slang_shader_target(SandboxShaders SOURCES ${SANDBOX_SHADERS})
add_dependencies(Sandbox SandboxShaders)