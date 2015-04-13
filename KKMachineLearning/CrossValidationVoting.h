#ifndef  _CROSSVALIDATIONVOTING_
#define  _CROSSVALIDATIONVOTING_

#include  "RunLog.h"



namespace  KKMLL 
{
  #if  !defined(FeatureVector_Defined_)
  class  FeatureVector;
  typedef  FeatureVector*  FeatureVectorPtr;
  #endif


  #if  !defined(_FeatureVectorList_Defined_)
  class  FeatureVectorList;
  typedef  FeatureVectorList*  FeatureVectorListPtr;
  #endif


  #if  !defined(_MLCLASS_)
  class  MLClass;
  typedef  MLClass*  MLClassPtr;
  typedef  MLClass const  MLClassConst;
  typedef  MLClassConst*  MLClassConstPtr;
  class  MLClassList;
  typedef  MLClassList*  MLClassListPtr;
  #endif



  #if   !defined(_ConfussionMatrix2_Defined_)
  class  ConfusionMatrix2;
  typedef  ConfusionMatrix2*  ConfusionMatrix2Ptr;
  #endif


  #if  !defined(_TrainingConfiguration2_Defined_)
  class  TrainingConfiguration2;
  typedef  TrainingConfiguration2*  TrainingConfiguration2Ptr;
  #endif

  #if  !defined(_TrainingConfiguration2List_Defined_)
  class  TrainingConfiguration2List;
  typedef  TrainingConfiguration2List*  TrainingConfiguration2ListPtr;
  #endif


  #if  !defined(_FileDesc_Defined_)
  class  FileDesc;
  typedef  FileDesc*  FileDescPtr;
  #endif



  class  CrossValidationVoting
  {
  public:
    CrossValidationVoting (TrainingConfiguration2ListPtr  _configs,
                           FeatureVectorListPtr           _examples,
                           MLClassListPtr                 _mlClasses,
                           kkint32                        _numOfFolds,
                           bool                           _featuresAreAlreadyNormalized,
                           FileDescPtr                    _fileDesc,
                           RunLog&                        _log
                          );

    ~CrossValidationVoting ();
    
    void  RunCrossValidation();

    void  RunValidationOnly (FeatureVectorListPtr validationData,
                             bool*                classedCorrectly = NULL
                            );


    ConfusionMatrix2Ptr    ConfussionMatrix ()  {return  confusionMatrix;}

    float   Accuracy                  ();
    double  ClassificationTime        ()  const {return  classificationTime;}
    float   FoldAccuracy (kkint32 foldNum)  const;
    KKStr   FoldAccuracysToStr        ()  const;
    KKStr   FoldStr                   ()  const;
    kkint32 NumOfSupportVectors       ()  const {return  numOfSupportVectors;}
    double  TrainingTime              ()  const {return  trainingTime;}

  private:
    void  AllocateMemory ();

    void  CrossValidate (FeatureVectorListPtr   testImages, 
                         FeatureVectorListPtr   trainingExamples,
                         kkint32                foldNum,
                         bool*                  classedCorrectly = NULL
                        );

    void  DeleteAllocatedMemory ();


    TrainingConfiguration2ListPtr configs;
    bool                          featuresAreAlreadyNormalized;
    FileDescPtr                   fileDesc;
    float*                        foldAccuracies;
    kkint32*                      foldCounts;
    ConfusionMatrix2Ptr           confusionMatrix;
    ConfusionMatrix2Ptr*          cmByNumOfConflicts;
    FeatureVectorListPtr          examples;
    MLClassListPtr                mlClasses;
    kkint32                       examplesPerClass;
    RunLog&                       log;
    kkint32                       maxNumOfConflicts;  /**< Will indicate the number confusionMatrices created in table in cmByNumOfConflicts; */
    kkint32                       numOfFolds;
    kkint32                       numOfSupportVectors;
    kkint32*                      numOfWinnersCounts;
    kkint32*                      numOfWinnersCorrects;
    kkint32*                      numOfWinnersOneOfTheWinners;
    double                        classificationTime;
    double                        trainingTime;
  };


  typedef  CrossValidationVoting*  CrossValidationVotingPtr;

} /* namespace KKMLL */

#endif

