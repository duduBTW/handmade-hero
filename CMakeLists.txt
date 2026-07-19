cmake_minimum_required(VERSION 4.1)
project(handmade)

set(CMAKE_CXX_STANDARD 14)

include_directories(code)

add_executable(handmade
        code/handmade.cpp
        code/handmade.h
        code/win32_handmade.cpp)
