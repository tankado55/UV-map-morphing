#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include "glm/vec3.hpp"
#include "glm/vec2.hpp"
#include "glm/mat3x3.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/dual_quaternion.hpp"
#include "linearTransform.h"

struct QuatTransform
{
	glm::dualquat dualQuaternion;

    private:
        glm::dualquat dualMult(glm::dualquat A, glm::dualquat B) const;

    public:
	    QuatTransform() : dualQuaternion(glm::dualquat(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.0f))) {}
	    QuatTransform(glm::mat4 M);
		QuatTransform(glm::dualquat dq): dualQuaternion(dq) {} 
	    QuatTransform(glm::vec3 a3, glm::vec3 b3, glm::vec3 c3,
	    				glm::vec2 a2, glm::vec2 b2, glm::vec2 c2);
	    glm::vec3 apply(glm::vec3 point) const;
};

static QuatTransform mix(const QuatTransform& a, const QuatTransform& b, float t) //TODO shortest path
{
    glm::quat primal = glm::mix(a.dualQuaternion.real, b.dualQuaternion.real, t);
    glm::quat dual = glm::mix(a.dualQuaternion.dual, b.dualQuaternion.dual, t);
	float length = glm::length(primal);
	primal /= length;
	dual /= length;

	glm::quat crossed = glm::cross(primal, dual);
	//dual -= crossed * primal;

	return QuatTransform(glm::dualquat(primal, dual));
}
