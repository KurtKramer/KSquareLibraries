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
                                        bool&                     _cancelFlag
                                       ):

  cancelFlag           (_cancelFlag),
  config               (_config),
  fileDesc             (_config->FileDesc ()),
  log                  (_data->Log ()),
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
  CheckFileDescCopasetic ();
  orderings = new Orderings (_data, numOfOrderings, _numOfFolds);
  weOwnOrderings = true;
}




CrossValidationMxN::CrossValidationMxN (TrainingConfiguration2Ptr _config,
                                        OrderingsPtr             _orderings,
                                        bool&                    _cancelFlag
                                       ):

  cancelFlag           (_cancelFlag),
  config               (_config),
  fileDesc             (_config->FileDesc ()),
  log                  (_orderings->Log ()),
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
  CheckFileDescCopasetic ();
}




CrossValidationMxN::~CrossValidationMxN ()
{
  CleanUpMemory ();

  if  (weOwnOrderings)
    delete  orderings;
  orderings = NULL;
}



void  CrossValidationMxN::CheckFileDescCopasetic ()
{
  if  (config->FileDesc () != orderings->FileDesc ())
  {
    // The Configuration 'fileDesc' is different than the orderings 'FileDesc'.
    // This is a VERY VERY bad situation. Processing needs to stop NOW NOW NOW.
    log.Level (-1) << endl << endl
                   << "CrossValidationMxN     *** ERROR ***    File Description Mismatch." << endl
                   << endl;
    osWaitForEnter ();
    exit (-1);
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




void  CrossValidationMxN::RunValidations ()
{ 
  CleanUpMemory ();

  meanConfusionMatrix = new ConfusionMatrix2 (*(orderings->MLClasses ()), log);

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

    cv->RunCrossValidation ();

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
void  CrossValidationMxN::RunTrainAndTest (kkint32  numExamplsToUseForTraining)
{ 
  CleanUpMemory ();

  meanConfusionMatrix = new ConfusionMatrix2 (*(orderings->MLClasses ()), log);

  kkuint32  cvIDX = 0;

  MLClassListPtr  mlClasses = orderings->MLClasses ();

  for  (cvIDX = 0;  cvIDX < numOfOrderings;  cvIDX++)
  {
    FeatureVectorListPtr  data = orderings->Ordering (cvIDX);

    FeatureVectorList  trainingData (fileDesc, false, log);
    FeatureVectorList  testData     (fileDesc, false, log);

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
                           NULL        // No McNemars test going to be performed.
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





void  CrossValidationMxN::ValidateOrderingIDX (const char*  desc,  
                                               kkuint32     idx
                                              )  const
{
  if  (!orderings)
  {
    log.Level (-1) << endl << endl
                   << desc << "   *** ERROR ***   There is not Orderings structure." 
                   << endl;
    osWaitForEnter ();
    exit (-1);
  }

  if  (idx >= orderings->Size ())
  {
    log.Level (-1) << endl << endl
                   << desc << "   *** ERROR ***   Orderings Idx[" << idx << "] out of range[" << orderings->Size () << "]." 
                   << endl;
    osWaitForEnter ();
    exit (-1);
  }
}  /* ValidateOderingIDX */




const
ConfusionMatrix2Ptr  CrossValidationMxN::ConfussionMatrix ()  const
{
  return  meanConfusionMatrix;
}  /* ConfussionMatrix */
