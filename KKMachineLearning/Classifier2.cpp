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
#include "ClassProb.h"
#include "MLClass.h"
#include "NormalizationParms.h"
#include "TrainingProcess2.h"
using namespace  KKMLL;



Classifier2::Classifier2 (TrainingProcess2Ptr  _trainer,
                          RunLog&              _log
                         ):

  abort                     (false),
  classifierClassIndex      (),
  classClassifierIndex      (),
  featuresAlreadyNormalized (false),
  mlClasses              (NULL),
  log                       (_log),
  subClassifiers            (NULL),
  configRootName            (),
  trainedModel              (NULL),
  trainedModelOldSVM        (NULL),
  trainedModelSVMModel      (NULL),
  noiseMLClass              (NULL),
  trainingProcess           (_trainer),
  unKnownMLClass            (NULL)
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

  if  (trainingProcess->Config () != NULL)
    configRootName = trainingProcess->Config ()->ConfigRootName ();

  featuresAlreadyNormalized = trainingProcess->FeaturesAlreadyNormalized ();
  mlClasses = new MLClassList (*(_trainer->MLClasses ()));
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
  noiseMLClass   = mlClasses->GetNoiseClass ();
  unKnownMLClass = mlClasses->GetUnKnownClass ();

  if  (trainedModel->ModelType () == Model::mtOldSVM)
  {
    trainedModelOldSVM = dynamic_cast<ModelOldSVMPtr> (trainedModel);
    if  (trainedModelOldSVM)
      trainedModelSVMModel = trainedModelOldSVM->SvmModel ();
  }

  BuildSubClassifierIndex ();
}



Classifier2::~Classifier2 ()
{
  delete mlClasses;       mlClasses      = NULL;
  delete subClassifiers;  subClassifiers = NULL;
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



ClassProbList const *  Classifier2::PriorProbability ()  const
{
  if  (!trainingProcess)
    return NULL;

  return  trainingProcess->PriorProbability ();
}  /* PriorProbability */







MLClassPtr  Classifier2::ClassifyAImageOneLevel (FeatureVector&  example,
                                                 double&         probability,
                                                 kkint32&        numOfWinners, 
                                                 bool&           knownClassOneOfTheWinners,
                                                 double&         breakTie
                                                )

{
  probability       = 0.0;

  double  probOfKnownClass = 0.0f;
  probability = 0.0;

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
    predictedClass = unKnownMLClass;
  }


  if  (subClassifiers)
  {
    Classifier2Ptr  subClassifer = LookUpSubClassifietByClass (predictedClass);
    if  (subClassifer)
  {
      double  subProbability = 0.0;
      kkint32 subNumOfWinners = 0;
      double  subBreakTie = 0.0;
      /**@todo  make sure that the following call does not normalize the featires. */
      MLClassPtr       subPrediction 
        = subClassifer->ClassifyAImageOneLevel (example, subProbability, subNumOfWinners, knownClassOneOfTheWinners, subBreakTie);
      if  (subPrediction)
      {
        probability = probability * subProbability;
        numOfWinners = numOfWinners + subNumOfWinners;
        breakTie += subBreakTie * (1.0 - breakTie);
      }
  }
  }

  if  (predictedClass->UnDefined ())
    predictedClass = noiseMLClass;

  example.MLClass (predictedClass);

  return  predictedClass;
}  /* ClassifyAImageOneLevel */




