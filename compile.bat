em++ hello.cpp mesh.cpp --bind -o hello.js ^
    -sEXPORTED_FUNCTIONS=_interpolate,_plusOne,_free,_malloc ^
    -s EXPORTED_RUNTIME_METHODS=ccall,cwrap ^
    -g ^
    -I/D:/dev/my_libs/glm-1.0.0-light ^
    -I/D:/dev/emsdk/upstream/emscripten/system/include
