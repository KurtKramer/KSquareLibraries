#include "FirstIncludes.h"
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include "MemoryDebug.h"
using namespace std;


#include "KKBaseTypes.h"
#include "OSservices.h"
#include "KKStr.h"
using namespace KKB;


#include "CrossValidationMxN.h"

#include "ConfusionMatrix2.h"
#include "CrossValidation.h"
#include "FeatureVector.h"
#include "Orderings.h"
#include "StatisticalFunctions.h"
#include "TrainingConfiguration2.h"
using namespace KKMLL;


CrossValidationMxN::CrossValidationMxN (TrainingConfiguration2Ptr _config,
                                        kkuint32                  _numOfOrderings,
                                        kkuint32                  _numOfFolds,
                                        FeatureVectorListPtr      _data,
                                        bool&                     _cancelFlag,
                                        RunLog&                   _log
                                       ):

  cancelFlag           (_cancelFlag),
  config               (_config),
  fileDesc             (_config->FileDesc ()),
  meanConfusionMatrix  (NULL),
  numOfFolds           (_numOfFolds),
  numOfOrderings       (_numOfOrderings),
  orderings            (NULL),
  weOwnOrderings       (false),
  
  trainingTimes        (),
  trainingTimeMean     (0.0),
  trainingTimeStdDev   (0.0),
  testTimes            (),
  testTimeMean         (0.0),
  testTimeStdDev       (0.0)
  
{
  CheckFileDescCopasetic (_log);
  orderings = new Orderings (_data, numOfOrderings, _numOfFolds, _log);
  weOwnOrderings = true;
}




CrossValidationMxN::CrossValidationMxN (TrainingConfiguration2Ptr _config,
                                        OrderingsPtr              _orderings,
                                        bool&                     _cancelFlag,
                                        RunLog&                   _log
                                       ):

  cancelFlag           (_cancelFlag),
  config               (_config),
  fileDesc             (_config->FileDesc ()),
  meanConfusionMatrix  (NULL),
  numOfFolds           (_orderings->NumOfFolds ()),
  numOfOrderings       (_orderings->NumOfOrderings ()),
  orderings            (_orderings),
  weOwnOrderings       (false),
  trainingTimes        (),
  trainingTimeMean     (0.0),
  trainingTimeStdDev   (0.0),
  testTimes            (),
  testTimeMean         (0.0),
  testTimeStdDev       (0.0)
{
  CheckFileDescCopasetic (_log);
}




CrossValidationMxN::~CrossValidationMxN ()
{
  CleanUpMemory ();

  if  (weOwnOrderings)
    delete  orderings;
  orderings = NULL;
}



void  CrossValidationMxN::CheckFileDescCopasetic (RunLog&  log)
{
  if  (config->FileDesc () != orderings->FileDesc ())
  {
    // The Configuration 'fileDesc' is different than the orderings 'FileDesc'.
    // This is a VERY VERY bad situation. Processing needs to stop NOW NOW NOW.
    KKStr errMsg = "CrossValidationMxN     ***ERROR***    File Description between config and orderings don't match.";
    log.Level (-1) << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }
}  /* CheckFileDescCopesetic */




void  CrossValidationMxN::CleanUpMemory ()
{
  accuracies.erase      (accuracies.begin      (),  accuracies.end      ());
  supportPoints.erase   (supportPoints.begin   (),  supportPoints.end   ());
  trainingTimes.erase   (trainingTimes.begin   (),  trainingTimes.end   ());
  testTimes.erase       (testTimes.begin       (),  testTimes.end       ());

  delete  meanConfusionMatrix;
  meanConfusionMatrix = NULL;
}  /* CleanUpMemory */