MLClassPtr  Classifier2::ClassifyAImageOneLevel (FeatureVector&  example)
{
  double  probability = 0.0;
  bool    knownClassOneOfTheWinners = false;
  kkint32 numOfWinners = 0;
  double  breakTie = 0.0;

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
  double   probability = 0.0;
  double   breakTie    = 0.0;

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
    predClass1 = noiseMLClass;


  if  (subClassifiers)
  {
    Classifier2Ptr  subClassifer = LookUpSubClassifietByClass (predClass1);
    if  (subClassifer)
    {
      MLClassPtr       subPredClass1      = NULL;
      MLClassPtr       subPredClass2      = NULL;
      kkint32          subPredClass1Votes = 0;
      kkint32          subPredClass2Votes = 0;
      double           subKnownClassProb  = 0.0;
      double           subPredClass1Prob  = 0.0;
      double           subPredClass2Prob  = 0.0;
      kkint32          subNumOfWinners    = 0;
      double           subBreakTie        = 0.0;

      subClassifer->ClassifyAImage (example, subPredClass1, subPredClass2, 
                                    subPredClass1Votes, subPredClass2Votes, subKnownClassProb,
                                    subPredClass1Prob,  subPredClass2Prob,  subNumOfWinners,
                                    subBreakTie
                                   );
      predClass1 = subPredClass1;
      predClass1Votes += subPredClass1Votes;
      predClass1Prob  *= subPredClass1Prob;
      knownClassProb  *= subKnownClassProb;
      numOfWinners    += subNumOfWinners;
      breakTie        += subBreakTie * (1.0 - breakTie);
    }

    subClassifer = LookUpSubClassifietByClass (predClass2);
    if  (subClassifer)
    {
      MLClassPtr       subPredClass1      = NULL;
      MLClassPtr       subPredClass2      = NULL;
      kkint32          subPredClass1Votes = 0;
      kkint32          subPredClass2Votes = 0;
      double           subKnownClassProb  = 0.0;
      double           subPredClass1Prob  = 0.0;
      double           subPredClass2Prob  = 0.0;
      kkint32          subNumOfWinners    = 0;
      double           subBreakTie        = 0.0;

      subClassifer->ClassifyAImage (example, subPredClass1, subPredClass2, 
                                    subPredClass1Votes, subPredClass2Votes, subKnownClassProb,
                                    subPredClass1Prob,  subPredClass2Prob,  subNumOfWinners,
                                    subBreakTie        
                                   );
      predClass2 = subPredClass1;
      predClass2Votes += subPredClass1Votes;
      predClass2Prob  *= subPredClass1Prob;
    }
  }

  example.MLClass (predClass1);

  return;
}  /* ClassifyAImage */





MLClassPtr  Classifier2::ClassifyAImage (FeatureVector&  example,
                                         double&         probability,
                                         kkint32&        numOfWinners,
                                         bool&           knownClassOneOfTheWinners,
                                         double&         breakTie
                                        )
{
  MLClassPtr       predictedClass = NULL;

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
  MLClassPtr       predictedClass = NULL;

  // Lets first Normalize Feature Data.

  predictedClass = ClassifyAImageOneLevel (example, numOfWinners, knownClassOneOfTheWinners);

  return  predictedClass;
} /* ClassifyAImage */




