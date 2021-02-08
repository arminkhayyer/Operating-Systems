cmake_minimum_required(VERSION 3.15)
project(project3 C)

set(CMAKE_C_STANDARD 99)
add_executable(project3 AUbatch.c QueueHeader.h)
add_executable(process process.c)