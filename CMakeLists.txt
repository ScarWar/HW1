cmake_minimum_required(VERSION 3.6)
project(HW1)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        data_filter.c)

add_executable(HW1 ${SOURCE_FILES})