cmake_minimum_required(VERSION 3.29.1)

project(webUV)

set (CMAKE_CXX_STANDARD 20)

set(SOURCES
    cpp/src/hello.cpp
    cpp/src/mesh.cpp
    cpp/src/linearTransform.cpp
    cpp/src/dualQuatTransform.cpp
    cpp/src/smartTransform.cpp
)

set(CMAKE_TOOLCHAIN_FILE "D:\\dev\\emsdk\\upstream\\emscripten\\cmake\\Modules\\Platform\\Emscripten.cmake")


add_subdirectory("cpp/vendor/glm-1.0.0-light/glm")

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
    cpp/vendor/glm-1.0.0-light
)