cmake_minimum_required(VERSION 3.9)
project(WindmillDetector)

find_package(OpenCV REQUIRED)
set(CMAKE_CXX_STANDARD 11)

add_executable(WindmillDetector Main.cpp WindmillDetector.cpp)
target_link_libraries(WindmillDetector ${OpenCV_LIBS})

