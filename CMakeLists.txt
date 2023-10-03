cmake_minimum_required(VERSION 3.9)
project(CountDown)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread")
set(CMAKE_CXX_STANDARD 17)
add_compile_definitions(UNICODE _UNICODE)

add_executable(main
    main.cpp
)
target_link_libraries(main PRIVATE)
