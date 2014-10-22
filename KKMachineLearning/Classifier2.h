#ifndef  _CLASSIFIER2_
#define  _CLASSIFIER2_
//***************************************************************************
//*                              Classifier2                                 *
//*                                                                         *
//*                                                                         *
//* <p>Copyright: Copyright (c) 2010</p>                                    *
//* <p>author     Kurt Kramer                                               * 
//*                                                                         *
//*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*
//*                                                                         *
//*  July 2002    Works hand in hand with TrainningProcess object.          *   
//*  Kurt Kramer  TrainingProcess2 will create a trainedModel from TrainingData     *
//*               using that trainedModel this object can then proceed to classify *
//*               images.                                                   *
//*                                                                         *
//*  2004-12-22  Kurt Kramer  Added method ProbabilitiesByClass which allow *
//*                           us to get the probabilities of each class.    *
//*                           class                                         *
//***************************************************************************

#include "Application.h"
#include "Model.h"
#include "ModelOldSVM.h"
#include "ModelParamOldSVM.h"
#include "ModelParam.h"
#include "RunLog.h"
#include "SVMModel.h"


namespace  KKMachineLearning
{

#ifndef  _FeatureVector_Defined_
class  FeatureVector;
typedef  FeatureVector*  FeatureVectorPtr;
#endif

#ifndef  _FeatureVectorList_Defined_
class  FeatureVectorList;
typedef  FeatureVectorList*  FeatureVectorListPtr;
#endif


#ifndef  _MLCLASS_
class  MLClass;
typedef  MLClass*  MLClassPtr;

class  MLClassList;
typedef  MLClassList*  MLClassListPtr;
#endif


#ifndef  _TRAININGCONFIGURATION2_
class  TrainingConfiguration2;
typedef  TrainingConfiguration2*  TrainingConfiguration2Ptr;
#endif


#ifndef  _TRAININGPROCESS2_
class  TrainingProcess2;
typedef  TrainingProcess2*  TrainingProcess2Ptr;
#endif



  class  Classifier2
  {
  public:
    Classifier2 (TrainingProcess2Ptr  _trainer,
                 RunLog&              _log
                );

    virtual  ~Classifier2 ();

    SVM_SelectionMethod   SelectionMethod ()  const;

    bool                 Abort ()  {return abort;}

    MLClassPtr           ClassifyAImage    (FeatureVector&  example);

    void  ClassifyAImage (FeatureVector&  example,
                          MLClassPtr&     predClass1,
                          MLClassPtr&     predClass2,
                          kkint32&        predClass1Votes,
                          kkint32&        predClass2Votes,
                          double&         knownClassProb,
                          double&         predClass1Prob,
                          double&         predClass2Prob,
                          kkint32&        numOfWinners,
                          double&         breakTie
                         );
 
    MLClassPtr  ClassifyAImage (FeatureVector&  example,
                                kkint32&        numOfWinners,
                                bool&           knownClassOneOfTheWinners
                               );

    MLClassPtr  ClassifyAImage (FeatureVector&  example,
                                double&         probability,
                                kkint32&        numOfWinners,
                                bool&           knownClassOneOfTheWinners,
                                double&         breakTie
                               );

    /**
     *@brief  For a given two class pair return the names of the 'numToFind' worst S/V's.
     *@details  This method will iterate through all the S/V's removing them one at a 
     *          time and recompute the decision boundary and probability.  It will then
     *          return the S/V's that when removed improve the probability in 'c1's 
     *          the most.
     *@param[in]  example  Example that was classified incorrectly.
     *@param[in]  numToFind  The number of teh worst examples you are looking for.
     *@param[in]  c1  Class that the 'example; parameter sghould have been classed as.
     *@param[in]  c2  Class that it was classified as.
     */
    vector<ProbNamePair>  FindWorstSupportVectors (FeatureVectorPtr  example,
                                                   kkint32           numToFind,
                                                   MLClassPtr        c1,
                                                   MLClassPtr        c2
                                                  );



    /**
     *@brief  For a given two class pair return the names of the 'numToFind' worst S/V's.
     *@details  This method will iterate through all the S/V's removing them one at a 
     *          time and rebuild a new SVM then submit example for testing.
     *@param[in]  example  Example that was classified incorrectly.
     *@param[in]  numToFind  The number of teh worst examples you are looking for.
     *@param[in]  c1  Class that the 'example; parameter sghould have been classed as.
     *@param[in]  c2  Class that it was classified as.
     */
    vector<ProbNamePair>  FindWorstSupportVectors2 (FeatureVectorPtr  example,
                                                    kkint32           numToFind,
                                                    MLClassPtr        c1,
                                                    MLClassPtr        c2
                                                   );


    virtual
    kkint32 MemoryConsumedEstimated ()  const;

    /**
     *@brief  For a given feature vector return back the probabilities and votes for each class.
     *@details
     *@param classes       [in]  List of classes that we can be predicted for  The ordering of 'votes' and 'probabilities' will be dictatd by this list.
     *@param example       [in]  Feature Vector to mak eprediction on.
     *@param votes         [out] Pointer to list of ints,  must be as large as 'classes'  The number of votes for each coresponding class will be stored hear.
     *@param probabilities [out] Pointer to list of double's,  must be as large as 'classes'  The priobability for each coresponding class will be stored hear.
     */
    void                 ProbabilitiesByClass (const MLClassList&  classes,
                                               FeatureVectorPtr    example,
                                               kkint32*            votes,
                                               double*             probabilities
                                              );

    void                 RetrieveCrossProbTable (MLClassList&  classes,
                                                 double**      crossProbTable  // two dimension matrix that needs to be classes.QueueSize ()  squared.
                                                );


    vector<KKStr>        SupportVectorNames (MLClassPtr  c1,
                                             MLClassPtr  c2
                                            );



  private:
    MLClassPtr  ClassifyAImageOneLevel (FeatureVector&  example);
 
    MLClassPtr  ClassifyAImageOneLevel (FeatureVector&  example,
                                        kkint32&        numOfWinners,
                                        bool&           knownClassOneOfTheWinners
                                       );


    MLClassPtr  ClassifyAImageOneLevel (FeatureVector&  example,
                                        double&         probability,
                                        kkint32&        numOfWinners, 
                                        bool&           knownClassOneOfTheWinners,
                                        double&         breakTie
                                       );


    //************************************************************
    // Variables that are Global to Classifier2 application. *
    //************************************************************

    bool                   abort;

    bool                   featuresAlreadyNormalized;

    MLClassListPtr         mlClasses;          /**< We will own the MLClass objects in this
                                                *   list.  Will be originally populated by
                                                *   TrainingConfiguration2 construction.
                                                */
    RunLog&                log;
  
    MLClassPtr             noiseImageClass;    /**< Point to class that represents Noise Images
                                                *  The object pointed to will also be included 
                                                *  in mlClasses.
                                                */
    ModelPtr               trainedModel;

    ModelOldSVMPtr         trainedModelOldSVM;

    SVMModelPtr            trainedModelSVMModel;

    TrainingProcess2Ptr    trainingProcess;

    MLClassPtr             unKnownImageClass;
  };
  typedef  Classifier2*   Classifier2Ptr;
}  /* namespace  KKMachineLearning */


#define  _Classifier2Defined_

#endif
