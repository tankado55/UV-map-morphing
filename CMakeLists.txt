cmake_minimum_required(VERSION 3.0)

# Set project name
project(MyProject)

# Set C++ version
set(CMAKE_CXX_STANDARD 11)

# Set source files
set(SOURCES
    hello.cpp
)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/output")

# Set compiler for Emscripten
#set(CMAKE_CXX_COMPILER em++)

set(CMAKE_TOOLCHAIN_FILE "D:\\dev\\emsdk\\upstream\\emscripten\\cmake\\Modules\\Platform")

# Set compiler flags
set(EMSCRIPTEN_FLAGS "${CMAKE_CXX_FLAGS} \
    -s EXPORTED_FUNCTIONS='_interpolate,_plusOne,_free,_malloc' \
    -s EXPORTED_RUNTIME_METHODS=ccall,cwrap \
    -g"
)

# Add include directories
include_directories(
    "D:\\dev\\emsdk\\upstream\\emscripten\\system\\include"
    "D:\dev\my_libs\glm-1.0.0-light"
    # Add additional include paths here as needed
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})


# command line
em++ hello.cpp mesh.cpp --bind -o hello.js ^
    -sEXPORTED_FUNCTIONS=_interpolate,_plusOne,_free,_malloc ^
    -s EXPORTED_RUNTIME_METHODS=ccall,cwrap ^
    -g ^
    -I/D:/dev/my_libs/glm-1.0.0-light ^
    -I/D:/dev/emsdk/upstream/emscripten/system/include