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

#include <iostream>

struct DualQuatTransform
{
	glm::dualquat dualQuaternion;

    private:
        glm::dualquat dualMult(glm::dualquat A, glm::dualquat B) const;

    public:
	    DualQuatTransform() : dualQuaternion(glm::dualquat(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.0f))) {}
		DualQuatTransform(glm::dualquat dq): dualQuaternion(dq) {}
	    void fromMatrix(glm::mat4 M);
	    void fromTo(glm::vec3 a3, glm::vec3 b3, glm::vec3 c3,
	    				glm::vec2 a2, glm::vec2 b2, glm::vec2 c2);
	    void fromTo(glm::vec3 a3, glm::vec3 b3, glm::vec3 c3,
	    				glm::vec3 a2, glm::vec3 b2, glm::vec3 c2);
	    glm::vec3 apply(glm::vec3 point) const;
		DualQuatTransform inverse() const;
};

DualQuatTransform mix(const DualQuatTransform& a, const DualQuatTransform& b, float t);
DualQuatTransform mixNoShortestPath(const DualQuatTransform& a, const DualQuatTransform& b, float t);

glm::dualquat myNormalized(glm::dualquat dq);

glm::dualquat sum(glm::dualquat dq1, glm::dualquat dq2);
