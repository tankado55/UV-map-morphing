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

    inline Eigen::Matrix3f closestRotationSVD(const Eigen::Matrix3f &M)
    {
        // Compute the SVD of the matrix M
        Eigen::JacobiSVD<Eigen::MatrixXf> svd(M, Eigen::ComputeFullU | Eigen::ComputeFullV);

        // Get U and V matrices from the SVD
        Eigen::Matrix3f U = svd.matrixU();
        Eigen::Matrix3f V = svd.matrixV();

        // Compute the closest rotation matrix R = U * V^T
        Eigen::Matrix3f R = U * V.transpose();

        // Ensure that the determinant of R is 1 (not -1)
        if (R.determinant() < 0)
        {
            U.col(2) *= -1; // Flip the sign of the third column of U
            R = U * V.transpose();
        }

        return R;
    }

    inline glm::mat3 closestRotationSVD(const glm::mat3 &M)
    {
        Eigen::Matrix3f eigenM = utils::glmToEigen(M);

        Eigen::Matrix3f eigenR = closestRotationSVD(eigenM);

        return utils::eigenToGlm(eigenR);
    }
}
