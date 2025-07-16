cmake_minimum_required(VERSION 3.10.0)
project(http_server VERSION 0.1.0 LANGUAGES C)

set (CMAKE_C_COMPILER gcc)
set (CMAKE_C_FLAGS "-std=c2x -ggdb3 -Wall -pedantic -Werror -O2")

add_library(http_server_lib STATIC
			    src/server.c
			    src/utils.c
			    src/worker.c
			    src/http_gen.c
			    src/http_parser.c
			    src/str.c)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

target_include_directories(http_server_lib PUBLIC include/)

add_executable(http_server
    src/main.c)

target_link_libraries(http_server PUBLIC http_server_lib)

enable_testing()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(cmocka 1.1.0 REQUIRED)
include (${CMAKE_SOURCE_DIR}/cmake/AddCMockaTest.cmake)

add_subdirectory(tests)

