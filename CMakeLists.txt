cmake_minimum_required(VERSION 3.29.1)

project(webUV)

add_library(${PROJECT_NAME} "cpp/src/mesh.cpp")
set (CMAKE_CXX_STANDARD 20)

set(CMAKE_TOOLCHAIN_FILE "D:\\dev\\emsdk\\upstream\\emscripten\\cmake\\Modules\\Platform\\Emscripten.cmake")


add_subdirectory("D:/dev/my_libs/glm-1.0.0-light")

target_include_directories(${PROJECT_NAME} PRIVATE 
    /D:/dev/my_libs/glm-1.0.0-light 
    /D:/dev/emsdk/upstream/emscripten/system/include
)

