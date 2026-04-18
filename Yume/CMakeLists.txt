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

find_package(slang CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

file(GLOB_RECURSE ENGINE_SOURCES src/*.cpp)

add_library(Yume STATIC ${ENGINE_SOURCES})

target_include_directories(Yume
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Yume
    PUBLIC
        Vulkan::Vulkan
        slang::slang
        glfw
        glm::glm-header-only
)

target_compile_definitions(Yume
    PRIVATE
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
        GLFW_INCLUDE_VULKAN
)