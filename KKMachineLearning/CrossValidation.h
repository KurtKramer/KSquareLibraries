#ifndef  _CROSSVALIDATION_
#define  _CROSSVALIDATION_

/**
 @class  KKMLL::CrossValidation
 @brief  A class that is meant to manage a n-Fold Cross Validation.
 @author  Kurt Kramer
 @details
 @code
 *********************************************************************
 *                           CrossValidation                         *
 *                                                                   *
 *                                                                   *
 *                                                                   *
 *-------------------------------------------------------------------*
 *  History                                                          *
 *                                                                   *
 *    Date     Programmer   Description                              *
 *  ---------- -----------  -----------------------------------------*
 *  2004       Kurt Kramer  Original Development.                    *
 *                                                                   *
 *                                                                   *
 *  2005-01-07 Kurt Kramer  Added classedCorrectly parameter to      *
 *                          CrossValidate. If not null it should     *
 *                          point to an array of bool that has as    *
 *                          many elements as there are in the        *
 *                          testImages list. Each element represents *
 *                          weather the corresponding element in     *
 *                          testImages was classified correctly.     *
 *********************************************************************
 @endcode
 */


#include  "KKBaseTypes.h"
#include  "RunLog.h"


namespace  KKMLL  
{
  #ifndef  _FeatureVector_Defined_
  class  FeatureVectorList;
  typedef  FeatureVectorList*  FeatureVectorListPtr;
  #endif


  #if  !defined (_MLCLASS_)
  class  MLClass;
  typedef  MLClass*  MLClassPtr;
  class  MLClassList;
  typedef  MLClassList*  MLClassListPtr;
  #endif


  #ifndef  _ConfussionMatrix2_
  class  ConfusionMatrix2;
  typedef  ConfusionMatrix2*  ConfusionMatrix2Ptr;
  typedef  ConfusionMatrix2 const *  ConfusionMatrix2ConstPtr;
  #endif


  #if  !defined(_FactoryFVProducer_Defined_)
  class  FactoryFVProducer;
  typedef  FactoryFVProducer*  FactoryFVProducerPtr;
  #endif


  #ifndef  _TrainingConfiguration2_Defined_
  class  TrainingConfiguration2;
  typedef  TrainingConfiguration2*  TrainingConfiguration2Ptr;
  #endif


  #ifndef  _FILEDESC_
  class  FileDesc;
  typedef  FileDesc*  FileDescPtr;
  typedef  FileDesc const *  FileDescConstPtr;
  #endif


  class  CrossValidation
  {
  public:
    CrossValidation (TrainingConfiguration2Ptr _config,
                     FeatureVectorListPtr      _examples,
                     MLClassListPtr            _mlClasses,
                     kkuint32                 _numOfFolds,
                     bool                      _featuresAreAlreadyNormalized,
                     FileDescConstPtr          _fileDesc,
                     RunLog&                   _log,
                     KKB::VolConstBool&        _cancelFlag
                    );

    ~CrossValidation ();
    
    void  RunCrossValidation (RunLog&  log);

    void  RunValidationOnly (FeatureVectorListPtr validationData,
                             bool*                classedCorrectly,
                             RunLog&              log
                            );

    ConfusionMatrix2ConstPtr    ConfussionMatrix () const  {return  confusionMatrix;}

    float         Accuracy     ();
    float         AccuracyNorm ();
    kkint32       DuplicateTrainDataCount () const {return  duplicateTrainDataCount;}
    float         FoldAccuracy (kkuint32 foldNum) const;

    void          NumOfFolds (kkuint32 _numOfFolds)  {numOfFolds = _numOfFolds;}

    const
    VectorFloat&  FoldAccuracies          () const {return  foldAccuracies;}

    KKStr         FoldAccuracysToStr      () const;

    ConfusionMatrix2Ptr  GiveMeOwnershipOfConfusionMatrix ();

    kkint32       NumOfSupportVectors        () const {return  numSVs;}
    kkint32       NumSVs                     () const {return  numSVs;}
    kkint32       TotalNumSVs                () const {return  totalNumSVs;}

    kkint32       SupportPointsTotal         () const {return  numSVs;}
    
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
                         FeatureVectorListPtr   trainingExamples,
                         kkint32                foldNum,
                         bool*                  classedCorrectly,
                         RunLog&                log
                        );

    void  DeleteAllocatedMemory ();

    //void  DistributesImagesRandomlyFromEachWithInFolds ();

    VolConstBool&             cancelFlag;
    TrainingConfiguration2Ptr config;
    kkint32                   duplicateTrainDataCount;
    FactoryFVProducerPtr      fvProducerFactory;
    bool                      featuresAreAlreadyNormalized;
    FileDescConstPtr          fileDesc;
    VectorFloat               foldAccuracies;
    VectorInt                 foldCounts;
    ConfusionMatrix2Ptr       confusionMatrix;
    ConfusionMatrix2Ptr*      cmByNumOfConflicts;
    FeatureVectorListPtr      examples;
    MLClassListPtr            mlClasses;
    kkint32                   imagesPerClass;
    kkint32                   maxNumOfConflicts;  /**< Will indicate the number confusionMatrices created in table in cmByNumOfConflicts; */
    kkuint32                  numOfFolds;

    kkint32                   numSVs;             /**< Total Support Vectors Detected. */

    kkint32                   totalNumSVs;        /**< This is different from 'numOfSupportVectors' it will reflect all the Support Vectors
                                                   * that are created in a Multi Class SVM. That is if a given example is used in three
                                                   * different binary classifiers it will be counted three times.
                                                   */

    kkint32*                  numOfWinnersCounts;
    kkint32*                  numOfWinnersCorrects;
    kkint32*                  numOfWinnersOneOfTheWinners;

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

}  /* namespace  KKMLL */

#endif
