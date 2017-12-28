#if  !defined(_CROSSVALIDATIONMXN_)
#define  _CROSSVALIDATIONMXN_
//*********************************************************************
//* Written by: Kurt Kramer                                           *
//*        For: Research Work, Plankton recognition System            *
//*                                                                   *
//*-------------------------------------------------------------------*
//*                                                                   *
//*********************************************************************

#include  "KKBaseTypes.h"
#include  "RunLog.h"

namespace KKMLL
{
  #if  !defined (_FILEDESC_)
  class  FileDesc;
  typedef  FileDesc*  FileDescPtr;
  typedef  FileDesc const *  FileDescConstPtr;
  #endif


  #ifndef  _MLCLASS_
  class  MLClass;
  typedef  MLClass*      MLClassPtr;
  class  MLClassList;
  typedef  MLClassList*  MLClassListPtr;
  #endif



  #ifndef  _ConfussionMatrix2_
  class  ConfusionMatrix2;
  typedef  ConfusionMatrix2*  ConfusionMatrix2Ptr;
  typedef  ConfusionMatrix2 const *  ConfusionMatrix2ConstPtr;
#endif


  #ifndef  _CROSSVALIDATION_
  class CrossValidation;
  typedef  CrossValidation*  CrossValidationPtr;
  #endif


  #ifndef  _FeatureVector_Defined_
  class  FeatureVector;
  typedef  FeatureVector*  FeatureVectorPtr;
  #endif


  #ifndef  _FeatureVectorList_Defined_
  class  FeatureVectorList;
  typedef  FeatureVectorList*  FeatureVectorListPtr;
  #endif


  #if  !defined(_ORDERINGS_)
  class  Orderings;
  typedef  Orderings*  OrderingsPtr;
  #endif


  #ifndef  _TrainingConfiguration2_Defined_
  class  TrainingConfiguration2;
  typedef  TrainingConfiguration2*  TrainingConfiguration2Ptr;
  #endif


  class CrossValidationMxN
  {
  public:
    CrossValidationMxN (TrainingConfiguration2Ptr _comfig,
                        kkuint32                  _numOfOrderings,
                        kkuint32                  _numOfFolds,
                        FeatureVectorListPtr      _data,
                        bool&                     _cancelFlag,
                        RunLog&                   _log
                       );

    CrossValidationMxN (TrainingConfiguration2Ptr _comfig,
                        OrderingsPtr              _data,
                        bool&                     _cancelFlag,
                        RunLog&                   _log
                       );

    ~CrossValidationMxN ();

    ConfusionMatrix2ConstPtr  ConfussionMatrix ()  const;

    void  RunTrainAndTest (kkint32  numExamplsToUseForTraining,
                           RunLog&  log
                          );

    void  RunValidations (RunLog&  log);

    // Access Methods
    kkint32               NumOfOrderings       () const {return numOfOrderings;}
    kkint32               NumOfFolds           () const {return numOfOrderings;}

    const  VectorFloat&   Accuracies           () const {return accuracies;}
    float                 AccuracyMean         () const {return accuracyMean;}
    float                 AccuracyStdDev       () const {return accuracyStdDev;}

    const VectorFloat&    SupportPoints        () const {return supportPoints;}
    float                 SupportPointsMean    () const {return supportPointsMean;}
    float                 SupportPointsStdDev  () const {return supportPointsStdDev;}

    const VectorDouble&   TestTimes            () const {return testTimes;}
    double                TestTimeMean         () const {return testTimeMean;}
    double                TestTimeStdDev       () const {return testTimeStdDev;}

    const VectorDouble&   TrainingTimes        () const {return trainingTimes;}
    double                TrainingTimeMean     () const {return trainingTimeMean;}
    double                TrainingTimeStdDev   () const {return trainingTimeStdDev;}

  private:
    void  CheckFileDescCopasetic (RunLog&  log);

    void  CleanUpMemory ();


    bool                      cancelFlag;
    TrainingConfiguration2Ptr config;
    FileDescConstPtr          fileDesc;
    ConfusionMatrix2Ptr       meanConfusionMatrix;
    kkuint32                  numOfFolds;
    kkuint32                  numOfOrderings;
    OrderingsPtr              orderings;
    bool                      weOwnOrderings;

    /* Validation Results  */
    VectorFloat   accuracies;
    float         accuracyMean;
    float         accuracyStdDev;

    VectorFloat   supportPoints;
    float         supportPointsMean;
    float         supportPointsStdDev;

    VectorDouble  trainingTimes;
    double        trainingTimeMean;
    double        trainingTimeStdDev;

    VectorDouble  testTimes;
    double        testTimeMean;
    double        testTimeStdDev;
  };
} /* namespace KKMLL */


#endif
