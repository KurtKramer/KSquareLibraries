#ifndef  _CLASSIFIER2_
#define  _CLASSIFIER2_



#include "Application.h"


// Problem apperas to b in file below,
#include "Model.h"



#include "ModelOldSVM.h"
#include "ModelParamOldSVM.h"



#include "ModelParam.h"
#include "RunLog.h"
#include "SVMModel.h"



namespace  KKMLL
{
#if  !defined(_CLASSPROB_)
class  ClassProb;
typedef  ClassProb*  ClassProbPtr;
typedef  ClassProb const *  ClassProbConstPtr;
class  ClassProbList;
typedef  ClassProbList*  ClassProbListPtr;
#endif


#ifndef  _FeatureVector_Defined_
class  FeatureVector;
typedef  FeatureVector*  FeatureVectorPtr;
#endif

#ifndef  _FeatureVectorList_Defined_
class  FeatureVectorList;
typedef  FeatureVectorList*  FeatureVectorListPtr;
#endif


#if   !defined(_MLCLASS_)
class  MLClass;
typedef  MLClass*  MLClassPtr;

class  MLClassList;
typedef  MLClassList*  MLClassListPtr;
#endif


#if  !defined(_TRAININGCONFIGURATION2_)
class  TrainingConfiguration2;
typedef  TrainingConfiguration2*  TrainingConfiguration2Ptr;
#endif


#if   !defined(_TrainingProcess2_Defined_)
class  TrainingProcess2;
typedef  TrainingProcess2*  TrainingProcess2Ptr;
#endif


#if  !defined(_TrainingProcess2List_Defined_)
class  TrainingProcess2List;
typedef  TrainingProcess2List*  TrainingProcess2ListPtr;
#endif

  class  Classifier2List;
  typedef  Classifier2List*  Classifier2ListPtr;

  class  Classifier2
  {
  public:
    typedef  Classifier2*  Classifier2Ptr;

    Classifier2 (TrainingProcess2Ptr  _trainer,
                 RunLog&              _log
                );

    virtual  ~Classifier2 ();

    bool                  Abort           ()  const  {return abort;}
    const KKStr&          ConfigRootName  ()  const  {return configRootName;}
    SVM_SelectionMethod   SelectionMethod ()  const;

    MLClassPtr            ClassifyAExample    (FeatureVector&  example);

