/*
 * This file is part of the statismo library.
 *
 * Author: Marcel Luethi (marcel.luethi@unibas.ch)
 *
 * Copyright (c) 2011 University of Basel
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the project's author nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __STATIMO_CORE_RAND_SVD_H_
#define __STATIMO_CORE_RAND_SVD_H_

#include "statismo/core/RandUtils.h"

#include <Eigen/Dense>

#include <cmath>
#include <limits>
#include <random>

namespace statismo
{
/**
 * \ingroup Core
 */
template <typename ScalarType>
class RandSVD
{
public:
  using VectorType = Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>;
  using MatrixType = Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

  RandSVD(const MatrixType & A, unsigned k)
  {
    unsigned n = A.rows();

    static std::normal_distribution<> dist(0, 1);
    static auto                       r = std::bind(dist, rand::RandGen());

    // create gaussian random amtrix
    MatrixType Omega(n, k);
    for (unsigned i = 0; i < n; i++)
    {
      for (unsigned j = 0; j < k; j++)
      {
        Omega(i, j) = r();
      }
    }


    MatrixType                              Y = A * A.transpose() * A * Omega;
    Eigen::FullPivHouseholderQR<MatrixType> qr(Y);
    MatrixType                              Q = qr.matrixQ().leftCols(k + k);

    MatrixType B = Q.transpose() * A;

    using SVDType = Eigen::JacobiSVD<MatrixType>;
    SVDType    SVD(B, Eigen::ComputeThinU);
    MatrixType Uhat = SVD.matrixU();
    m_D = SVD.singularValues();
    m_U = (Q * Uhat).leftCols(k);
  }

  MatrixType
  MatrixU() const
  {
    return m_U;
  }

  VectorType
  SingularValues() const
  {
    return m_D;
  }


private:
  VectorType m_D;
  MatrixType m_U;
};

} // namespace statismo
#endif
