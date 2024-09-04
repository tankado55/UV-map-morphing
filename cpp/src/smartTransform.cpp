#include "smartTransform.h"

#define LINEAR_THEN_QUAT

inline glm::mat3 closestRotation(glm::mat3 M)
{
    glm::mat3 rotationMatrix = M;

    for (int i = 0; i < 20; i++)
    {
        if (glm::determinant(glm::transpose(rotationMatrix)) == 0)
        {
            std::cout << "not invertible\n";
        }
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

    glm::vec3 n2 = glm::cross(glm::vec3(a2v.x, 0.0, a2v.y), glm::vec3(b2v.x, 0.0, b2v.y));
    glm::vec3 n3 = glm::cross(a3v, b3v);
    if (glm::length(n2) == 0)
    {
        std::cout << "n2 is 0" << std::endl;
        return;
        n2 = glm::vec3(0, 1.0, 0);
    }
    if (glm::length(n3) == 0)
    {
        std::cout << "n3 is 0" << std::endl;
    }

    n2 = glm::normalize(n2);
    n3 = glm::normalize(n3);

    glm::mat3 T2(glm::vec3(a2v.x, 0.0, a2v.y), glm::vec3(b2v.x, 0.0, b2v.y), n2);
    glm::mat3 T3(a3v, b3v, n3);

    glm::mat3 R = T2 * glm::inverse(T3);

    linearTransf.M = glm::mat4(
        glm::vec4(R[0], 0.0),
        glm::vec4(R[1], 0.0),
        glm::vec4(R[2], 0.0),
        glm::vec4(glm::vec3(bari2.x, 0.0, bari2.y) - R * bari3, 1.0));

    // Dualquat
    if (R[0].x != R[0].x)
        return;
    glm::mat3 rotationMatrix = utils::closestRotationSVD(R);
    glm::vec3 translationVector = glm::vec3(bari2.x, 0.0, bari2.y) - rotationMatrix * bari3;

    glm::quat rotationQuaternion = glm::quat_cast(rotationMatrix);
    rotationQuaternion = glm::normalize(rotationQuaternion);
    glm::quat translQuat(0.0, translationVector.x, translationVector.y, translationVector.z);
    glm::quat dual = translQuat * rotationQuaternion * 0.5f;
    dqTransf.dualQuaternion = glm::dualquat(rotationQuaternion, dual);

#ifdef LINEAR_THEN_QUAT
    DualQuatTransform dqInverse = dqTransf.inverse();
    glm::vec3 ai = dqInverse.apply(glm::vec3(a2.x, 0.0, a2.y));
    glm::vec3 bi = dqInverse.apply(glm::vec3(b2.x, 0.0, b2.y));
    glm::vec3 ci = dqInverse.apply(glm::vec3(c2.x, 0.0, c2.y));
    residualTranf.fromTo(a3, b3, c3, ai, bi, ci);
#else
    a3 = dqTransf.apply(a3);
    b3 = dqTransf.apply(b3);
    c3 = dqTransf.apply(c3);
    residualTranf.fromTo(a3, b3, c3, a2, b2, c2);
#endif
}

glm::vec3 SmartTransform::apply(glm::vec3 point) const
{
#ifdef LINEAR_THEN_QUAT
    glm::vec3 result = residualTranf.apply(point);
    result = dqTransf.apply(result);
#else
    glm::vec3 result = dqTransf.apply(point);
    result = residualTranf.apply(result);
#endif
    return result;
}

SmartTransform mix(SmartTransform a, SmartTransform b, float t, bool splitResidual, int whichPath)
{
    SmartTransform st;
    float t0;
    float t1;
    if (splitResidual == false)
    {
        t0 = t1 = t;
    }
    else
    {
        #ifdef LINEAR_THEN_QUAT
        t1 = glm::clamp(t * 2.0, 0.0, 1.0);
        t0 = glm::clamp(t * 2.0 - 1.0, 0.0, 1.0);
        #else
        t0 = glm::clamp(t * 2.0, 0.0, 1.0);
        t1 = glm::clamp(t * 2.0 - 1.0, 0.0, 1.0);
        #endif

    }
    st.dqTransf = mixNoShortestPath(a.dqTransf, DualQuatTransform(b.dqTransf.dualQuaternion * float(whichPath)), t0);

    st.residualTranf = mix(a.residualTranf, b.residualTranf, t1);

    return st;
}

SmartTransform mixLinear(SmartTransform a, SmartTransform b, float t)
{
    SmartTransform st;
    st.residualTranf = mix(a.linearTransf, b.linearTransf, t);
    return st;
}