MLClassPtr  Classifier2::ClassifyAImage (FeatureVector&  example)
{
  kkint32 numOfWinners = 0;
  bool   knownClassOneOfTheWinners = false;
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

 


void  Classifier2::PredictRaw (FeatureVectorPtr  example,
                               MLClassPtr     &  predClass,
                               double&           dist
                              )
{
  trainedModel->PredictRaw (example, predClass, dist);
  if  (subClassifiers)
  {
    Classifier2Ptr  subClassifer = LookUpSubClassifietByClass (predClass);
    if  (subClassifer)
      subClassifer->PredictRaw (example, predClass, dist);
  }
}  /* PredictRaw */




void  Classifier2::ProbabilitiesByClass (const MLClassList& classes,
                                         FeatureVectorPtr   example,
                                         kkint32*           votes,
                                         double*            probabilities
                                        )
{
  ClassProbListPtr  predictions = ProbabilitiesByClass (example);

  kkuint32  numClasses = classes.size ();
  for  (kkuint32 x = 0;  x < numClasses;  ++x)
  {
    votes[x] = 0;
    probabilities[x] = 0.0;

    MLClassPtr      c = classes.IdxToPtr (x);
    ClassProbPtr cp = predictions->LookUp (c);
    if  (cp)
    {
      votes[x] = (kkint32)(0.5f + cp->votes);
      probabilities[x] = cp->probability;
    }
  }
}  /* ProbabilitiesByClass */




MLClassListPtr  Classifier2::PredictionsThatHaveSubClassifier (ClassProbListPtr  predictions)
{
  MLClassListPtr  classes = new MLClassList ();
  ClassProbList::iterator  idx;
  for  (idx = predictions->begin ();  idx != predictions->end ();  ++idx)
  {
    ClassProbPtr  cp = *idx;
    Classifier2Ptr  subClassifier = LookUpSubClassifietByClass (cp->classLabel);
    if  (subClassifier)
      classes->PushOnBack (cp->classLabel);
  }

  return  classes;
}  /* PredictionsThatHaveSubClassifier */






ClassProbListPtr  Classifier2::GetListOfPredictionsForClassifier (Classifier2Ptr    classifier,
                                                                  ClassProbListPtr  predictions
                                                                 )
{
  ClassProbListPtr  subPredictions = new ClassProbList (false);
  ClassifierClassIndexType::iterator  idx;
  idx = classifierClassIndex.find (classifier);
  while  (idx != classifierClassIndex.end ())
  {
    if  (idx->first != classifier)
      break;

    ClassProbPtr cp = predictions->LookUp (idx->second);
    if  (cp)
      subPredictions->PushOnBack (cp);
  }
  return  subPredictions;
}  /* GetListOfPredictionsForClassifier */




/**
 *@param[in]  upperLevelPredictions  Will take ownership and replace with new consolidated results.
 */
ClassProbListPtr  Classifier2::ProcessSubClassifersMethod1 (FeatureVectorPtr  example,
                                                            ClassProbListPtr  upperLevelPredictions
                                                           )
{
  if  (!subClassifiers)
    return upperLevelPredictions;

  ClassProbListPtr results = new ClassProbList ();
 
  Classifier2List::iterator  idx;

  for  (idx = subClassifiers->begin ();  idx != subClassifiers->end ();  ++idx)
  {
    Classifier2Ptr  subClassifier = *idx;
    ClassProbListPtr  subSetPredictions = subClassifier->ProbabilitiesByClass (example);

    ClassProbList::iterator  idx2;
    for  (idx2 = subSetPredictions->begin ();  idx2 != subSetPredictions->end ();  ++idx2)
    {
      ClassProbPtr  cp = *idx2;
      results->MergeIn (cp);
    }

    delete  subSetPredictions;
    subSetPredictions = NULL;
  }

  {
    ClassProbList::iterator  idx2;
    for  (idx2 = upperLevelPredictions->begin ();  idx2 != upperLevelPredictions->end ();  ++idx2)
    {
      ClassProbPtr  oldPrediction = *idx2;
      ClassProbPtr  alreadyInResults = results->LookUp (oldPrediction->classLabel);
      if  (!alreadyInResults)
        results->MergeIn (oldPrediction);
    }
  }

  return results;
}  /* ProcessSubClassifersMethod1 */





/**
 *@param[in]  upperLevelPredictions  Will take ownership and replace with new consolidated results.
 */
ClassProbListPtr  Classifier2::ProcessSubClassifersMethod2 (FeatureVectorPtr   example,
                                                            ClassProbListPtr   upperLevelPredictions
                                                           )
{
  if  (!subClassifiers)
  {
    ClassProbListPtr  results = new ClassProbList (*upperLevelPredictions);
    return results;
  }

  ClassProbListPtr results = new ClassProbList ();

  ClassProbList::const_iterator  idx1;
  for  (idx1 = upperLevelPredictions->begin ();  idx1 != upperLevelPredictions->end ();  ++idx1)
  {
    ClassProbPtr  ulp = *idx1;
    Classifier2Ptr  subClassifier = LookUpSubClassifietByClass (ulp->classLabel);
    if  (subClassifier == NULL)
    {
      results->PushOnBack (new ClassProb (*ulp));
    }
    else
    {
      ClassProbListPtr  subPredictions = subClassifier->ProbabilitiesByClass (example);
      if  (subPredictions == NULL)
      {
        results->PushOnBack (new ClassProb (*ulp));
      }
      else
      {
         ClassProbList::const_iterator  idx2;
         for  (idx2 = subPredictions->begin ();  idx2 != subPredictions->end ();  ++idx2)
         {
           ClassProbPtr  subPred = *idx2;
           double probability = ulp->probability * subPred->probability;
           float  votes = ulp->votes + subPred->votes;
           results->PushOnBack (new ClassProb (subPred->classLabel, probability, votes));
         }
         delete  subPredictions;
         subPredictions = NULL;
      }
    }

  }
  
  results->NormalizeToOne ();
  return results;
}  /* ProcessSubClassifersMethod2 */






ClassProbListPtr  Classifier2::ProbabilitiesByClass (FeatureVectorPtr  example)
{
  if  (!trainedModel)
    return NULL;

  ClassProbListPtr  results = trainedModel->ProbabilitiesByClass (example);
  if  (!results)
    return NULL;

  ClassProbListPtr  expandedResults = ProcessSubClassifersMethod2 (example, results);
  delete  results;
  results = NULL;


  return  expandedResults;
}  /* ProbabilitiesByClass */





void  Classifier2::RetrieveCrossProbTable (MLClassList&  classes,
                                           double**      crossProbTable  // two dimension matrix that needs to be classes.QueueSize ()  squared.
                                          )
{
  if  (trainedModel)
    trainedModel->RetrieveCrossProbTable (classes, crossProbTable);
  }





void  Classifier2::ProbabilitiesByClassDual (FeatureVectorPtr   example,
                                             KKStr&             classifier1Desc,
                                             KKStr&             classifier2Desc,
                                             ClassProbListPtr&  classifier1Results,
                                             ClassProbListPtr&  classifier2Results
                                            )
{
  if  (trainedModel)
    trainedModel->ProbabilitiesByClassDual (example, classifier1Desc, classifier2Desc, classifier1Results, classifier2Results);
}





void  Classifier2::BuildSubClassifierIndex ()
{
  delete  subClassifiers;
  subClassifiers = NULL;
  classClassifierIndex.clear ();
  classifierClassIndex.clear ();

  if  (trainingProcess == NULL)
    return;

  if  (trainingProcess->SubTrainingProcesses () == NULL)
    return;

  if  (trainingProcess->Config () == NULL)
    return;

  subClassifiers = new Classifier2List (true);
  {
    TrainingProcess2ListPtr  subProcessors = trainingProcess->SubTrainingProcesses ();
    TrainingProcess2List::const_iterator  idx;
    for  (idx = subProcessors->begin ();  idx != subProcessors->end ();  ++idx)
    {
      TrainingProcess2Ptr  tp = *idx;
      Classifier2Ptr  subClassifier = new Classifier2 (tp, log);
      subClassifiers->PushOnBack (subClassifier);
    }
  }

  {
    TrainingConfiguration2Const*  config = trainingProcess->Config ();
    
    const TrainingClassList&  trainClasses = config->TrainingClasses ();
    TrainingClassList::const_iterator  idx;
    for  (idx = trainClasses.begin ();  idx != trainClasses.end ();  ++idx)
    {
      TrainingClassPtr  tcp = *idx;
      if  (tcp->SubClassifier () != NULL)
      {
        Classifier2Ptr  subClassifier = subClassifiers->LookUpByName (tcp->SubClassifier ()->ConfigRootName ());
        if  (subClassifier)
        {
          ClassClassifierIndexType::const_iterator  idx;
          idx = classClassifierIndex.find (tcp->MLClass ());
          if  (idx == classClassifierIndex.end ())
            classClassifierIndex.insert (ClassClassifierPair (tcp->MLClass (), subClassifier));
          classifierClassIndex.insert (ClassifierClassPair (subClassifier, tcp->MLClass ()));
        }
      }
    }
  }
}  /* BuildSubClassifierIndex */




Classifier2Ptr  Classifier2::LookUpSubClassifietByClass (MLClassPtr       c)
{
  ClassClassifierIndexType::const_iterator  idx;
  idx = classClassifierIndex.find (c);
  if  (idx == classClassifierIndex.end ())
    return NULL;
  else
    return idx->second;
}  /* LookUpSubClassifietByClass */






Classifier2List::Classifier2List (bool _owner):
    KKQueue<Classifier2> (_owner)
{
}



Classifier2List::~Classifier2List ()
{
}



Classifier2Ptr  Classifier2List::LookUpByName (const KKStr&  rootName)  const
{
  Classifier2List::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  ++idx)
  {
    Classifier2Ptr  c = *idx;
    if  (c->ConfigRootName ().EqualIgnoreCase (rootName))
      return c;
  }
  return NULL;
}  /* LookUpByName */


