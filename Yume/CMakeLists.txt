cmake_minimum_required(VERSION 3.20)
project(Yume VERSION 1.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Vulkan (System SDK) ----
set(MIN_VULKAN_VERSION 1.4)
find_package(Vulkan ${MIN_VULKAN_VERSION} REQUIRED)

message(STATUS "Found Vulkan SDK: ${Vulkan_VERSION}")

# ---- GLM (Auto Download, Version Locked) ----
include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)

file(GLOB_RECURSE ENGINE_SOURCES src/*.cpp)

add_library(Yume STATIC ${ENGINE_SOURCES})

target_include_directories(Yume
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${glfw_SOURCE_DIR}/include
)

target_link_libraries(Yume
    PUBLIC
        Vulkan::Vulkan
    PRIVATE
        glm::glm
        glfw
)

target_compile_definitions(Yume
    PRIVATE
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
        GLFW_INCLUDE_VULKAN
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")