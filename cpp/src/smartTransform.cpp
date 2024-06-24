#include "smartTransform.h"

void SmartTransform::fromTo(glm::vec3 a3, glm::vec3 b3, glm::vec3 c3, glm::vec2 a2, glm::vec2 b2, glm::vec2 c2)
{
    qTransf.fromTo(a3, b3, c3, a2, b2, c2);
    a3 = qTransf.apply(a3);
    b3 = qTransf.apply(b3);
    c3 = qTransf.apply(c3);
    residualTranf.fromTo(a3, b3, c3, a2, b2, c2);
}

glm::vec3 SmartTransform::apply(glm::vec3 point) const
{
    glm::vec3 result = qTransf.apply(point);
    result = residualTranf.apply(result);
    return result;
}

SmartTransform mix(SmartTransform a, SmartTransform b, float t)
{
	SmartTransform st;
    st.qTransf = mix(a.qTransf, b.qTransf, t);
    st.residualTranf = mix(a.residualTranf, b.residualTranf, t);
    return st;
}