void  CrossValidationMxN::RunValidations (RunLog&  log)
{ 
  CleanUpMemory ();

  meanConfusionMatrix = new ConfusionMatrix2 (*(orderings->MLClasses ()));

  kkuint32  cvIDX = 0;

  MLClassListPtr  mlClasses = orderings->MLClasses ();

  for  (cvIDX = 0;  cvIDX < numOfOrderings;  cvIDX++)
  {
    FeatureVectorListPtr  data = orderings->Ordering (cvIDX);

    CrossValidationPtr  cv = new CrossValidation (config,
                                                  data,
                                                  mlClasses,
                                                  numOfFolds,
                                                  false,
                                                  fileDesc,
                                                  log,
                                                  cancelFlag
                                                 );

    cv->RunCrossValidation (log);

    accuracies.push_back    (cv->Accuracy       ());
    supportPoints.push_back ((float)cv->NumOfSupportVectors ());
    trainingTimes.push_back (cv->TrainTimeTotal ());
    testTimes.push_back     (cv->TestTimeTotal  ());

    meanConfusionMatrix->AddIn (*(cv->ConfussionMatrix ()), log);

    delete  cv;
  }

  CalcMeanAndStdDev (accuracies,      accuracyMean,       accuracyStdDev);
  CalcMeanAndStdDev (supportPoints,   supportPointsMean,  supportPointsStdDev);
  CalcMeanAndStdDev (trainingTimes,   trainingTimeMean,   trainingTimeStdDev);
  CalcMeanAndStdDev (testTimes,       testTimeMean,       testTimeStdDev);

  double  factor = 1.0 / (double)numOfOrderings;

  meanConfusionMatrix->FactorCounts (factor);
}  /* RunValidations */




/**
 * @brief Will run M number of Train then Test passes.
 *        
 * @param[in] numExamplsToUseForTraining The number examples in each ordering(group)
 *            that are to be used for training, the remaining examples will be 
 *            used as test data.
 */
void  CrossValidationMxN::RunTrainAndTest (kkint32  numExamplsToUseForTraining,
                                           RunLog&  log
                                          )
{ 
  CleanUpMemory ();

  meanConfusionMatrix = new ConfusionMatrix2 (*(orderings->MLClasses ()));

  kkuint32  cvIDX = 0;

  MLClassListPtr  mlClasses = orderings->MLClasses ();

  for  (cvIDX = 0;  cvIDX < numOfOrderings;  cvIDX++)
  {
    FeatureVectorListPtr  data = orderings->Ordering (cvIDX);

    FeatureVectorList  trainingData (fileDesc, false);
    FeatureVectorList  testData     (fileDesc, false);

    FeatureVectorList::iterator  fvIDX;

    for  (fvIDX = data->begin ();  fvIDX != data->end ();  fvIDX++)
    {
      FeatureVectorPtr example = *fvIDX;

      if  (trainingData.QueueSize () < numExamplsToUseForTraining)
        trainingData.PushOnBack (example);
      else
        testData.PushOnBack (example);
    }

    CrossValidationPtr  cv = new CrossValidation (config,
                                                  &trainingData,
                                                  mlClasses,
                                                  numOfFolds,
                                                  false,
                                                  fileDesc,
                                                  log,
                                                  cancelFlag
                                                 );

    cv->RunValidationOnly (&testData, 
                           NULL,        // No McNemars test going to be performed.
                           log
                          );

    accuracies.push_back    (cv->Accuracy       ());
    supportPoints.push_back ((float)cv->NumOfSupportVectors ());
    trainingTimes.push_back (cv->TrainTimeTotal ());
    testTimes.push_back     (cv->TestTimeTotal  ());

    meanConfusionMatrix->AddIn (*(cv->ConfussionMatrix ()), log);

    delete  cv;
  }

  CalcMeanAndStdDev (accuracies,      accuracyMean,       accuracyStdDev);
  CalcMeanAndStdDev (supportPoints,   supportPointsMean,  supportPointsStdDev);
  CalcMeanAndStdDev (trainingTimes,   trainingTimeMean,   trainingTimeStdDev);
  CalcMeanAndStdDev (testTimes,       testTimeMean,       testTimeStdDev);

  double  factor = 1.0 / (double)numOfOrderings;

  meanConfusionMatrix->FactorCounts (factor);
}  /* RunTrainAndTest */



ConfusionMatrix2ConstPtr  CrossValidationMxN::ConfussionMatrix ()  const
{
  return  meanConfusionMatrix;
}  /* ConfussionMatrix */
