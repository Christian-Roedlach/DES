cmake_minimum_required(VERSION 3.1.0)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(roundtrip C)

include_directories(./include/)

add_executable (master ./src/roundtrip_master.c)
target_compile_options(master PRIVATE -Wall -Wextra) #-Werror
#target_link_libraries (master)
