cmake_minimum_required(VERSION 3.23)
project(f_15_data_visualization)

set(CMAKE_CXX_STANDARD 14)

include_directories("Capstone Solution C++")

add_executable(f_15_data_visualization
        "Capstone Solution C++/main.cpp"
        "Capstone Solution C++/packet.cpp" "Capstone Solution C++/test_stuff.cpp" "Capstone Solution C++/test_stuff.h" "Capstone Solution C++/packet.h" "Capstone Solution C++/packet_factory.h" "Capstone Solution C++/packet_factory.cpp" "Capstone Solution C++/ChanSpecData.cpp" "Capstone Solution C++/ChanSpecData.h" "Capstone Solution C++/IntraPackHeader.cpp" "Capstone Solution C++/IntraPackHeader.h" "Capstone Solution C++/Words.cpp" "Capstone Solution C++/Words.h" "Capstone Solution C++/Messages.cpp" "Capstone Solution C++/Messages.h")
