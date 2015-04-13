#ifndef  _MLLTYPES_
#define  _MLLTYPES_

/**
 *@namespace  KKMLL
 *@brief  Namespace for all K^2 Machine Learning  code.
 *@details
 * A library of routines used for Machine Learning, building Classifiers, etc.
 *
 *@section MachineLearninig "Machine Learning Code"
 *
 *@subsection  FeatureData "Feature Data"
 * There are several Feature Data formats supported.  Each one has its own class that is derived from
 * 'FeatureFileIO'.  The description of the data is managed by 'FileDesc'.  For each type of dataset there
 * will exist on one instance of a FileDesc class.
 *@see Attribute
 *@see FeatureNumList
 *@see FeatureFileIO
 *@see FeatureVector
 *@see FeatureVectorList
 *@see MLClass
 *@see BitReduction
 *@see FeatureEncoder
 *@see NormalizationParms
 *@see Orderings
 *
 *@subsection  LearningAlgorithms "Learning Algorithms"
 *There are several learning algorithms implemented.  The Learning algorithms are all sub-classed from 'Model' and their
 *related parameters are all sub-classed from 'ModelParam'.
 *@see Model
 *@see ModelKnn
 *@see ModelOldSVM
 *@see ModelSvmBase
 *@see ModelParam
 *@see TrainingClass
 *@see TrainingProcess2
 *
 *@subsection AnalysisRoutines  "Analysis Routines"
 *@see ConfusionMatrix2
 *@see CrossValidation
 *@see CrossValidationMxN
 *@see CrossValidationVoting
 *@see SizeDistribution
 */

namespace  KKMLL  
{
}


#endif

