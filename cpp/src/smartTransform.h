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
#include "quatTransform.h"

struct SmartTransform
{
	LinearTransform lTransf;
    QuatTransform qTransf;
    LinearTransform residualTranf;

	SmartTransform() : lTransf(), qTransf(), residualTranf() {}
    SmartTransform(LinearTransform& transf);
    SmartTransform(LinearTransform& transf, QuatTransform& quaternionTransform, float t);
    SmartTransform(LinearTransform& lt, QuatTransform& qt, LinearTransform& rt): lTransf(lt.M), qTransf(qt.dualQuaternion), residualTranf(rt.M) {}
	SmartTransform(glm::vec3 a3, glm::vec3 b3, glm::vec3 c3,
					glm::vec2 a2, glm::vec2 b2, glm::vec2 c2);
	glm::vec3 apply(glm::vec3 point) const;
};

static SmartTransform mix(SmartTransform a, SmartTransform b, float t)
{
	LinearTransform lt = mix(a.lTransf, b.lTransf, t);
	QuatTransform qt = mix(a.qTransf, b.qTransf, t);
	SmartTransform st(lt, qt, t);
    return st;
}
