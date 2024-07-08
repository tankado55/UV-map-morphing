#include "dualQuatHomTransform.h"

void DualQuatHomTransform::fromTo(glm::vec3 a3, glm::vec3 b3, glm::vec3 c3, glm::vec2 a2, glm::vec2 b2, glm::vec2 c2)
{
    homothety.fromTo(a3, b3, c3, a2, b2, c2);
    a2 = glm::vec2(homothety.apply(glm::vec3(a2, 0.0)));
    b2 = glm::vec2(homothety.apply(glm::vec3(b2, 0.0)));
    c2 = glm::vec2(homothety.apply(glm::vec3(c2, 0.0)));
    // a3 = homothety.apply(a3);
    // b3 = homothety.apply(b3);
    // c3 = homothety.apply(c3);
    
    dqTransf.fromTo(a3, b3, c3, a2, b2, c2);
    a3 = dqTransf.apply(a3);
    b3 = dqTransf.apply(b3);
    c3 = dqTransf.apply(c3);
    residualTranf.fromTo(a3, b3, c3, a2, b2, c2);
}

glm::vec3 DualQuatHomTransform::apply(glm::vec3 point) const
{
    glm::vec3 result = dqTransf.apply(point);
    //result = homothety.applyInverse(result); 
    result = residualTranf.apply(result); 
    return result;
}

DualQuatHomTransform mix(const DualQuatHomTransform &a, const DualQuatHomTransform &b, float t)
{
    float t0 = glm::clamp(t*2.0, 0.0,1.0);
    float t1 = glm::clamp(t*2.0 - 1.0, 0.0,1.0);

    DualQuatHomTransform transform;
    transform.dqTransf = mix(a.dqTransf, b.dqTransf, t0);
    transform.homothety = mixWithIdentity(b.homothety, t1);
    transform.residualTranf = mix(a.residualTranf, b.residualTranf, t1);
    return transform;
}
