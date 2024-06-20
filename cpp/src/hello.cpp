#include <emscripten.h>
#include <emscripten/em_macros.h>
#include <math.h>
#include <emscripten/bind.h>
#include <iostream>
#include "glm/glm.hpp"

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
/*
class MyClass {
public:
  MyClass(int x, std::string y)
    : x(x)
    , y(y)
  {}

  void incrementX() {
    ++x;
  }

  int getX() const { return x; }
  void setX(int x_) { x = x_; }

  static std::string getStringFromInstance(const MyClass& instance) {
    return instance.y;
  }

private:
  int x;
  std::string y;
};

// Binding code

EMSCRIPTEN_BINDINGS(my_class_example) {
  class_<MyClass>("MyClass")
    .constructor<int, std::string>()
    .function("incrementX", &MyClass::incrementX)
    .property("x", &MyClass::getX, &MyClass::setX)
    .property("x_readonly", &MyClass::getX)
    .class_function("getStringFromInstance", &MyClass::getStringFromInstance)
    ;
}*/



// em++ hello.cpp -o hello.js -sEXPORTED_FUNCTIONS=_interpolate,_plusOne,_free,_malloc -sEXPORTED_RUNTIME_METHODS=ccall,cwrap -g


