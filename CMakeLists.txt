cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(castle)

add_subdirectory(code/castle)
add_subdirectory(code/castle_asset_packer)
add_subdirectory(code/castle_mod_builder)
add_subdirectory(code/castle_common)
