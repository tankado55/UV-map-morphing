#include "quatTransform.h"

QuatTransform::QuatTransform(glm::mat4 M)
{
    glm::mat3 rotationMatrix = glm::mat3(M);
    glm::vec3 translationVector = glm::vec3(M[3]);

    glm::quat rotationQuaternion = glm::quat_cast(rotationMatrix);
    rotationQuaternion = glm::normalize(rotationQuaternion);
    glm::quat translQuat(0.0, translationVector.x, translationVector.y, translationVector.z);
    glm::quat dual = translQuat * rotationQuaternion * 0.5f;
    dualQuaternion = glm::dualquat(rotationQuaternion, dual);
}

QuatTransform::QuatTransform(glm::vec3 a3, glm::vec3 b3, glm::vec3 c3,
                                 glm::vec2 a2, glm::vec2 b2, glm::vec2 c2)
{
    LinearTransform ln(a3, b3, c3, a2, b2, c2);
    QuatTransform(ln.M);
}

glm::vec3 QuatTransform::apply(glm::vec3 point) const
{
    glm::dualquat pointDQ(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::quat(0.0f, point.x, point.y, point.z));
    glm::dualquat result = dualMult(pointDQ, glm::dualquat(glm::conjugate(dualQuaternion.real), -glm::conjugate(dualQuaternion.dual)));
    result = dualMult(dualQuaternion, result);
    return glm::vec3(result.dual.x, result.dual.y, result.dual.z);
}

glm::dualquat QuatTransform::dualMult(glm::dualquat A, glm::dualquat B) const
{
    return glm::dualquat(A.real * B.real, (A.real * B.dual) + (A.dual * B.real));
}
