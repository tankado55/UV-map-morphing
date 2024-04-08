make_minimum_required(VERSION 3.8)

project(test)

add_definitions(-std=c++17)
set (CMAKE_CXX_STANDARD 17)

add_library(calc SHARED calc.cpp)
add_executable(hello hello.cpp)
target_link_libraries(test PUBLIC calc)