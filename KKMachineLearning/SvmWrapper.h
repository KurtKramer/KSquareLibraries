#ifndef  _SVMWRAPPER_
#define  _SVMWRAPPER_

#include  "SVMparam.h"

namespace KKMLL 
{
  typedef enum {NORMAL, BAGGING, BOOSTING, SUBSPACE, SAMPLESV} Learn_Type;

  typedef  std::vector<kkint32>  Ivector;
  typedef  std::vector<float>    Fvector;
  typedef  std::vector<double>   Dvector;

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
   *                     of each class will be returned where there sum is 1.0
   *@param[in] unknownClassFeatureData  data structure you build that represents a sparse array of 
   *                     the feature data that is to be used for prediction.
   *@param[out] probabilities array that will receive the predicted probability for each class.
   *@param[in] knownClass  If you happen to know hat the class really is you can specify it 
   *                     here so as to get the probability of it returned back to you.
   *@param[out] predClass1  The prediction will be returned back in this field.
   *@param[out] predClass2  The second most likely class based off either probability or number of votes.
   *@param[out] predClass1Votes  Number votes that class 'predClass1' received.
   *@param[out] predClass2Votes  Number votes that class 'predClass2' received.
   *@param[out] predClass1Prob  Probability that class 'predClass1' received.
   *@param[out] predClass2Prob  Probability that class 'predClass2' received.
   *@param[out] probOfKnownClass  Probability that 'knownClass' class receved.
   *@param[out] winners   If voting was specified in 'svmParam' and there was a tie 
   *                     between 2 or more classes; them the classes that ties
   *                     will be in this vector.
   *@param[in] crossClassProbTable A 2 dimensional table that will have the computed probabilities
   *                     between all the possible 2 class combinations.
   *@param[in] breakTie  The difference in probability between the two most likely classes.
   */
  void   SvmPredictClass (SVMparam&               svmParam,
                          struct svm_model**      subModel,
                          const struct svm_node*  unknownClassFeatureData, 
                          kkint32*                votes,
                          double*                 probabilities,
                          kkint32                 knownClass,
                          kkint32&                predClass1,
                          kkint32&                predClass2,
                          kkint32&                predClass1Votes,
                          kkint32&                predClass2Votes,
                          double&                 predClass1Prob,
                          double&                 predClass2Prob,
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


void     SvmPredictRaw (svm_model**      submodel, 
                        const svm_node*  unKnownData,
                        double&          label,
                        double&          dist
                       );

                           
  void   SvmDestroyModel (struct svm_model**  subModel);

}  /* namespace KKMLL */

#endif
