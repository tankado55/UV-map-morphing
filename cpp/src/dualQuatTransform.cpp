#include "dualQuatTransform.h"

void DualQuatTransform::fromTo(glm::vec3 a3, glm::vec3 b3, glm::vec3 c3, glm::vec2 a2, glm::vec2 b2, glm::vec2 c2)
{
    LinearTransform ln;
    ln.fromTo(a3, b3, c3, a2, b2, c2);
    fromMatrix(ln.M);
}

glm::vec3 DualQuatTransform::apply(glm::vec3 point) const
{
    glm::dualquat pointDQ(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::quat(0.0f, point.x, point.y, point.z));
    glm::dualquat result = dualMult(pointDQ, glm::dualquat(glm::conjugate(dualQuaternion.real), -glm::conjugate(dualQuaternion.dual)));
    result = dualMult(dualQuaternion, result);
    return glm::vec3(result.dual.x, result.dual.y, result.dual.z);
}

glm::dualquat DualQuatTransform::dualMult(glm::dualquat A, glm::dualquat B) const
{
    return glm::dualquat(A.real * B.real, (A.real * B.dual) + (A.dual * B.real));
}

void DualQuatTransform::fromMatrix(glm::mat4 M)
{
    glm::mat3 rotationMatrix = glm::mat3(M);
    glm::vec3 translationVector = glm::vec3(M[3]);

    glm::quat rotationQuaternion = glm::quat_cast(rotationMatrix);
    rotationQuaternion = glm::normalize(rotationQuaternion);
    glm::quat translQuat(0.0, translationVector.x, translationVector.y, translationVector.z);
    glm::quat dual = translQuat * rotationQuaternion * 0.5f;
    dualQuaternion = glm::dualquat(rotationQuaternion, dual);
}

DualQuatTransform mix(const DualQuatTransform& a, const DualQuatTransform& b, float t)
{
	glm::dualquat _a = a.dualQuaternion;
	glm::dualquat _b = b.dualQuaternion;

	// if (glm::dot(_a.real, _b.real) < 0.0f){
	// 	_b = -_b;
	// }

    glm::quat primal = glm::mix(_a.real, _b.real, t);
    glm::quat dual = glm::mix(_a.dual, _b.dual, t);
	float length = glm::length(primal);
	primal /= length;
	dual /= length;

	float dotted = glm::dot(primal, dual);
	dual -= dotted * primal;


	return DualQuatTransform(glm::dualquat(primal, dual));
}
