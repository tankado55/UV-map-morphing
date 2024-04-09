#include <emscripten.h>
#include <emscripten/em_macros.h>
#include <math.h>
#include <iostream>
#include "glm-1.0.0-light/glm/glm.hpp"

extern "C"
{

    int int_sqrt(int x)
    {
        return sqrt(x);
    }

    void plusOne(float buffer[], int size)
    {
        for (int i = 0; i < size; i++)
            buffer[i] += 1.0;

        // std::cout << "c++, bufferSize: " << size << std::endl;
        std::cout << "c++, firstEl: " << buffer[0] << std::endl;
    }

    void interpolate(float positions[], float uvs[], float t, float size, float result[])
    {
        float interpolationValue = t/100.0;
        for (int i = 0; i < size; i++)
        {
            result[i] = glm::mix(
                positions[i],
                uvs[i],
                interpolationValue);
        }
    }
}


// em++ cpp_src/hello.cpp cpp_src/mesh.cpp-o hello.js -sEXPORTED_FUNCTIONS=_interpolate,_plusOne,_free,_malloc -sEXPORTED_RUNTIME_METHODS=ccall,cwrap -g
