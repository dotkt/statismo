/*
 * Copyright (c) 2015 University of Basel
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

#include "utils/statismoLoggingUtils.h"

#include "statismo/ITK/itkReducedVarianceModelBuilder.h"
#include "statismo/ITK/itkStandardImageRepresenter.h"
#include "statismo/ITK/itkStandardMeshRepresenter.h"
#include "statismo/ITK/itkIO.h"
#include "statismo/ITK/itkStatisticalModel.h"

#include "lpo.h"

#include <itkImage.h>
#include <itkMesh.h>

#include <string>

namespace po = lpo;
using namespace std;

namespace
{

struct ProgramOptions
{
  string      strInputFileName;
  string      strOutputFileName;
  unsigned    uNumberOfComponents{ 0 };
  unsigned    uNumberOfDimensions{ 0 };
  double      dTotalVariance{ 0.0 };
  string      strType;
  bool        bIsQuiet{ false };
  std::string strLogFile{ "" };
};

bool
IsOptionsConflictPresent(ProgramOptions & opt)
{
  statismo::utils::ToLower(opt.strType);

  return (opt.strType != "shape" && opt.strType != "deformation") ||
         (opt.dTotalVariance != 0 && opt.uNumberOfComponents != 0) ||
         (opt.dTotalVariance == 0 && opt.uNumberOfComponents == 0) || opt.strInputFileName.empty() ||
         opt.strOutputFileName.empty() || opt.strInputFileName == opt.strOutputFileName;
}


template <class DataType, class RepresenterType>
void
ReduceModel(const ProgramOptions & opt, statismo::Logger * logger)
{
  using StatisticalModelType = typename itk::StatisticalModel<DataType>;
  using ReducedVarianceModelBuilderType = typename itk::ReducedVarianceModelBuilder<DataType>;

  auto representer = RepresenterType::New();
  representer->SetLogger(logger);
  auto model = itk::StatismoIO<DataType>::LoadStatisticalModel(representer, opt.strInputFileName);

  auto reducedVarModelBuilder = ReducedVarianceModelBuilderType::New();
  reducedVarModelBuilder->SetLogger(logger);
  typename StatisticalModelType::Pointer outputModel;

  if (opt.uNumberOfComponents != 0)
  {
    if (opt.uNumberOfComponents > model->GetNumberOfPrincipalComponents())
    {
      itkGenericExceptionMacro(<< "The model has " << model->GetNumberOfPrincipalComponents()
                               << " components. Upscaling models to more componenets isn't possible with this tool.");
    }
    outputModel =
      reducedVarModelBuilder->BuildNewModelWithLeadingComponents(model.GetPointer(), opt.uNumberOfComponents);
  }
  else
  {
    outputModel = reducedVarModelBuilder->BuildNewModelWithVariance(model.GetPointer(), opt.dTotalVariance);
  }

  itk::StatismoIO<DataType>::SaveStatisticalModel(outputModel, opt.strOutputFileName);
}

} // namespace

int
main(int argc, char ** argv)
{
  ProgramOptions                                     poParameters;
  po::program_options<std::string, unsigned, double> parser{ argv[0], "Program help:" };

  parser
    .add_opt<std::string>({ "type",
                            "t",
                            "Specifies the type of the model: shape and deformation are the two available types",
                            &poParameters.strType,
                            "shape" },
                          true)
    .add_opt<unsigned>({ "dimensionality",
                         "d",
                         "Dimensionality of the input image (only available if you're building a deformation model)",
                         &poParameters.uNumberOfDimensions,
                         3,
                         2,
                         3 },
                       true)
    .add_opt<std::string>({ "input-file", "i", "The path to the model file.", &poParameters.strInputFileName, "" },
                          true)
    .add_opt<unsigned>({ "numcomponents",
                         "n",
                         "Creates a new model with the specified number of components.",
                         &poParameters.uNumberOfComponents,
                         0 })
    .add_opt<double>({ "totalvariance",
                       "v",
                       "Creates a new Model that will have a fraction of the old models' variance. This parameter is "
                       "in percent and thus ranges from 0 to 100.",
                       &poParameters.dTotalVariance,
                       0.0f,
                       0.0f,
                       100.0f })
    .add_pos_opt<std::string>(
      { "Name of the output file where the reduced model will be written to.", &poParameters.strOutputFileName })
    .add_flag({ "quiet", "q", "Quiet mode (no log).", &poParameters.bIsQuiet, false })
    .add_opt<std::string>({ "log-file", "", "Path to the log file.", &poParameters.strLogFile, "" }, false);


  if (!parser.parse(argc, argv))
  {
    return EXIT_FAILURE;
  }

  if (IsOptionsConflictPresent(poParameters))
  {
    cerr << "A conflict in the options exists or insufficient options were set." << endl;
    cout << parser << endl;
    return EXIT_FAILURE;
  }

  try
  {
    const unsigned kDimensionality2D = 2;
    const unsigned kDimensionality3D = 3;

    std::unique_ptr<statismo::Logger> logger{ nullptr };
    if (!poParameters.bIsQuiet)
    {
      logger = statismo::cli::CreateLogger(poParameters.strLogFile);
    }

    if (poParameters.strType == "shape")
    {
      using DataType = itk::Mesh<float, kDimensionality3D>;
      using RepresenterType = itk::StandardMeshRepresenter<float, kDimensionality3D>;
      ReduceModel<DataType, RepresenterType>(poParameters, logger.get());
    }
    else
    {
      if (poParameters.uNumberOfDimensions == kDimensionality2D)
      {
        using VectorPixelType = itk::Vector<float, kDimensionality2D>;
        using DataType = itk::Image<VectorPixelType, kDimensionality2D>;
        using RepresenterType = itk::StandardImageRepresenter<VectorPixelType, kDimensionality2D>;
        ReduceModel<DataType, RepresenterType>(poParameters, logger.get());
      }
      else
      {
        using VectorPixelType = itk::Vector<float, kDimensionality3D>;
        using DataType = itk::Image<VectorPixelType, kDimensionality3D>;
        using RepresenterType = itk::StandardImageRepresenter<VectorPixelType, kDimensionality3D>;
        ReduceModel<DataType, RepresenterType>(poParameters, logger.get());
      }
    }
  }
  catch (itk::ExceptionObject & e)
  {
    cerr << "Could not reduce the model:" << endl;
    cerr << e.what() << endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
