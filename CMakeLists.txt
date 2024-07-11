cmake_minimum_required(VERSION 3.29.1)

project(webUV)

add_executable(${PROJECT_NAME} "cpp/src/mesh.cpp")
set (CMAKE_CXX_STANDARD 17)
target_include_directories(webUV PUBLIC )



add_subdirectory("D:/dev/my_libs/glm-1.0.0-light")
set(CMAKE_TOOLCHAIN_FILE "D:\\dev\\emsdk\\upstream\\emscripten\\cmake\\Modules\\Platform\\Emscripten.cmake")

target_include_directories(hello PRIVATE 
    /D:/dev/my_libs/glm-1.0.0-light 
    /D:/dev/emsdk/upstream/emscripten/system/include
)


