em++ -g -fdebug-compilation-dir='.' ^
    cpp/src/hello.cpp cpp/src/mesh.cpp cpp/src/linearTransform.cpp cpp/src/dualQuatTransform.cpp cpp/src/smartTransform.cpp cpp/src/utils.cpp --bind -o main_wasm.js ^
    -s EXPORTED_FUNCTIONS=_interpolate,_free,_malloc ^
    -s EXPORTED_RUNTIME_METHODS=ccall,cwrap ^
    -I D:\dev\my_libs\glm-1.0.0-light ^
    -I /D:/dev/emsdk/upstream/emscripten/system/include ^
    -I D:\dev\my_libs\eigen-3.4.0 ^
    -s ALLOW_MEMORY_GROWTH ^
    --preload-file /res/baking@/r ^
    -std=c++20
