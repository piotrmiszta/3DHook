cmake_minimum_required(VERSION 3.10.0)
project(http_server VERSION 0.1.0 LANGUAGES C)

set (CMAKE_C_COMPILER clang-19)
set (CMAKE_C_FLAGS "-std=c2x -ggdb3 -Wall -pedantic -Werror")

add_executable(http_server
    src/main.c
    src/server.c)
target_include_directories(http_server PUBLIC include/)
