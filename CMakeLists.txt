cmake_minimum_required(VERSION 3.23)
project(f_15_data_visualization)

set(CMAKE_CXX_STANDARD 14)

include_directories("Capstone Solution C++")

add_executable(f_15_data_visualization
        "Capstone Solution C++/1553helper.hpp"
        "Capstone Solution C++/main.cpp"
        "Capstone Solution C++/packet.cpp" "Capstone Solution C++/test_stuff.cpp" "Capstone Solution C++/test_stuff.h" "Capstone Solution C++/packet.h")
