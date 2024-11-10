cmake_minimum_required(VERSION 3.28)

project(castle)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(code/castle)
add_subdirectory(code/castle_asset_packer)
add_subdirectory(code/castle_mod_builder)
add_subdirectory(code/castle_common)
