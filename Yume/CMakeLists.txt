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
find_package(Stb REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)

file(GLOB_RECURSE ENGINE_SOURCES src/*.cpp)

message(STATUS "ENGINE_SOURCES:")
foreach(file ${ENGINE_SOURCES})
    message(STATUS "  ${file}")
endforeach()

add_library(Yume STATIC ${ENGINE_SOURCES})

target_include_directories(Yume
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${Stb_INCLUDE_DIR}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Yume
    PUBLIC
        Vulkan::Vulkan
        slang::slang
        glfw
        glm::glm-header-only
        tinyobjloader::tinyobjloader
)

target_compile_definitions(Yume
    PUBLIC
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
        GLFW_INCLUDE_VULKAN
        VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_ENABLE_EXPERIMENTAL
)

set_target_properties(Yume PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${PROJECT_NAME})