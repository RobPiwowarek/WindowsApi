cmake_minimum_required(VERSION 3.8)
project(windowsapi)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "-static")

set(SOURCE_FILES right.cpp)
add_executable(windowsapi ${SOURCE_FILES})