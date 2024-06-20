#include "smartTransform.h"

SmartTransform::SmartTransform(LinearTransform& transf)
{
    lTransf = tranf;
    qTransf(lTransf);
    
}

SmartTransform::SmartTransform(glm::vec3 a3, glm::vec3 b3, glm::vec3 c3, glm::vec2 a2, glm::vec2 b2, glm::vec2 c2) : lTransf(a3, b3, c3, a2, b2, c2),
                                                                                                                     qTransf(a3, b3, c3, a2, b2, c2)
{
    glm::vec3 aNewStart = qTransf.apply(a3);
    glm::vec3 bNewStart = qTransf.apply(b3);
    glm::vec3 cNewStart = qTransf.apply(c3);

    residualTranf(aNewStart, bNewStart, cNewStart, a2, b2, c2);
}

glm::vec3 SmartTransform::apply(glm::vec3 point) const
{
    return glm::vec3(residualTranf.apply(point));
}
