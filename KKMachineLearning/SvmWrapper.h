#ifndef  _SVMWRAPPER_
#define  _SVMWRAPPER_

/**
 @file SvmWrapper.h
 @code
 **********************************************************************************
 * @author Kurt Kramer                                                            *
 *                                                                                *
 *  Provides an interface to the svm.cpp functions.  This was copyied from a ver- *
 *  sion written by Tong Luo.  Received from Tong June-2-2003.                    *
 *                                                                                *
 **********************************************************************************
 @endcode
 */


#include  "SVMparam.h"


namespace KKMachineLearning 
{

  typedef enum {NORMAL, BAGGING, BOOSTING, SUBSPACE, SAMPLESV} Learn_Type;


  typedef  std::vector<kkint32>  Ivector;
  typedef  std::vector<float>  Fvector;
  typedef  std::vector<double> Dvector;


  void    SvmSaveModel (ostream&            o,
                        struct svm_model**  model
                       );


  void    SvmSaveModel (struct svm_model**  subModel,
                        const char*         fileName,
                        bool&               successfull
                       );


  struct  svm_model**   SvmLoadModel (const char* fileName);

  struct  svm_model**   SvmLoadModel (istream&  f,
                                      RunLog&   log
                                     );


  struct  svm_model**   SvmTrainModel (const struct svm_parameter&  param,
                                       struct svm_problem&          subprob
                                      );

  void  EncodeProblem (const struct svm_paramater&  param, 
                       struct svm_problem&          prob_in,
                       struct svm_problem&          prob_out
                      );


  /**
   *@param[in] svmParam  Structure that has all the parameters used for building the SVM.
   *@param[in] subModel  This is the model(classifier) returned by SvmTrainModel.
   *@param[in] probabilities Array that must be as big as number of classes the probability 
   *                     of each class will be retrned where there sum is 1.0
   *
   *@param[in] unknownClassFeatureData  data structure you build that represents a sparse array of 
   *                     the feature data that is to be used for prediction.
   *
   *@param[in] knownClass  If you happen to know hat the class really is you can specify it 
   *                     here so as to get the probability of it returned bacj to you.
   *
   *@param[in] predClass   The prediction will be retuend back in this field.
   *@param[in] winners   If voting was specified in 'svmParam' and there was a tie 
   *                     between 2 or more classes; them the classes that ties
   *                     will be in this vector.
   *
   *@param[in] crossClassProbTable A 2 dimensional table that will have the computed probabities
   *                     between all the posible 2 class combinations.
   *
   *@param[in] breakTie  The difference in probability between the two most likely classes.
   *
   */
  void   SvmPredictClass (SVMparam&               svmParam,
                          struct svm_model**      subModel,
                          const struct svm_node*  unknownClassFeatureData, 
                          kkint32*                  votes,
                          double*                 probabilities,
                          kkint32                 knownClass,
                          kkint32&                predClass1,
                          kkint32&                predClass2,
                          kkint32&                predClass1Votes,
                          kkint32&                predClass2Votes,
                          double&                 probOfPredClass1,
                          double&                 probOfPredClass2,
                          double&                 probOfKnownClass,
                          Ivector&                winners,
                          double**                crossClassProbTable,
                          double&                 breakTie
                         );



  kkint32  SvmPredictTwoClass (const struct svm_parameter&   param,
                             svm_model**                   submodel, 
                             const svm_node*               unKnownData, 
                             kkint32                       desired, 
                             double&                       dist,
                             double&                       probability,
                             kkint32                       excludeSupportVectorIDX
                            );

  void   SvmDestroyModel (struct svm_model**  subModel);

}  /* namespace KKMachineLearning */

#endif
