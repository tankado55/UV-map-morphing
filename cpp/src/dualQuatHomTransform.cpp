#include "dualQuatHomTransform.h"

void DualQuatHomTransform::fromTo(glm::vec3 a3, glm::vec3 b3, glm::vec3 c3, glm::vec2 a2, glm::vec2 b2, glm::vec2 c2)
{
    homothety.fromTo(a3, b3, c3, a2, b2, c2);
    a2 = glm::vec2(homothety.apply(glm::vec3(a2, 0.0)));
    b2 = glm::vec2(homothety.apply(glm::vec3(b2, 0.0)));
    c2 = glm::vec2(homothety.apply(glm::vec3(c2, 0.0)));
    dqTransf.fromTo(a3, b3, c3, a2, b2, c2);
}

glm::vec3 DualQuatHomTransform::apply(glm::vec3 point) const
{
    return dqTransf.apply(point);
}

DualQuatHomTransform mix(const DualQuatHomTransform &a, const DualQuatHomTransform &b, float t)
{
    DualQuatHomTransform transform;
    transform.dqTransf = mix(a.dqTransf, b.dqTransf, t);
    return transform;
}
