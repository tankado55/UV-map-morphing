#include "smartTransform.h"

void SmartTransform::fromTo(glm::vec3 a3, glm::vec3 b3, glm::vec3 c3, glm::vec2 a2, glm::vec2 b2, glm::vec2 c2)
{
    dqTransf.fromTo(a3, b3, c3, a2, b2, c2);
    a3 = dqTransf.apply(a3);
    b3 = dqTransf.apply(b3);
    c3 = dqTransf.apply(c3);
    residualTranf.fromTo(a3, b3, c3, a2, b2, c2);
}

glm::vec3 SmartTransform::apply(glm::vec3 point) const
{
    glm::vec3 result = dqTransf.apply(point);
    result = residualTranf.apply(result);
    return result;
}

SmartTransform mix(SmartTransform a, SmartTransform b, float t)
{
	SmartTransform st;
// st.dqTransf = mix(a.dqTransf, b.dqTransf, t);
//     st.residualTranf = mix(a.residualTranf, b.residualTranf, t);
    float t0 = glm::clamp(t*2.0, 0.0,1.0);
    float t1 = glm::clamp(t*2.0 - 1.0, 0.0,1.0);

    st.dqTransf = mix(a.dqTransf, b.dqTransf, t0);
    st.residualTranf = mix(a.residualTranf, b.residualTranf, t1);
    return st;
}
