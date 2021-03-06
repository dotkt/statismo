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

#ifndef __STATIMO_ITK_STATISTICAL_DEFORMATION_MODEL_TRANSFORM_H_
#define __STATIMO_ITK_STATISTICAL_DEFORMATION_MODEL_TRANSFORM_H_

#include "statismo/ITK/itkStandardImageRepresenter.h"
#include "statismo/ITK/itkStatisticalModel.h"
#include "statismo/ITK/itkStatisticalModelTransformBase.h"

#include <itkImage.h>
#include <itkVector.h>

namespace itk
{

/**
 * \brief Deformation transform used with scalar or vector valued Images
 *
 * \ingroup Transforms
 * \ingroup ITK
 */
template <typename DataSet, typename ScalarType, unsigned int DIMENSION>
class ITK_EXPORT StatisticalDeformationModelTransform
  : public itk::StatisticalModelTransformBase<DataSet, ScalarType, DIMENSION>
{

public:
  /* Standard class typedefs. */
  using Self = StatisticalDeformationModelTransform;
  using Superclass = itk::StatisticalModelTransformBase<DataSet, ScalarType, DIMENSION>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  itkSimpleNewMacro(Self);
  /** Run-time type information (and related methods). */
  itkTypeMacro(StatisticalDeformationModelTransform, Superclass);

  using InputPointType = typename Superclass::InputPointType;
  using OutputPointType = typename Superclass::OutputPointType;
  using RepresenterType = typename Superclass::RepresenterType;

  virtual ::itk::LightObject::Pointer
  CreateAnother() const
  {
    ::itk::LightObject::Pointer smartPtr;
    Pointer                     another = Self::New().GetPointer();
    this->CopyBaseMembers(another);

    smartPtr = static_cast<Pointer>(another);
    return smartPtr;
  }

  /**
   * Transform a given point according to the deformation induced by the StatisticalModel,
   * given the current parameters.
   *
   * \param pt point to tranform
   * \return transformed point
   */
  virtual OutputPointType
  TransformPoint(const InputPointType & pt) const override
  {
    auto d = this->m_statisticalModel->DrawSampleAtPoint(this->m_coeffVector, pt);

    OutputPointType transformedPoint;
    for (unsigned i = 0; i < pt.GetPointDimension(); i++)
    {
      transformedPoint[i] = pt[i] + d[i];
    }
    return transformedPoint;
  }
};


} // namespace itk

#endif
