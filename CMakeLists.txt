cmake_minimum_required(VERSION 3.20)
project(Yume)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subfolders
add_subdirectory(Yume)
add_subdirectory(Sandbox)

include(${CMAKE_SOURCE_DIR}/Yume/cmake/utils.cmake)

get_target_property(SANDBOX_OUTPUT_DIR Sandbox RUNTIME_OUTPUT_DIRECTORY)
add_copy_yume_assets_target("${SANDBOX_OUTPUT_DIR}/assets" CopyYumeAssets)

add_dependencies(Sandbox CopyYumeAssets)