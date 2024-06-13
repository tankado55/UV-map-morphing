#include "linearTransform.h"

LinearTransform::LinearTransform(glm::mat4 _M):
    M(_M)
{
    glm::mat3 rotationMatrix = glm::mat3(_M);
    glm::vec3 translationVector = glm::vec3(_M[3]);

    glm::quat rotationQuaternion = glm::quat_cast(rotationMatrix);
    dualQuaternion = glm::dualquat(rotationQuaternion, translationVector);
}

LinearTransform::LinearTransform(glm::vec3 a3, glm::vec3 b3, glm::vec3 c3,
                                 glm::vec2 a2, glm::vec2 b2, glm::vec2 c2)
{
    float d1 = a2.x;
    float d2 = a2.y;
    float d4 = b2.x;
    float d5 = b2.y;
    float d7 = c2.x;
    float d8 = c2.y;

    glm::vec2 bari2 = (a2 + b2 + c2) / 3.0f;
    glm::vec3 bari3 = (a3 + b3 + c3) / 3.0f;

    a2 -= bari2;
    b2 -= bari2;
    c2 -= bari2;
    a3 -= bari3;
    b3 -= bari3;
    c3 -= bari3;

    glm::vec3 n2 = glm::normalize(glm::cross(glm::vec3(a2.x, 0.0, a2.y), glm::vec3(b2.x, 0.0, b2.y)));
    glm::vec3 n3 = glm::normalize(glm::cross(a3, b3));

    glm::mat3 T2(glm::vec3(a2.x, 0.0, a2.y), glm::vec3(b2.x, 0.0, b2.y), n2);
    glm::mat3 T3(a3, b3, n3);

    glm::mat3 R = T2 * glm::inverse(T3);

    glm::vec3 t = glm::vec3(bari2.x, 0.0, bari2.y) - R * bari3;

    M = glm::mat4(
        glm::vec4(R[0], 0.0),
        glm::vec4(R[1], 0.0),
        glm::vec4(R[2], 0.0),
        glm::vec4(t, 1.0)
    );

    glm::quat rotationQuaternion = glm::quat_cast(R);
    rotationQuaternion = glm::normalize(rotationQuaternion);
    float norm = glm::length(rotationQuaternion);
    glm::quat translQuat(0.0, t.x, t.y, t.z);
    glm::quat dual = translQuat * rotationQuaternion * 0.5f;
    dualQuaternion = glm::dualquat(rotationQuaternion, dual);
}

glm::dualquat dualMult(glm::dualquat A, glm::dualquat C)
{
    return glm::dualquat(A.real * C.real, (A.real * C.dual) + (A.dual * C.real));
}

glm::vec3 LinearTransform::apply(glm::vec3 point) const
{
    //return glm::vec3(M * glm::vec4(vec, 1.0));
    // glm::dualquat pointDQ(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::quat(0.0f, point.x, point.y, point.z));
    // glm::dualquat pointDQ(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::quat(0.0f, point.x, point.y, point.z));
    // glm::dualquat resultDQ = dualQuaternion * pointDQ * glm::dualquat(glm::conjugate(dualQuaternion.real), glm::conjugate(dualQuaternion.dual));
    // return glm::vec3(resultDQ.dual.x, resultDQ.dual.y, resultDQ.dual.z);

    // glm::quat point_dual(0.0, 0.5f * point);
    // glm::dualquat temp(dualQuaternion.real, dualQuaternion.real * point_dual + dualQuaternion.dual);
    // glm::quat result = 2.0f * temp.dual * glm::conjugate(temp.real);
    // return glm::vec3(result.x, result.y, result.z);

    glm::dualquat pointDQ(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::quat(0.0f, point.x, point.y, point.z));
    glm::dualquat result = dualMult(pointDQ, glm::dualquat(glm::conjugate(dualQuaternion.real), -glm::conjugate(dualQuaternion.dual)));
    result = dualMult(dualQuaternion, result);
    return glm::vec3(result.dual.x, result.dual.y, result.dual.z);
}