#include "linearTransform.h"

LinearTransform::LinearTransform(glm::vec3 a3, glm::vec3 b3, glm::vec3 c3, glm::vec2 a2, glm::vec2 b2, glm::vec2 c2)
{
    glm::vec2 bari2 = (a2 + b2 + c2) / 3.0f;
    glm::vec3 bari3 = (a3 + b3 + c3) / 3.0f;

    a2 -= bari2;
    b2 -= bari2;
    c2 -= bari2;
    a3 -= bari3;
    b3 -= bari3;
    c3 -= bari3;

    glm::vec3 n2 = glm::normalize(glm::cross(glm::vec3(a2, 0.0), glm::vec3(b2, 0.0)));
    glm::vec3 n3 = glm::normalize(glm::cross(a3, b3));

    glm::mat3 T2(glm::vec3(a2, 0.0), glm::vec3(b2, 0.0), n2);
    glm::mat3 T3(a3, b3, n3);

    glm::mat3 R = T2 * glm::inverse(T3);

    glm::vec3 t = glm::vec3(bari2, 0.0) - R * bari3;

    M = glm::mat4(
        glm::vec4(R[0], 0.0),
        glm::vec4(R[1], 0.0),
        glm::vec4(R[2], 0.0),
        glm::vec4(t, 1.0)
    );
}

glm::vec3 LinearTransform::apply(glm::vec3 vec) const
{
    return glm::vec3(M * glm::vec4(vec, 1.0));
}