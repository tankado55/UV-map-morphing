em++ -g -fdebug-compilation-dir='.' ^
    cpp/src/hello.cpp cpp/src/mesh.cpp cpp/src/linearTransform.cpp cpp/src/dualQuatTransform.cpp cpp/src/smartTransform.cpp cpp/src/dualQuatHomTransform.cpp cpp/src/homothety.cpp cpp/src/testTransform.cpp --bind -o hello.js ^
    -sEXPORTED_FUNCTIONS=_interpolate,_free,_malloc ^
    -s EXPORTED_RUNTIME_METHODS=ccall,cwrap ^
    -I D:\dev\my_libs\glm-1.0.0-light ^
    -I/D:/dev/emsdk/upstream/emscripten/system/include ^
    -sALLOW_MEMORY_GROWTH
    
