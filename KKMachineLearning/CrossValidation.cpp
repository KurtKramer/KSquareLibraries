#include "FirstIncludes.h"

#include <stdio.h>

#include <iomanip>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
 
#include "MemoryDebug.h"

using namespace std;




#include "KKBaseTypes.h"
#include "OSservices.h"
#include "RunLog.h"
using namespace KKB;

#include "CrossValidation.h"

#include "Classifier2.h"
#include "ConfusionMatrix2.h"
#include "FileDesc.h"
#include "MLClass.h"
#include "FeatureVector.h"
#include "StatisticalFunctions.h"
#include "TrainingConfiguration2.h"
#include "TrainingProcess2.h"
using namespace  KKMachineLearning;



CrossValidation::CrossValidation (TrainingConfiguration2Ptr  _config,
                                  FeatureVectorListPtr      _examples,
                                  MLClassListPtr         _mlClasses,
                                  kkint32                     _numOfFolds,
                                  bool                      _featuresAreAlreadyNormalized,
                                  FileDescPtr               _fileDesc,
                                  RunLog&                   _log,
                                  bool&                     _cancelFlag
                                 ):

   cancelFlag                   (_cancelFlag),
   config                       (_config),
   duplicateTrainDataCount      (0),
   featuresAreAlreadyNormalized (_featuresAreAlreadyNormalized),
   fileDesc                     (_fileDesc),
   foldAccuracies               (),
   foldCounts                   (),
   confusionMatrix              (NULL),
   cmByNumOfConflicts           (NULL),
   examples                     (new FeatureVectorList (*_mlClasses, *_examples, _log)),
   mlClasses                 (_mlClasses),
   imagesPerClass               (0),
   log                          (_log),
   maxNumOfConflicts            (0),
   numOfFolds                   (_numOfFolds),
   numSVs                       (0),
   totalNumSVs                  (0),
   numOfWinnersCounts           (NULL),
   numOfWinnersCorrects         (NULL),
   numOfWinnersOneOfTheWinners  (NULL),
   testTime                     (0.0),
   trainingTime                 (0.0),

   accuracyMean                 (0.0f),
   accuracyStdDev               (0.0f),

   avgPredProb                  (0.0),
   totalPredProb                (0.0),

   supportPointsMean            (0.0f),
   supportPointsStdDev          (0.0f),
   supportPoints                (),

   testTimeMean                 (0.0),
   testTimeStdDev               (0.0),
   testTimes                    (),

   trainTimeMean                (0.0),
   trainTimeStdDev              (0.0),
   trainTimes                   (),

   weOwnConfusionMatrix         (false)

{
  if  (config)
    imagesPerClass = config->ExamplesPerClass ();
  else
     imagesPerClass = -1;
}



CrossValidation::~CrossValidation ()
{
  DeleteAllocatedMemory ();
  delete  examples;  examples = NULL;
}
  
  


void  CrossValidation::AllocateMemory ()
{
  maxNumOfConflicts = mlClasses->QueueSize () + 1;
  confusionMatrix = new ConfusionMatrix2 (*mlClasses, log);
  weOwnConfusionMatrix = true;
  cmByNumOfConflicts = new ConfusionMatrix2Ptr[maxNumOfConflicts];

  numOfWinnersCounts          = new kkint32[maxNumOfConflicts];
  numOfWinnersCorrects        = new kkint32[maxNumOfConflicts];
  numOfWinnersOneOfTheWinners = new kkint32[maxNumOfConflicts];

  kkint32  conflictIDX;

  for  (conflictIDX = 0;  conflictIDX < maxNumOfConflicts;  conflictIDX++)
  {
    cmByNumOfConflicts          [conflictIDX] = new ConfusionMatrix2 (*mlClasses, log);
    numOfWinnersCounts          [conflictIDX] = 0;
    numOfWinnersCorrects        [conflictIDX] = 0;
    numOfWinnersOneOfTheWinners [conflictIDX] = 0;
  }

  //foldAccuracies  = new float[numOfFolds];  // Changed to vector<float>  aka  VectorFloat
  //foldCounts      = new kkint32[numOfFolds];    // Changed to vector<kkint32>    aka  VectorInt
}  /* AllocateMemory */




