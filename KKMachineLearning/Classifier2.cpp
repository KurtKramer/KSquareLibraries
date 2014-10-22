#include  "FirstIncludes.h"

//*******************************************************************
//*                           Classifier2                            *
//*******************************************************************

#include <ctype.h>
#include <stdio.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "MemoryDebug.h"


#ifdef WIN32
#include <windows.h>
#else
#endif

using namespace  std;


#include "KKBaseTypes.h"
#include "KKException.h"
#include "RunLog.h"
using namespace  KKB;


#include "Classifier2.h"
#include "MLClass.h"
#include "NormalizationParms.h"
#include "TrainingProcess2.h"
using namespace  KKMachineLearning;



Classifier2::Classifier2 (TrainingProcess2Ptr  _trainer,
                          RunLog&              _log
                         ):

  abort                     (false),
  featuresAlreadyNormalized (false),
  mlClasses              (NULL),
  log                       (_log),
  noiseImageClass           (NULL),
  trainedModel              (NULL),
  trainedModelOldSVM        (NULL),
  trainedModelSVMModel      (NULL),
  trainingProcess           (_trainer),
  unKnownImageClass         (NULL)
{
  if  (!_trainer)
  {
    log.Level (-1) << endl
      << "Classifier2::Classifier2    ***ERROR***     (_trainer == NULL)" << endl
      << endl;
    throw KKException ("Classifier2::Classifier2    ***ERROR***     (_trainer == NULL)");
  }


  if  (_trainer->Abort ())
  {
    log.Level (-1) << endl
      << "Classifier2::Classifier2    ***ERROR***     '_trainer' is invalid." << endl
      << endl;
    throw KKException ("Classifier2::Classifier2    ***ERROR***     '_trainer' is invalid.");
  }

  featuresAlreadyNormalized = trainingProcess->FeaturesAlreadyNormalized ();
  mlClasses = new MLClassList (*(_trainer->ImageClasses ()));
  trainedModel = _trainer->TrainedModel ();
  if  (trainedModel == NULL)
  {
    KKStr errMsg = "Classifier2::Classifier2    ***ERROR***     (trainedModel == NULL).";
    log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  if  (!trainedModel->ValidModel ())
  {
    KKStr errMsg = "Classifier2::Classifier2    ***ERROR***     trainedModel is not valid.";
    log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  log.Level (20) << "Classifier2::Classifier2" << endl;
  noiseImageClass   = mlClasses->GetNoiseClass ();
  unKnownImageClass = mlClasses->GetUnKnownClass ();

  if  (trainedModel->ModelType () == Model::mtOldSVM)
  {
    trainedModelOldSVM = dynamic_cast<ModelOldSVMPtr> (trainedModel);
    if  (trainedModelOldSVM)
      trainedModelSVMModel = trainedModelOldSVM->SvmModel ();
  }
}



Classifier2::~Classifier2 ()
{
  delete  mlClasses;
}


kkint32  Classifier2::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = sizeof (*this);
  if  (mlClasses)  memoryConsumedEstimated += mlClasses->MemoryConsumedEstimated ();
  return  memoryConsumedEstimated;
}  /* MemoryConsumedEstimated */



SVM_SelectionMethod   Classifier2::SelectionMethod ()  const
{
  if  (!trainedModelOldSVM)
    return   SelectionMethod_NULL;
  else
    return  trainedModelOldSVM->SelectionMethod ();
}





MLClassPtr  Classifier2::ClassifyAImageOneLevel (FeatureVector&  example,
                                                 double&         probability,
                                                 kkint32&        numOfWinners, 
                                                 bool&           knownClassOneOfTheWinners,
                                                 double&         breakTie
                                                )

{
  probability       = 0.0;

  double  probOfKnownClass = 0.0f;

  MLClassPtr  origClass       = example.MLClass ();
  MLClassPtr  predictedClass  = NULL;
  MLClassPtr  predictedClass2 = NULL;

  kkint32        class1Votes = -1;
  kkint32        class2Votes = -1;

  double         predictedClass2Prob = 0.0f;

  trainedModel->Predict (&example, 
                   origClass,
                   predictedClass,
                   predictedClass2,
                   class1Votes,
                   class2Votes,
                   probOfKnownClass,
                   probability,
                   predictedClass2Prob,
                   numOfWinners,
                   knownClassOneOfTheWinners,
                   breakTie
                   );

  if  (predictedClass == NULL)
  {
    log.Level (-1) << endl << endl 
                   << "Classifier2::ClassifyAImageOneLevel   The trainedModel returned back a NULL ptr for predicted class" << endl
                   << endl;
    predictedClass = unKnownImageClass;
  }


  if  (predictedClass->UnDefined ())
  {
    predictedClass = noiseImageClass;
    example.MLClass (noiseImageClass);
  }
  else
  {
    example.MLClass (predictedClass);
  }

  return  predictedClass;
}  /* ClassifyAImageOneLevel */




MLClassPtr  Classifier2::ClassifyAImageOneLevel (FeatureVector&  example)
{
  double  probability;
  bool    knownClassOneOfTheWinners;
  kkint32 numOfWinners;
  double  breakTie;

  return  ClassifyAImageOneLevel (example, 
                                  probability, 
                                  numOfWinners,
                                  knownClassOneOfTheWinners,
                                  breakTie
                                );
}  /* ClassifyAImageOneLevel */




MLClassPtr  Classifier2::ClassifyAImageOneLevel (FeatureVector&  example,
                                                 kkint32&        numOfWinners,
                                                 bool&           knownClassOneOfTheWinners
                                                )
{
  double   probability;
  double   breakTie;

  return  ClassifyAImageOneLevel (example, 
                                  probability, 
                                  numOfWinners, 
                                  knownClassOneOfTheWinners,
                                  breakTie
                                );
}  /* ClassifyAImageOneLevel */







void  Classifier2::ClassifyAImage (FeatureVector&  example,
                                   MLClassPtr&     predClass1,
                                   MLClassPtr&     predClass2,
                                   kkint32&        predClass1Votes,
                                   kkint32&        predClass2Votes,
                                   double&         knownClassProb,
                                   double&         predClass1Prob,
                                   double&         predClass2Prob,
                                   kkint32&        numOfWinners,
                                   double&         breakTie
                                  )
{
  bool   knownClassOneOfTheWiners = false;

  predClass1     = NULL;
  predClass2     = NULL;
  knownClassProb = -1.0f;
  predClass1Prob = -1.0f;
  predClass2Prob = -1.0f;

  MLClassPtr origClass  = example.MLClass ();


  trainedModel->Predict (&example,
                         origClass,
                         predClass1,
                         predClass2,
                         predClass1Votes,
                         predClass2Votes,
                         knownClassProb,
                         predClass1Prob,
                         predClass2Prob,
                         numOfWinners,
                         knownClassOneOfTheWiners,
                         breakTie
                        );

  if  (!predClass1)
  {
    predClass1 = noiseImageClass;
    example.MLClass (noiseImageClass);
  }
  else
  {
    example.MLClass (predClass1);
  }

  return;
}  /* ClassifyAImage */





MLClassPtr  Classifier2::ClassifyAImage (FeatureVector&  example,
                                         double&         probability,
                                         kkint32&        numOfWinners,
                                         bool&           knownClassOneOfTheWinners,
                                         double&         breakTie
                                        )
{
  MLClassPtr predictedClass;

  probability       = 0.0;

  predictedClass = ClassifyAImageOneLevel (example, 
                                           probability, 
                                           numOfWinners, 
                                           knownClassOneOfTheWinners,
                                           breakTie
                                          );
  return  predictedClass;
}  /* ClassifyAImage */




MLClassPtr  Classifier2::ClassifyAImage (FeatureVector&  example,
                                         kkint32&        numOfWinners,
                                         bool&           knownClassOneOfTheWinners
                                        )
{
  MLClassPtr predictedClass;

  // Lets first Normalize Feature Data.

  predictedClass = ClassifyAImageOneLevel (example, numOfWinners, knownClassOneOfTheWinners);

  return  predictedClass;
} /* ClassifyAImage */




MLClassPtr  Classifier2::ClassifyAImage (FeatureVector&  example)
{
  kkint32 numOfWinners;
  bool  knownClassOneOfTheWinners;
  return  ClassifyAImage (example, numOfWinners, knownClassOneOfTheWinners);
}




vector<KKStr>  Classifier2::SupportVectorNames (MLClassPtr  c1,
                                                MLClassPtr  c2
                                               )
{
  if  (!trainedModelSVMModel)
  {
    vector<KKStr>  results;
    return  results;
  }
  else
  {
    return  trainedModelSVMModel->SupportVectorNames (c1, c2);
  }
}





vector<ProbNamePair>  Classifier2::FindWorstSupportVectors (FeatureVectorPtr  example,
                                                            kkint32           numToFind,
                                                            MLClassPtr        c1,
                                                            MLClassPtr        c2
                                                           )
{
  if  (!trainedModelSVMModel)
  {
    vector<ProbNamePair>  results;
    return  results;
  }

  return  trainedModelSVMModel->FindWorstSupportVectors (example, numToFind, c1, c2);
}

 


vector<ProbNamePair>  Classifier2::FindWorstSupportVectors2 (FeatureVectorPtr  example,
                                                             kkint32           numToFind,
                                                             MLClassPtr        c1,
                                                             MLClassPtr        c2
                                                            )
{
  if  (!trainedModelSVMModel)
  {
    vector<ProbNamePair>  results;
    return  results;
  }

  return  trainedModelSVMModel->FindWorstSupportVectors2 (example, numToFind, c1, c2);
}

 





void  Classifier2::ProbabilitiesByClass (const MLClassList& classes,
                                         FeatureVectorPtr   example,
                                         kkint32*           votes,
                                         double*            probabilities
                                        )
{
  if  (trainedModel)
  {
    trainedModel->ProbabilitiesByClass (example, classes, votes, probabilities);
  }
}  /* ProbabilitiesByClass */





void  Classifier2::RetrieveCrossProbTable (MLClassList&  classes,
                                           double**      crossProbTable  // two dimension matrix that needs to be classes.QueueSize ()  squared.
                                          )
{
  if  (trainedModel)
  {
    trainedModel->RetrieveCrossProbTable (classes, crossProbTable);
  }
}

