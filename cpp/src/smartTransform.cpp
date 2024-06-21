#include "smartTransform.h"

SmartTransform::SmartTransform(LinearTransform& transf):
    lTransf(transf.M),
    qTransf(lTransf.M)
{
    
    glm::mat4 rotMatrix = glm::mat4_cast(qTransf.dualQuaternion.real);
    glm::quat t_quat = 2.0f * (qTransf.dualQuaternion.dual * glm::conjugate(qTransf.dualQuaternion.real));
    glm::mat4 quatFake = glm::mat4(
        glm::vec4(rotMatrix[0]),
        glm::vec4(rotMatrix[1]),
        glm::vec4(rotMatrix[2]),
        glm::vec4(t_quat.x, t_quat.y, t_quat.z, 1.0)
    );
    residualTranf = LinearTransform(glm::mat4(lTransf.M * glm::inverse(quatFake)));
    //residualTranf = LinearTransform(quatFake);
}

SmartTransform::SmartTransform(glm::vec3 a3, glm::vec3 b3, glm::vec3 c3, glm::vec2 a2, glm::vec2 b2, glm::vec2 c2) : lTransf(a3, b3, c3, a2, b2, c2),
                                                                                                                     qTransf(a3, b3, c3, a2, b2, c2)
{
    glm::vec3 aNewStart = qTransf.apply(a3);
    glm::vec3 bNewStart = qTransf.apply(b3);
    glm::vec3 cNewStart = qTransf.apply(c3);

    residualTranf = LinearTransform(aNewStart, bNewStart, cNewStart, a2, b2, c2);
}

glm::vec3 SmartTransform::apply(glm::vec3 point) const
{
    glm::vec3 result = qTransf.apply(point);
    //glm::vec3 result = residualTranf.apply(point);
    result = residualTranf.apply(result);
    return result;
}
