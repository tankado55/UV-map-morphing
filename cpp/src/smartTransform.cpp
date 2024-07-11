#include "smartTransform.h"

inline glm::mat3 closestRotation(glm::mat3 M)
{
    glm::mat3 rotationMatrix = M;

    for (int i = 0; i < 20; i++)
    {
        rotationMatrix = 0.5f * rotationMatrix + 0.5f * glm::inverse(glm::transpose(rotationMatrix));
    }
    return rotationMatrix;
}

void SmartTransform::fromTo(glm::vec3 a3, glm::vec3 b3, glm::vec3 c3, glm::vec2 a2, glm::vec2 b2, glm::vec2 c2)
{
    glm::vec2 bari2 = (a2 + b2 + c2) / 3.0f;
    glm::vec3 bari3 = (a3 + b3 + c3) / 3.0f;
    
    glm::vec2 a2v = a2 - bari2;
    glm::vec2 b2v = b2 - bari2;
    glm::vec3 a3v = a3 - bari3;
    glm::vec3 b3v = b3 - bari3;

    glm::vec3 n2 = glm::normalize(glm::cross(glm::vec3(a2v.x, 0.0, a2v.y), glm::vec3(b2v.x, 0.0, b2v.y)));
    glm::vec3 n3 = glm::normalize(glm::cross(a3v, b3v));

    glm::mat3 T2(glm::vec3(a2v.x, 0.0, a2v.y), glm::vec3(b2v.x, 0.0, b2v.y), n2);
    glm::mat3 T3(a3v, b3v, n3);

    glm::mat3 R = T2 * glm::inverse(T3);

    linearTransf.M = glm::mat4(
        glm::vec4(R[0], 0.0),
        glm::vec4(R[1], 0.0),
        glm::vec4(R[2], 0.0),
        glm::vec4(glm::vec3(bari2.x, 0.0, bari2.y) - R * bari3, 1.0)
    );

// Dualquat
    glm::mat3 rotationMatrix = closestRotation(R);
    glm::vec3 translationVector = glm::vec3(bari2.x, 0.0, bari2.y) - rotationMatrix * bari3;

    glm::quat rotationQuaternion = glm::quat_cast(rotationMatrix);
    rotationQuaternion = glm::normalize(rotationQuaternion);
    glm::quat translQuat(0.0, translationVector.x, translationVector.y, translationVector.z);
    glm::quat dual = translQuat * rotationQuaternion * 0.5f;
    dqTransf.dualQuaternion = glm::dualquat(rotationQuaternion, dual);
    
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

SmartTransform mix(SmartTransform a, SmartTransform b, float t, bool splitResidual)
{
	SmartTransform st;
    if (splitResidual == false)
    {
        st.dqTransf = mix(a.dqTransf, b.dqTransf, t);
        st.residualTranf = mix(a.residualTranf, b.residualTranf, t);
    }
    else
    {
        float t0 = glm::clamp(t*2.0, 0.0,1.0);
        float t1 = glm::clamp(t*2.0 - 1.0, 0.0,1.0);

        st.dqTransf = mix(a.dqTransf, b.dqTransf, t0);
        st.residualTranf = mix(a.residualTranf, b.residualTranf, t1);
    }
    
    return st;
}

SmartTransform mixLinear(SmartTransform a, SmartTransform b, float t)
{
	SmartTransform st;
    st.residualTranf = mix(a.linearTransf, b.linearTransf, t);
    return st;
}
