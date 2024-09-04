#include "dualQuatTransform.h"
#include <Eigen/Dense>



void DualQuatTransform::fromTo(glm::vec3 a3, glm::vec3 b3, glm::vec3 c3, glm::vec2 a2, glm::vec2 b2, glm::vec2 c2)
{
    LinearTransform ln;
    ln.fromTo(a3, b3, c3, a2, b2, c2);
    fromMatrix(ln.M);
}

void DualQuatTransform::fromTo(glm::vec3 a3, glm::vec3 b3, glm::vec3 c3, glm::vec3 a2, glm::vec3 b2, glm::vec3 c2)
{
    LinearTransform ln;
    ln.fromTo(a3, b3, c3, a2, b2, c2);

    glm::vec3 bari2 = (a2 + b2 + c2) / 3.0f;
    glm::vec3 bari3 = (a3 + b3 + c3) / 3.0f;

    glm::vec3 a2v = a2 - bari2;
    glm::vec3 b2v = b2 - bari2;
    glm::vec3 a3v = a3 - bari3;
    glm::vec3 b3v = b3 - bari3;

    glm::vec3 n2 = glm::cross(a2v, b2v);
    glm::vec3 n3 = glm::cross(a3v, b3v);
    if (glm::length(n2) == 0)
    {
        std::cout << "n2 is 0" << std::endl;
        return;
        n2 = glm::vec3(0, 1.0, 0);
    }
    if (glm::length(n3) == 0)
    {
        std::cout << "n3 is 0" << std::endl;
    }

    n2 = glm::normalize(n2);
    n3 = glm::normalize(n3);

    glm::mat3 T2(a2v, b2v, n2);
    glm::mat3 T3(a3v, b3v, n3);

    glm::mat3 R = T2 * glm::inverse(T3);

    glm::mat3 rotationMatrix = utils::closestRotationSVD(R);
    glm::vec3 translationVector = glm::vec3(bari2.x, 0.0, bari2.y) - rotationMatrix * bari3;

    glm::quat rotationQuaternion = glm::quat_cast(rotationMatrix);
    rotationQuaternion = glm::normalize(rotationQuaternion);
    glm::quat translQuat(0.0, translationVector.x, translationVector.y, translationVector.z);
    glm::quat dual = translQuat * rotationQuaternion * 0.5f;
    dualQuaternion = glm::dualquat(rotationQuaternion, dual);
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

    for (int i = 0; i < 20; i++)
    {
        rotationMatrix = 0.5f * rotationMatrix + 0.5f * glm::inverse(glm::transpose(rotationMatrix));

    }

    glm::quat rotationQuaternion = glm::quat_cast(rotationMatrix);
    rotationQuaternion = glm::normalize(rotationQuaternion);
    glm::quat translQuat(0.0, translationVector.x, translationVector.y, translationVector.z);
    glm::quat dual = translQuat * rotationQuaternion * 0.5f;
    dualQuaternion = glm::dualquat(rotationQuaternion, dual);


    // glm::vec3 a(7.0, 9.0, 16.0);
    // glm::vec3 b1 = rotationMatrix * a + translationVector;
    // glm::vec3 b2 = apply(a);
    // std::cout << "DEBUG: " << glm::dot(b1 - b2, b1 - b2) << std::endl;
}

DualQuatTransform mix(const DualQuatTransform& a, const DualQuatTransform& b, float t)
{
	glm::dualquat _a = a.dualQuaternion;
	glm::dualquat _b = b.dualQuaternion;

	if (glm::dot(_a.real, _b.real) < 0.0f){
		_b = -_b;
	}
    return mixNoShortestPath(_a, _b, t);
}

DualQuatTransform mixNoShortestPath(const DualQuatTransform& a, const DualQuatTransform& b, float t)
{
    glm::quat primal = glm::mix(a.dualQuaternion.real, b.dualQuaternion.real, t);
    glm::quat dual = glm::mix(a.dualQuaternion.dual, b.dualQuaternion.dual, t);
	return DualQuatTransform( myNormalized(glm::dualquat(primal, dual)));
}

DualQuatTransform DualQuatTransform::inverse() const
{
    glm::dualquat result = glm::dualquat(glm::conjugate(dualQuaternion.real), glm::conjugate(dualQuaternion.dual));
    return DualQuatTransform(result);
}

glm::dualquat myNormalized(glm::dualquat dq)
{
    float length = glm::length(dq.real);
	dq.real /= length;
	dq.dual /= length;

	float dotted = glm::dot(dq.real, dq.dual);
	dq.dual -= dotted * dq.real;

    return dq;
}

glm::dualquat sum(glm::dualquat dq1, glm::dualquat dq2)
{
    if (glm::dot(dq1.real, dq2.real) < 0.0f){
		dq2 = -dq2;
	}
    return dq1 + dq2;
}
