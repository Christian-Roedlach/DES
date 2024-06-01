cmake_minimum_required(VERSION 3.1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(slave_node CXX)

find_library(wiringPi_LIB wiringPi REQUIRED)

include_directories(./include/)

#add_executable (master ./src/roundtrip_master.c ./src/lib.c ./src/network.c)
#target_compile_options(master PRIVATE -Wall -Wextra -Wpedantic) #-Werror
#target_link_libraries (master)

add_executable (slave_node 
    src/slave_node_main.cpp 
    src/network.cpp
    src/timesync.cpp
    src/lib-timer.cpp
    src/signal_pin.cpp
    src/logging.cpp
)
target_compile_options(slave_node PRIVATE -Wall -Wextra -Wpedantic) #-Werror
target_link_libraries(slave_node ${wiringPi_LIB})

add_executable (test_master 
    src/master_testfunction.cpp 
)
target_compile_options(test_master PRIVATE -Wall -Wextra -Wpedantic) #-Werror
