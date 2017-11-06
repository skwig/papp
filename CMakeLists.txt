cmake_minimum_required(VERSION 3.8)
project(papp)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread")

add_executable(mato main.cpp)
add_executable(pekari pekari.cpp)
add_executable(maliari maliari.cpp)
add_executable(volici volici.cpp)
add_executable(balici balici.cpp)
add_executable(generator_pismenok generator_pismenok.cpp)
add_executable(slachtici_a_poddani slachtici_a_poddani.cpp)
#add_executable(organizmy)
add_executable(lovci_a_zberaci lovci_a_zberaci.cpp)
#add_executable(generator_pismenok)
#add_executable(slachtici_a_poddani)
