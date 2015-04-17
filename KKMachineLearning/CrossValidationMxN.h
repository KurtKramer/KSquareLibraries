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
  #endif


  #ifndef  _MLCLASS_
  class  MLClass;
  typedef  MLClass*       MLClassPtr;
  typedef  MLClass const  MLClassConst;
  typedef  MLClassConst*  MLClassConstPtr;
  class  MLClassList;
  typedef  MLClassList*  MLClassListPtr;
  #endif





  #ifndef  _ConfussionMatrix2_
  class  ConfusionMatrix2;
  typedef  ConfusionMatrix2*  ConfusionMatrix2Ptr;
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
                        bool&                     _cancelFlag
                       );

    CrossValidationMxN (TrainingConfiguration2Ptr _comfig,
                        OrderingsPtr              _data,
                        bool&                     _cancelFlag
                       );

    ~CrossValidationMxN ();

    const
    ConfusionMatrix2Ptr    ConfussionMatrix ()  const;

    void  RunTrainAndTest (kkint32  numExamplsToUseForTraining);

    void  RunValidations ();

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
    void  CheckFileDescCopasetic ();

    void  CleanUpMemory ();

    void  ValidateOrderingIDX (const char*  desc,  
                               kkuint32     idx
                              )  const;


    bool                      cancelFlag;
    TrainingConfiguration2Ptr config;
    FileDescPtr               fileDesc;
    RunLog&                   log;
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