void  CrossValidation::DeleteAllocatedMemory ()
{
  if  (weOwnConfusionMatrix)
  {
    delete confusionMatrix;
    confusionMatrix = NULL;
  }

  kkint32  conflictIDX;

  if  (cmByNumOfConflicts)
  {
    for  (conflictIDX = 0;  conflictIDX < maxNumOfConflicts;  conflictIDX++)
    {
      delete  cmByNumOfConflicts[conflictIDX];
      cmByNumOfConflicts[conflictIDX] = NULL;
    }
 
    delete  cmByNumOfConflicts;
    cmByNumOfConflicts = NULL;
  }

  // delete  foldAccuracies;               foldAccuracies = NULL;
  // delete  foldCounts;                   foldCounts     = NULL;
  
  delete  numOfWinnersCounts;           numOfWinnersCounts          = NULL;
  delete  numOfWinnersCorrects;         numOfWinnersCorrects        = NULL;
  delete  numOfWinnersOneOfTheWinners;  numOfWinnersOneOfTheWinners = NULL;

}  /* DeleteAllocatedMemory */



void  CrossValidation::RunCrossValidation ()
{
  log.Level (10) << "CrossValidation::RunCrossValidation    numOfFolds[" << numOfFolds << "]" << endl;

  if  (numOfFolds < 1)
  {
    log.Level (-1) << endl
                   << endl
                   << "CrossValidation::RunCrossValidation     **** ERROR ****" << endl
                   << endl
                   << "                                  Invalid  numOfFolds[" << numOfFolds << "]." << endl
                   << endl;
    return;
  }

  DeleteAllocatedMemory ();
  AllocateMemory ();

  kkint32  imageCount       = examples->QueueSize ();
  kkint32  numImagesPerFold = (imageCount + numOfFolds - 1) / numOfFolds;
  kkint32  firstInGroup     = 0;

  totalPredProb = 0.0;


  kkint32  foldNum;

  for  (foldNum = 0;  foldNum < numOfFolds;  foldNum++)
  {
    kkint32  lastInGroup;

    // If We are doing the last Fold Make sure that we are including all the examples 
    // that have not been tested.
    if  (foldNum == (numOfFolds - 1))
      lastInGroup = imageCount;
    else
      lastInGroup = firstInGroup + numImagesPerFold - 1;


    log.Level (20) << "Fold [" << (foldNum + 1) << "]  of  [" << numOfFolds << "]" << endl;

    FeatureVectorListPtr  trainingImages = new FeatureVectorList (fileDesc, true, log);

    FeatureVectorListPtr  testImages     = new FeatureVectorList (fileDesc, true, log);

    log.Level (30) << "Fold Num["        << foldNum        << "]   "
                   << "FirstTestImage["  << firstInGroup   << "]   "
                   << "LastInGroup["     << lastInGroup    << "]."
                   << endl;

    for  (kkint32  x = 0; (x < imageCount)  &&  (!cancelFlag); x++)
    {
      FeatureVectorPtr  newImage = new FeatureVector (*(examples->IdxToPtr (x)));

      if  ((x >= firstInGroup)  &&  (x <= lastInGroup))
      {
        testImages->PushOnBack (newImage);
      }
      else
      {
        trainingImages->PushOnBack (newImage);
      }
    }

    log.Level (20) << "Number Of Training Images : " << trainingImages->QueueSize () << endl;
    log.Level (20) << "Number Of Test Images     : " << testImages->QueueSize ()     << endl;

    if  (cancelFlag)
      break;
    
    CrossValidate (testImages, trainingImages, foldNum);

    delete  trainingImages;
    delete  testImages;
    firstInGroup = firstInGroup + numImagesPerFold;
  }

  if  (!cancelFlag)
  {
    avgPredProb = totalPredProb / imageCount;

    CalcMeanAndStdDev (foldAccuracies,  accuracyMean,       accuracyStdDev);
    CalcMeanAndStdDev (supportPoints,   supportPointsMean,  supportPointsStdDev);
    CalcMeanAndStdDev (testTimes,       testTimeMean,       testTimeStdDev);
    CalcMeanAndStdDev (trainTimes,      trainTimeMean,      trainTimeStdDev);
  }
}  /* RunCrossValidation */





