#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include "cpp_src/glm-1.0.0-light/glm/vec3.hpp"
#include "cpp_src/glm-1.0.0-light/glm/vec2.hpp"
#include "cpp_src/glm-1.0.0-light/glm/mat3x3.hpp"
#include "cpp_src/glm-1.0.0-light/glm/gtc/matrix_transform.hpp"
#include "cpp_src/glm-1.0.0-light/glm/glm.hpp"
#include "cpp_src/glm-1.0.0-light/glm/gtc/type_ptr.hpp"
#include "cpp_src/glm-1.0.0-light/glm/gtc/quaternion.hpp"
#include "cpp_src/glm-1.0.0-light/glm/gtx/dual_quaternion.hpp"

struct LinearTransform
{
	glm::mat4 M;
	glm::dualquat dualQuaternion;

	LinearTransform() : M(glm::mat4(1.0)), dualQuaternion(glm::dualquat(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.0f))) {}
	LinearTransform(glm::mat4 _M);
	LinearTransform(glm::vec3 a3, glm::vec3 b3, glm::vec3 c3,
					glm::vec2 a2, glm::vec2 b2, glm::vec2 c2);
	glm::vec3 apply(glm::vec3 vec) const;
};

static LinearTransform mix(LinearTransform a, LinearTransform b, float t)
{
    //return LinearTransform(a.M * (1 - t) + b.M * t);

	LinearTransform result; //refactoring: if I remove the M from the internal state is ok, otherwise it is inconsistent
    //glm::dualquat dq = glm::mix(a.dualQuaternion, b.dualQuaternion, t);
    // glm::dualquat dq = a.dualQuaternion * (1 - t) + b.dualQuaternion * t;
    glm::dualquat dq = a.dualQuaternion +  t * b.dualQuaternion;
    dq = glm::normalize(dq);
	result.dualQuaternion = dq;
	return result;
}
