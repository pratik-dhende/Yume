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

# ---- Executable ----
add_executable(Yume
    Yume/src/ResourceManager.cpp
    Yume/src/main.cpp
)

# ---- Link Dependencies (Modern CMake) ----
target_link_libraries(Yume
    PRIVATE
        Vulkan::Vulkan
        glm::glm
)