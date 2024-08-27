#pragma once

#include <Eigen/Dense>
#include <glm/mat3x3.hpp>

namespace utils
{
    inline Eigen::Matrix3f glmToEigen(const glm::mat3 &M)
    {
        Eigen::Matrix3f eigenM;

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                eigenM(i, j) = M[i][j];

        return eigenM;
    }

    inline glm::mat3 eigenToGlm(const Eigen::Matrix3f &M)
    {
        glm::mat3 glmM;

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                glmM[i][j] = M(i, j);

        return glmM;
    }
}
