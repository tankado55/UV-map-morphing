#pragma once

#include <Eigen/Core>
#include <Eigen/Sparse>

template <
    typename DerivedV,
    typename DerivedF,
    typename Derivedb>
inline bool arap_precomputation(
  const Eigen::MatrixBase<DerivedV> & V,
  const Eigen::MatrixBase<DerivedF> & F,
  const int dim,
  ARAPData & data);