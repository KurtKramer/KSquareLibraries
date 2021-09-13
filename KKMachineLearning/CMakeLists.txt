set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Wextra -pedantic -Werror)

include_directories(../KKBase)

add_library(KKMachineLearning 
  Attribute.cpp
  BinaryClassParms.cpp
  ClassAssignments.cpp
  ClassificationBiasMatrix.cpp
  Classifier2.cpp
  ClassProb.cpp
  ClassStatistic.cpp
  ConfusionMatrix2.cpp
  CrossValidation.cpp
  CrossValidationMxN.cpp
  CrossValidationVoting.cpp
  DuplicateImages.cpp
  FactoryFVProducer.cpp
  FeatureEncoder2.cpp
  FeatureEncoder.cpp
  FeatureFileIOArff.cpp
  FeatureFileIOC45.cpp
  FeatureFileIOColumn.cpp
  FeatureFileIO.cpp
  FeatureFileIODstWeb.cpp
  FeatureFileIORoberts.cpp
  FeatureFileIOSparse.cpp
  FeatureFileIOUCI.cpp
  FeatureNumList.cpp
  FeatureVector.cpp
  FeatureVectorProducer.cpp
  FileDesc.cpp
  GrayScaleImagesFV.cpp
  GrayScaleImagesFVProducer.cpp
  ImageFeaturesDataIndexed.cpp
  ImageFeaturesNameIndexed.cpp
  KKMLVariables.cpp
  MLClass.cpp
  Model.cpp
  ModelDual.cpp
  ModelKnn.cpp
  ModelOldSVM.cpp
  ModelParam.cpp
  ModelParamDual.cpp
  ModelParamKnn.cpp
  ModelParamOldSVM.cpp
  ModelParamSvmBase.cpp
  ModelParamUsfCasCor.cpp
  ModelSvmBase.cpp
  ModelUsfCasCor.cpp
  NormalizationParms.cpp
  Orderings.cpp
  SizeDistribution.cpp
  svm289_BFS.cpp
  svm2.cpp
  svm.cpp
  SVMModel.cpp
  SVMparam.cpp
  SvmWrapper.cpp
  TrainingClass.cpp
  TrainingConfiguration2.cpp
  TrainingProcess2.cpp
  UsfCasCor.cpp
)

target_include_directories(KKMachineLearning PUBLIC $ENV{OutsidePackages}/zlib-1.2.11)

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    target_compile_features(KKMachineLearning PRIVATE cxx_lambda_init_captures cxx_variadic_templates)

    target_compile_options(KKMachineLearning PRIVATE 
                        -Wall
                        -Wextra # reasonable and standard
                        -Wshadow # warn the user if a variable`
                                 # declaration shadows one from a
                                 # parent context
                        -Wnon-virtual-dtor # warn the user if a class
                                           # with virtual functions
                                           # has a non-virtual
                                           # destructor. This helps
                                           # catch hard to track down
                                           # memory errors
                        -Wold-style-cast # warn for c-style casts
                        -Wcast-align # warn for potential performance
                                     # problem casts
                        -Wunused # warn on anything being unused
                        -Woverloaded-virtual # warn if you overload
                                             # (not override) a
                                             # virtual function
                        -Wpedantic # warn if non-standard C++ is used
                        -Wconversion # warn on type conversions that
                                     # may lose data
                        -Wsign-conversion # warn on sign conversions
                        -Wnull-dereference # warn if a null
                                           # dereference is detected
                        -Wdouble-promotion # warn if float is
                                           # implicit promoted to
                                           # double
                        -Wformat=2 # warn on security issues around
                                   # functions that format output (ie
                                   # printf)
                      )
endif ()
