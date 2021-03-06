/*
 * ConditionalModelBuilder.h
 *
 * Created by Remi Blanc,
 *
 * Copyright (c) 2011 ETH Zurich
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

#ifndef __STATIMO_CORE_CONDITIONAL_MODEL_BUILDER_H_
#define __STATIMO_CORE_CONDITIONAL_MODEL_BUILDER_H_

#include "statismo/core/CommonTypes.h"
#include "statismo/core/DataManagerWithSurrogates.h"
#include "statismo/core/ModelBuilder.h"
#include "statismo/core/StatisticalModel.h"

#include <vector>
#include <memory>

namespace statismo
{

/**
 * \brief Creates a statistical model conditioned on some external data
 *
 * The principle of this class is to exploit supplementary information (surrogate variables) describing
 * the samples (e.g. the age and gender of the subject) to generate a conditional statistical model as described
 * in  \cite 1.
 *
 * This class assumes a joint multivariate gaussian distribution of the sample vector and
 * the continuous surrogates.
 *
 * Categorical surrogates (e.g. gender) are taken into account by selecting the subset of samples that fit in the
 * requested categories.
 *
 * \warning Conditioning on too many categories may lead to small or empty training sets
 * \warning Using more surrogate variables than training samples may cause instabilities
 *
 * \sa DataManagerWithSurrogates
 * \ingroup ModelBuilders
 * \ingroup Core
 */
template <typename T>
class ConditionalModelBuilder : public ModelBuilderBase<T, ConditionalModelBuilder<T>>
{
public:
  using Superclass = ModelBuilderBase<T, ConditionalModelBuilder<T>>;
  using StatisticalModelType = typename Superclass::StatisticalModelType;
  // bool indicates whether the variable is in use
  using CondVariableValuePair = std::pair<bool, statismo::ScalarType>;
  using CondVariableValueVectorType = std::vector<CondVariableValuePair>;
  using DataManagerType = DataManagerWithSurrogates<T>;
  using DataItemListType = typename DataManagerType::DataItemListType;
  using DataItemWithSurrogatesType = typename DataManagerType::DataItemWithSurrogatesType;
  using SurrogateTypeInfoType = typename DataManagerType::SurrogateTypeInfoType;
  friend Superclass;

  /**
   * Builds new model from provided data and requested constraints.
   *
   * \param sampleSet List training samples with associated surrogate data - typically obtained from a
   * DataManagerWithSurrogates.
   * \param surrogateTypesInfo Vector with length corresponding to the number of surrogate
   * variables, indicating whether a variable is continuous or categorical - typically obtained from a
   * DataManagerWithSurrogates.
   * \param conditioningInfo Vector indicating which surrogates are used for conditioning,
   * and the conditioning value.
   * \param noiseVariance  Variance of the noise assumed on data
   * \param modelVarianceRetained  Ratio of retained variance for the model
   * \return New statistical model
   *
   */
  UniquePtrType<StatisticalModelType>
  BuildNewModel(const DataItemListType &            sampleSet,
                const SurrogateTypeInfoType &       surrogateTypesInfo,
                const CondVariableValueVectorType & conditioningInfo,
                float                               noiseVariance,
                double                              modelVarianceRetained = 1.0f) const;

private:
  std::size_t
  PrepareData(const DataItemListType &            DataItemList,
              const SurrogateTypeInfoType &       surrogateTypesInfo,
              const CondVariableValueVectorType & conditioningInfo,
              DataItemListType &                  acceptedSamples,
              MatrixType &                        surrogateMatrix,
              VectorType &                        conditions) const;

  CondVariableValueVectorType m_conditioningInfo;
};


} // namespace statismo

#include "ConditionalModelBuilder.hxx"

#endif
