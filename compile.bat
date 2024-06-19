em++ -g -fdebug-compilation-dir='.' ^
    hello.cpp mesh.cpp linearTransform.cpp quatTransform.cpp --bind -o hello.js ^
    -sEXPORTED_FUNCTIONS=_interpolate,_free,_malloc ^
    -s EXPORTED_RUNTIME_METHODS=ccall,cwrap ^
    -I/D:/dev/my_libs/glm-1.0.0-light ^
    -I/D:/dev/emsdk/upstream/emscripten/system/include ^
    -I/D:/dev/webUVMapVisualizer/cpp_src/glm-1.0.0-light/glm/gtx ^
    -sALLOW_MEMORY_GROWTH
    