void  CrossValidation::RunValidationOnly (FeatureVectorListPtr validationData,
                                          bool*                classedCorrectly
                                         )
{
  log.Level (10) << "CrossValidation::RunValidationOnly" << endl;
  DeleteAllocatedMemory ();
  AllocateMemory ();

  totalPredProb = 0.0;

  // We need to get a duplicate copy of each image data because the trainer and classifier
  // will normalize the data.
  FeatureVectorListPtr  trainingImages = examples->DuplicateListAndContents ();
  FeatureVectorListPtr  testImages     = validationData->DuplicateListAndContents ();

  CrossValidate (testImages, trainingImages, 0, classedCorrectly);

  if  (testImages->QueueSize () > 0)
    avgPredProb = totalPredProb / testImages->QueueSize ();
  else
    avgPredProb = 0.0f;

  delete  trainingImages;  trainingImages = NULL;
  delete  testImages;      testImages     = NULL;


  if  (!cancelFlag)
  {
    CalcMeanAndStdDev (foldAccuracies,  accuracyMean,       accuracyStdDev);
    CalcMeanAndStdDev (supportPoints,   supportPointsMean,  supportPointsStdDev);
    CalcMeanAndStdDev (testTimes,       testTimeMean,       testTimeStdDev);
    CalcMeanAndStdDev (trainTimes,      trainTimeMean,      trainTimeStdDev);
  }
}  /* RunValidationOnly */




void  CrossValidation::CrossValidate (FeatureVectorListPtr   testImages, 
                                      FeatureVectorListPtr   trainingImages,
                                      kkint32                foldNum,
                                      bool*                  classedCorrectly
                                     )
{
  log.Level (20) << "CrossValidation::CrossValidate   FoldNum[" << foldNum  << "]." << endl;

  KKStr  statusMessage;

  TrainingProcess2  trainer (config, 
                             trainingImages, 
                             mlClasses,
                             NULL,
                             config->FvFactoryProducer (),
                             log,
                             featuresAreAlreadyNormalized,
                             cancelFlag,
                             statusMessage
                            );
  try
  {
    trainer.CreateModelsFromTrainingData ();
  }
  catch  (const std::exception  e)
  {
    log.Level (-1) << endl << endl
      << "CrossValidation::CrossValidat  ***ERROR***  Exception Occurred in 'CreateModelsFromTrainingData'"  << endl
      << "            Exception[" << e.what () << "]" << endl
      << endl;
    trainer.Abort (true);
  }
  catch (...)
  {
    log.Level (-1) << endl << endl
      << "CrossValidation::CrossValidat  ***ERROR***  Exception Occurred in 'CreateModelsFromTrainingData'"  << endl
      << endl;
    trainer.Abort (true);
  }

  if  (trainer.Abort ())
    return;

  duplicateTrainDataCount += trainer.DuplicateDataCount ();
  trainingTime            += trainer.TrainingTime ();

  kkint32  foldNumSVs = 0;
  kkint32  foldTotalNumSVs = 0;
  trainer.SupportVectorStatistics (foldNumSVs, foldTotalNumSVs);
  numSVs      += foldNumSVs;
  totalNumSVs += foldTotalNumSVs;
    
  log.Level (20) << "CrossValidate   Creating Classification Object" << endl;

  Classifier2  classifier (&trainer, log);
  {
    // Make sure that a Noise class exists
    mlClasses->GetNoiseClass ();
  }

  log.Level (20) << "CrossValidate   Classifying Test Images." << endl;

  double            breakTie                   = 0.0f;
  FeatureVectorPtr  example                    = NULL;
  MLClassPtr        knownClass                 = NULL;
  bool              knownClassOneOfTheWinners  = false;
  kkint32           numOfWinners               = 0;
  MLClassPtr        predictedClass             = NULL;
  double            probability                = 0.0f;

  kkint32  numTestExamples = testImages->QueueSize ();

  kkint32  foldCorrect = 0;
  kkint32  foldCount   = 0;

  vector<FeatureVectorPtr>  exampleHist                   (numTestExamples);
  vector<MLClassPtr>        knownClassHist                (numTestExamples);
  vector<bool>              knownClassOneOfTheWinnersHist (numTestExamples, false);
  vector<kkint32>           numOfWinersHist               (numTestExamples, 0);
  vector<MLClassPtr>        predictedClassHist            (numTestExamples);
  vector<double>            probabilityHist               (numTestExamples, 0.0f);

  FeatureVectorList::iterator  exampleIDX;

  double  startClassificationTime = osGetSystemTimeUsed ();

  for  (exampleIDX = testImages->begin ();  (exampleIDX != testImages->end ())  &&  (!cancelFlag);  exampleIDX++)
  {
    example = *exampleIDX;

    knownClass = example->MLClass ();

    predictedClass =  classifier.ClassifyAImage (*example, 
                                                 probability, 
                                                 numOfWinners,
                                                 knownClassOneOfTheWinners,
                                                 breakTie
                                                );

    exampleHist                   [foldCount] = example;
    knownClassHist                [foldCount] = knownClass;
    predictedClassHist            [foldCount] = predictedClass;
    probabilityHist               [foldCount] = probability;
    numOfWinersHist               [foldCount] = numOfWinners;
    knownClassOneOfTheWinnersHist [foldCount] = knownClassOneOfTheWinners;

    foldCount++;
  }

  double  endClassificationTime = osGetSystemTimeUsed ();
  double  testTimeThisFold = (endClassificationTime - startClassificationTime);
  testTime += testTimeThisFold;


  // lets update statistics
  foldCount = 0;
  for  (foldCount = 0;  (foldCount < numTestExamples)  &&  (!cancelFlag);  foldCount++)
  {
    example                    = exampleHist                   [foldCount];
    predictedClass             = predictedClassHist            [foldCount];
    probability                = probabilityHist               [foldCount];
    numOfWinners               = numOfWinersHist               [foldCount];
    knownClass                 = knownClassHist                [foldCount];
    knownClassOneOfTheWinners  = knownClassOneOfTheWinnersHist [foldCount];


    totalPredProb += probability;


    confusionMatrix->Increment (knownClass, 
                                predictedClass, 
                                (kkint32)(example->OrigSize ()),
                                probability
                               );

    cmByNumOfConflicts[numOfWinners]->Increment (knownClass, 
                                                 predictedClass, 
                                                 (kkint32)(example->OrigSize ()), 
                                                 probability
                                                );

    bool  correctClassificationMade = false;
    numOfWinnersCounts[numOfWinners]++;
    if  (knownClass == predictedClass)
    {
      correctClassificationMade = true;
      numOfWinnersCorrects[numOfWinners]++;
      foldCorrect++;
    }

    if  (classedCorrectly)
    {
      classedCorrectly[foldCount] = correctClassificationMade;
    }

    if  (knownClassOneOfTheWinners)
       numOfWinnersOneOfTheWinners[numOfWinners]++;
  }

  float  foldAccuracy = 0.0;

  if  (foldCount > 0)
    foldAccuracy = 100.0f * (float)foldCorrect / (float)foldCount;

  foldAccuracies.push_back  (foldAccuracy);
  foldCounts.push_back      (foldCount);
 
  supportPoints.push_back   ((float)trainer.NumOfSupportVectors ());
  trainTimes.push_back      (trainer.TrainingTime ());
  testTimes.push_back       (testTimeThisFold);

  log.Level (20) << "CrossValidation::CrossValidate - Done." << endl;
}  /* CrossValidate */



