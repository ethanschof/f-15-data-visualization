cmake_minimum_required(VERSION 3.23)
project(f_15_data_visualization)

set(CMAKE_CXX_STANDARD 14)

include_directories("Capstone Solution C++")

add_executable(f_15_data_visualization
        "Capstone Solution C++/1553helper.hpp"
        "Capstone Solution C++/ch10.hpp"
        "Capstone Solution C++/main.cpp"
        "Capstone Solution C++/packet.hpp")
