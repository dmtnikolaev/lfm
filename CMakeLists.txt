cmake_minimum_required(VERSION 3.0)
project(lfm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Wall -Wextra -Wpedantic")

file(GLOB SRC
    src/*.cpp
)

add_library(${PROJECT_NAME}
    ${SRC}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    include
)

add_subdirectory(test)