float  CrossValidation::Accuracy ()
{
  if  (confusionMatrix)
    return  (float)confusionMatrix->Accuracy ();
  else
    return 0.0f;
}  /* Accuracy */



float  CrossValidation::AccuracyNorm ()
{
  if  (confusionMatrix)
    return  (float)confusionMatrix->AccuracyNorm ();
  else
    return 0.0f;
}  /* Accuracy */




KKStr  CrossValidation::FoldAccuracysToStr ()  const
{
  if  (kkint32 (foldAccuracies.size ()) != numOfFolds)
  {
    log.Level (-1) << endl
                   << endl
                   << "CrossValidation::FoldAccuracysToStr      *** ERROR ***" << endl
                   << endl
                   << "  *** foldAccuracies.size () != numOfFolds ***" << endl
                   << endl;
    return "";
    //osWaitForEnter ();
    //exit (-1);
  }

  KKStr  foldAccuracyStr (9 * numOfFolds);  // Pre Reserving enough space for all Accuracies.

  for  (kkuint32 foldNum = 0;  foldNum < foldAccuracies.size ();  foldNum++)
  {
    if  (foldNum > 0)
      foldAccuracyStr << "\t";
    foldAccuracyStr << StrFormatDouble (foldAccuracies[foldNum], "ZZ,ZZ0.00%");
  }

  return  foldAccuracyStr;
}  /* FoldAccuracysToStr */





float  CrossValidation::FoldAccuracy (kkint32 foldNum)  const
{
  if  ((foldNum < 0)  ||  (foldNum >= (kkint32)foldAccuracies.size ()))
  {
    log.Level (-1) << endl
                   << endl
                   << "CrossValidation::FoldAccuracy      *** ERROR ***" << endl
                   << endl
                   << "                                   FoldNum[" << foldNum << "] Is Out Of Range." << endl
                   << endl;
    osWaitForEnter ();
    exit (-1);
  }

  return  foldAccuracies[foldNum];
}  /* FoldAccuracy */




ConfusionMatrix2Ptr  CrossValidation::GiveMeOwnershipOfConfusionMatrix ()
{
  weOwnConfusionMatrix = false;
  return  confusionMatrix;
}
