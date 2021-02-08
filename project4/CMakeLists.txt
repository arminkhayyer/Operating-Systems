cmake_minimum_required(VERSION 3.15)
project(project4 C)

set(CMAKE_C_STANDARD 99)

add_executable(project4 fsysdriver.c diskSimulator.c cpmfsys.c)