    void  ClassifyAExample (FeatureVector&  example,
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
 
    MLClassPtr  ClassifyAExample (FeatureVector&  example,
                                  kkint32&        numOfWinners,
                                  bool&           knownClassOneOfTheWinners
                                 );

    MLClassPtr  ClassifyAExample (FeatureVector&  example,
                                  double&         probability,
                                  kkint32&        numOfWinners,
                                  bool&           knownClassOneOfTheWinners,
                                  double&         breakTie
                                 );

    /**
     *@brief  For a given two class pair return the names of the 'numToFind' worst S/V's.
     *@details  This method will iterate through all the S/V's removing them one at a 
     *          time and re-compute the decision boundary and probability.  It will then
     *          return the S/V's that when removed improve the probability in 'c1's 
     *          the most.
     *@param[in]  example  The example that was classified incorrectly.
     *@param[in]  numToFind  The number of the worst examples you are looking for.
     *@param[in]  c1  Class that the 'example; parameter should have been classed as.
     *@param[in]  c2  Class that it was classified as.
     */
    std::vector<ProbNamePair>  FindWorstSupportVectors (FeatureVectorPtr  example,
                                                        kkint32           numToFind,
                                                        MLClassPtr        c1,
                                                        MLClassPtr        c2
                                                       );



    /**
     *@brief  For a given two class pair return the names of the 'numToFind' worst S/V's.
     *@details  This method will iterate through all the S/V's removing them one at a 
     *          time and rebuild a new SVM then submit example for testing.
     *@param[in]  example  The example that was classified incorrectly.
     *@param[in]  numToFind  The number of the worst examples you are looking for.
     *@param[in]  c1  Class that the 'example; parameter should have been classed as.
     *@param[in]  c2  Class that it was classified as.
     */
    std::vector<ProbNamePair>  FindWorstSupportVectors2 (FeatureVectorPtr  example,
                                                         kkint32           numToFind,
                                                         MLClassPtr        c1,
                                                         MLClassPtr        c2
                                                       );


    virtual
    kkMemSize  MemoryConsumedEstimated ()  const;


    void  PredictRaw (FeatureVectorPtr  example,
                      MLClassPtr     &  predClass,
                      double&           dist
                     );



    /**
     *@brief  Returns the distribution of the training data used to build the classifier.
     *@details  The caller will NOT own this list.  Ownership will remain with 'trainingProcess'
     * member of this class.
     */
    ClassProbList const *  PriorProbability ()  const;


    /**
     *@brief  For a given feature vector return back the probabilities and votes for each class.
     *@details
     *@param classes       [in]  List of classes that we can be predicted for  The ordering of 'votes' and 'probabilities' will be dictated by this list.
     *@param example       [in]  Feature Vector to make prediction on.
     *@param votes         [out] Pointer to list of int's,  must be as large as 'classes'  The number of votes for each corresponding class will be stored hear.
     *@param probabilities [out] Pointer to list of double's,  must be as large as 'classes'  The probability for each corresponding class will be stored hear.
     */
    void                 ProbabilitiesByClass (const MLClassList&  classes,
                                               FeatureVectorPtr    example,
                                               kkint32*            votes,
                                               double*             probabilities
                                              );

    
    ClassProbListPtr     ProbabilitiesByClass (FeatureVectorPtr  example);


    void                 ProbabilitiesByClassDual (FeatureVectorPtr   example,
                                                   KKStr&             classifier1Desc,
                                                   KKStr&             classifier2Desc,
                                                   ClassProbListPtr&  classifier1Results,
                                                   ClassProbListPtr&  classifier2Results
                                                  );


    void                 RetrieveCrossProbTable (MLClassList&  classes,
                                                 double**      crossProbTable  // two dimension matrix that needs to be classes.QueueSize ()  squared.
                                                );


    std::vector<KKStr>   SupportVectorNames (MLClassPtr  c1,
                                             MLClassPtr  c2
                                            );



  private:
    typedef  std::map<MLClassPtr, Classifier2Ptr>      ClassClassifierIndexType;
    typedef  std::pair<MLClassPtr, Classifier2Ptr>     ClassClassifierPair;
    typedef  std::multimap<Classifier2Ptr,MLClassPtr>  ClassifierClassIndexType;
    typedef  std::pair<Classifier2Ptr,MLClassPtr>      ClassifierClassPair;


    void             BuildSubClassifierIndex ();

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

    Classifier2Ptr  LookUpSubClassifietByClass (MLClassPtr c);

    MLClassListPtr  PredictionsThatHaveSubClassifier (ClassProbListPtr  predictions);

    ClassProbListPtr  ProcessSubClassifersMethod1 (FeatureVectorPtr  example,
                                                   ClassProbListPtr  upperLevelPredictions
                                                  );

    ClassProbListPtr  ProcessSubClassifersMethod2 (FeatureVectorPtr  example,
                                                   ClassProbListPtr  upperLevelPredictions
                                                  );

    ClassProbListPtr  GetListOfPredictionsForClassifier (Classifier2Ptr    classifier,
                                                         ClassProbListPtr  predictions
                                                        );

    bool                     abort;

    ClassClassifierIndexType classClassifierIndex;
    ClassifierClassIndexType classifierClassIndex;

    KKStr                    configRootName;  /**< Name from 'TrainingConfiguration2' instance that was used to build the
                                               * 'TrainingProcess2'instance that this instance will refer to.
                                               */

    bool                     featuresAlreadyNormalized;

    RunLog&                  log;
  
    MLClassListPtr           mlClasses;       /**< We will own the MLClass objects in this
                                               *   list.  Will be originally populated by
                                               *   TrainingConfiguration2 construction.
                                               */
  
    MLClassPtr               noiseMLClass;    /**< Point to class that represents Noise Images
                                               *  The object pointed to will also be included 
                                               *  in mlClasses.
                                               */

    Classifier2ListPtr      subClassifiers;

    ModelPtr                trainedModel;

    ModelOldSVMPtr          trainedModelOldSVM;

    SVMModelPtr             trainedModelSVMModel;

    TrainingProcess2Ptr     trainingProcess;

    MLClassPtr              unKnownMLClass;
  };
  typedef  Classifier2*   Classifier2Ptr;

#define  _Classifier2_Defined_




  class  Classifier2List:  public KKQueue<Classifier2>
  {
  public:
    Classifier2List (bool _owner);
    virtual  ~Classifier2List ();

    Classifier2Ptr  LookUpByName (const KKStr&  rootName)  const;
  };
  typedef  Classifier2List*  Classifier2ListPtr;

#define  _Classifier2List_Defined_



}  /* namespace  KKMLL */


#endif
