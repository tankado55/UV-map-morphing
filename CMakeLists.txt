cmake_minimum_required(VERSION 3.5)

# Set project name
project(MyProject)

# Set C++ version
set(CMAKE_CXX_STANDARD 11)

# Set source files
set(SOURCES
    hello.cpp
    mesh.cpp
)

set(CMAKE_TOOLCHAIN_FILE "D:\\dev\\emsdk\\upstream\\emscripten\\cmake\\Modules\\Platform\\Emscripten.cmake")

set_target_properties(${PROJECT_NAME} PROPERTIES 
    COMPILE_FLAGS "-sEXPORTED_FUNCTIONS=_interpolate,_plusOne,_free,_malloc -sEXPORTED_RUNTIME_METHODS=ccall,cwrap -g --bind"
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    /D:/dev/my_libs/glm-1.0.0-light 
    /D:/dev/emsdk/upstream/emscripten/system/include
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})
