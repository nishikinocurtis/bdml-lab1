cmake_minimum_required(VERSION 3.24)
project(lab1)

set(CMAKE_CXX_STANDARD 11)

add_executable(lab_server main.cpp handlers.cpp handlers.h)
