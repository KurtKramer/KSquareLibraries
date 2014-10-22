#ifndef  _CROSSVALIDATIONVOTING_
#define  _CROSSVALIDATIONVOTING_

//*********************************************************************
//* Written by: Kurt Kramer                                           *
//*        For: Research Work, Plankton recognition System            *
//*                                                                   *
//*-------------------------------------------------------------------*
//*                       CrossValidationVoting                       *
//*                                                                   *
//*-------------------------------------------------------------------*
//*  History                                                          *
//*                                                                   *
//*    Date     Programmer   Description                              *
//*  ---------- -----------  -----------------------------------------*
//*  2004       Kurt Kramer  Origanol Development.                    *
//*                                                                   *
//*                                                                   *
//*  2005-01-07 Kurt Kramer  Added classedCorrectly parameter to      *
//*                          CrossValidate.  If not null it should    *
//*                          point to an array of bool that has as    *
//*                          many elemenst as there are in the        *
//*                          testImages list.  Each element reprsents *
//*                          weather the coresponding element in      *
//*                          testImages was classified correctly.     *
//*                                                                   *
//*********************************************************************


//  2005-01-07
//  Added the parameter classedCorrectly to CrossValidate



#include  "RunLog.h"



namespace  KKMachineLearning 
{

  #ifndef  _FeatureVectorList_Defined_
  class  FeatureVectorList;
  typedef  FeatureVectorList*  FeatureVectorListPtr;
  #endif


  #ifndef  _MLCLASS_
  class  MLClassList;
  typedef  MLClassList*  MLClassListPtr;
  #endif



  #ifndef  _ConfussionMatrix2_
  class  ConfusionMatrix2;
  typedef  ConfusionMatrix2*  ConfusionMatrix2Ptr;
  #endif


  #ifndef  _TrainingConfiguration2Defined_
  class  TrainingConfiguration2;
  typedef  TrainingConfiguration2*  TrainingConfiguration2Ptr;
  class  TrainingConfiguration2List;
  typedef  TrainingConfiguration2List*  TrainingConfigurationList2Ptr;
  #endif


  #ifndef  _FileDesc_Defined_
  class  FileDesc;
  typedef  FileDesc*  FileDescPtr;
  #endif



  class  CrossValidationVoting
  {
  public:
    CrossValidationVoting (TrainingConfigurationList2Ptr  _configs,
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
                         FeatureVectorListPtr   trainingImages,
                         kkint32                foldNum,
                         bool*                  classedCorrectly = NULL
                        );

    void  DeleteAllocatedMemory ();


    TrainingConfigurationList2Ptr configs;
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

} /* namespace KKMachineLearning */

#endif

