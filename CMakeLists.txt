cmake_minimum_required(VERSION 3.29)
project(simple_c C)

set(CMAKE_C_STANDARD 11)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(rgr rgr.c)

target_link_libraries(rgr ${CURSES_LIBRARIES})
