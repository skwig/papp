cmake_minimum_required(VERSION 3.8)
project(papp)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread")

add_executable(mato main.cpp)
add_executable(pekari pekari.cpp)