cmake_minimum_required(VERSION 3.15)
Project(JohnK VERSION 1.0.3)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(JUCE)
add_subdirectory(src bin)
add_subdirectory(cmake)
add_subdirectory(test)
add_subdirectory(extern)

