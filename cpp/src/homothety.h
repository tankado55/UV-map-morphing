#pragma once

#include "glm/vec3.hpp"
#include "glm/vec2.hpp"
#include "glm/glm.hpp"

class Homothety
{
private:
    glm::vec3 center;
    float ratio;

public:
    Homothety(): center(0.0,0.0,0.0), ratio(1.0) {}
    void fromTo(glm::vec3 a3, glm::vec3 b3, glm::vec3 c3,
                    glm::vec2 a2, glm::vec2 b2, glm::vec2 c2);
    glm::vec3 apply(glm::vec3 point) const;
    glm::vec3 applyInverse() const;
};