#ifndef  _CROSSVALIDATION_
#define  _CROSSVALIDATION_

/**
 *@class  KKMachineLearning::CrossValidation
 *@brief  A class that is meant to manage a n-Fold Cross Validation.
 *@author  Kurt Kramer
 *@details
 */


#include  "KKBaseTypes.h"
#include  "RunLog.h"


namespace  KKMachineLearning  
{

  #ifndef  _FeatureVector_Defined_
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
  #endif


  #ifndef  _FILEDESC_
  class  FileDesc;
  typedef  FileDesc*  FileDescPtr;
  #endif



  class  CrossValidation
  {
  public:
    CrossValidation (TrainingConfiguration2Ptr _config,
                     FeatureVectorListPtr      _examples,
                     MLClassListPtr         _mlClasses,
                     int32                     _numOfFolds,
                     bool                      _featuresAreAlreadyNormalized,
                     FileDescPtr               _fileDesc,
                     RunLog&                   _log,
                     bool&                     _cancelFlag
                    );

    ~CrossValidation ();
    
    void  RunCrossValidation ();

    void  RunValidationOnly (FeatureVectorListPtr validationData,
                             bool*                classedCorrectly = NULL
                            );

    const
    ConfusionMatrix2Ptr    ConfussionMatrix () const  {return  confusionMatrix;}

    float         Accuracy     ();
    float         AccuracyNorm ();
    int32         DuplicateTrainDataCount () const {return  duplicateTrainDataCount;}
    float         FoldAccuracy (int32 foldNum) const;

    void          NumOfFolds (int32 _numOfFolds)  {numOfFolds = _numOfFolds;}

    const
    VectorFloat&  FoldAccuracies          () const {return  foldAccuracies;}

    KKStr         FoldAccuracysToStr      () const;

    ConfusionMatrix2Ptr  GiveMeOwnershipOfConfusionMatrix ();

    int32         NumOfSupportVectors        () const {return  numSVs;}
    int32         NumSVs                     () const {return  numSVs;}
    int32         TotalNumSVs                () const {return  totalNumSVs;}
    int32         SupportPointsTotal         () const {return  numSVs;}
    
    const VectorFloat&   Accuracies          () const {return foldAccuracies;}
    float                AccuracyMean        () const {return accuracyMean;}
    float                AccuracyStdDev      () const {return accuracyStdDev;}

    double               AvgPredProb         () const {return avgPredProb;}

    const VectorFloat&   SupportPoints       () const {return supportPoints;}
    double               SupportPointsMean   () const {return supportPointsMean;}
    double               SupportPointsStdDev () const {return supportPointsStdDev;}


    const VectorDouble&  TestTimes       () const {return testTimes;}
    double               TestTimeMean    () const {return testTimeMean;}
    double               TestTimeStdDev  () const {return testTimeStdDev;}
    double               TestTimeTotal   () const {return testTime;}

    const VectorDouble&  TrainTimes      () const {return trainTimes;}
    double               TrainTimeMean   () const {return trainTimeMean;}
    double               TrainTimeStdDev () const {return trainTimeStdDev;}
    double               TrainTimeTotal  () const {return  trainingTime;}



  private:
    void  AllocateMemory ();

    void  CrossValidate (FeatureVectorListPtr   testImages, 
                         FeatureVectorListPtr   trainingImages,
                         int32                  foldNum,
                         bool*                  classedCorrectly = NULL
                        );

    void  DeleteAllocatedMemory ();

    //void  DistributesImagesRandomlyFromEachWithInFolds ();


    bool                      cancelFlag;
    TrainingConfiguration2Ptr config;
    int32                     duplicateTrainDataCount;
    bool                      featuresAreAlreadyNormalized;
    FileDescPtr               fileDesc;
    VectorFloat               foldAccuracies;
    VectorInt                 foldCounts;
    ConfusionMatrix2Ptr       confusionMatrix;
    ConfusionMatrix2Ptr*      cmByNumOfConflicts;
    FeatureVectorListPtr      examples;
    MLClassListPtr         mlClasses;
    int32                     imagesPerClass;
    RunLog&                   log;
    int32                     maxNumOfConflicts;  /**< Will indicate the number confusionMatrices created in table in cmByNumOfConflicts; */
    int32                     numOfFolds;

    int32                     numSVs;             /**< Total Support Vectors Detected. */

    int32                     totalNumSVs;        /**< This is different from 'numOfSupportVectors' it will reflect all the Support Vectors
                                                   * that are created in a Multi Class SVM.  That is if a given example is used in three
                                                   * different binary classifiers it will be counted three times.
                                                   */

    int32*                    numOfWinnersCounts;
    int32*                    numOfWinnersCorrects;
    int32*                    numOfWinnersOneOfTheWinners;

    double                    testTime;
    double                    trainingTime;

    float                     accuracyMean;
    float                     accuracyStdDev;

    double                    avgPredProb;
    double                    totalPredProb;

    float                     supportPointsMean;
    float                     supportPointsStdDev;
    VectorFloat               supportPoints;

    double                    testTimeMean;
    double                    testTimeStdDev;
    VectorDouble              testTimes;

    double                    trainTimeMean;
    double                    trainTimeStdDev;
    VectorDouble              trainTimes;

    bool                      weOwnConfusionMatrix;
  };


  typedef  CrossValidation*  CrossValidationPtr;

}  /* namespace  KKMachineLearning */

#endif

