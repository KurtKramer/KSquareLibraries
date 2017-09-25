#include "FirstIncludes.h"

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <sstream>
#include <iomanip>
#include <set>
#include <vector>
#include "MemoryDebug.h"
using namespace std;


#include "GlobalGoalKeeper.h"
#include "KKBaseTypes.h"
#include "KKException.h"
#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace  KKB;


#include "ModelDual.h"
#include "Classifier2.h"
#include "ClassProb.h"
#include "FeatureNumList.h"
#include "FeatureVector.h"
#include "ModelParamDual.h"
#include "TrainingConfiguration2.h"
#include "TrainingProcess2.h"
using namespace  KKMLL;




ModelDual::ModelDual ():
  Model (),
  param         (NULL),
  config1       (NULL),
  config2       (NULL),
  trainer1      (NULL),
  trainer2      (NULL),
  classifier1   (NULL),
  classifier2   (NULL)
{
}




ModelDual::ModelDual (FactoryFVProducerPtr  _factoryFVProducer):
  Model (_factoryFVProducer),
  param         (NULL),
  config1       (NULL),
  config2       (NULL),
  trainer1      (NULL),
  trainer2      (NULL),
  classifier1   (NULL),
  classifier2   (NULL)
{
}


ModelDual::ModelDual (const KKStr&           _name,
                      const ModelParamDual&  _param,         // Create new model from
                      FactoryFVProducerPtr   _factoryFVProducer
                     ):
  Model (_name, _param, _factoryFVProducer),
  param         (NULL),
  config1       (NULL),
  config2       (NULL),
  trainer1      (NULL),
  trainer2      (NULL),
  classifier1   (NULL),
  classifier2   (NULL)
{
  param = dynamic_cast<ModelParamDualPtr> (Model::param);
}




ModelDual::ModelDual (const ModelDual&   _model):
  Model  (_model),
  param         (NULL),
  config1       (NULL),
  config2       (NULL),
  trainer1      (NULL),
  trainer2      (NULL),
  classifier1   (NULL),
  classifier2   (NULL)
{
  param = dynamic_cast<ModelParamDualPtr> (Model::param);
}





/**
 * @brief Frees any memory allocated by, and owned by the ModelDual
 */
ModelDual::~ModelDual ()
{
  // The base class owns param,  so we do not delete it.
  DeleteExistingClassifiers ();
  delete  config1;      config1     = NULL;
  delete  config2;      config2     = NULL;
  delete  trainer1;     trainer1    = NULL;
  delete  trainer2;     trainer2    = NULL;
  delete  classifier1;  classifier1 = NULL;
  delete  classifier2;  classifier2 = NULL;

}


kkMemSize ModelDual::MemoryConsumedEstimated ()  const
{
  kkMemSize memoryConsumedEstimated = Model::MemoryConsumedEstimated ();
  if  (trainer1)    memoryConsumedEstimated += trainer1->MemoryConsumedEstimated ();
  if  (trainer2)    memoryConsumedEstimated += trainer2->MemoryConsumedEstimated ();
  if  (classifier1) memoryConsumedEstimated += classifier1->MemoryConsumedEstimated ();
  if  (classifier2) memoryConsumedEstimated += classifier2->MemoryConsumedEstimated ();
  return  memoryConsumedEstimated;
}




KKStr   ModelDual::Description ()  const
{
  KKStr  result = "Dual(" + Name () + ")";
  if  (trainer1)
    result << " " << trainer1->ModelTypeStr () + "(" + osGetRootName (trainer1->ConfigFileName ()) + ")";
  else if  (config1)
    result << " " << config1->ModelTypeStr  () + "(" + osGetRootName (config1->FileName ()) + ")";

  if  (trainer2)
    result << " " << trainer2->ModelTypeStr () + "(" + osGetRootName (trainer2->ConfigFileName ()) + ")";
  else if  (config1)
    result << " " << config2->ModelTypeStr  () + "(" + osGetRootName (config2->FileName ()) + ")";

  return  result;
}



ModelDualPtr  ModelDual::Duplicate ()  const
{
  return new ModelDual (*this);
}



ModelParamDualPtr   ModelDual::Param ()
{
  return param;
}


void  ModelDual::DeleteExistingClassifiers ()
{
  delete  config1;      config1     = NULL;
  delete  config2;      config2     = NULL;
  delete  trainer1;     trainer1    = NULL;
  delete  trainer2;     trainer2    = NULL;
  delete  classifier1;  classifier1 = NULL;
  delete  classifier2;  classifier2 = NULL;
}  /* DeleteExistingClassifiers */




void  ModelDual::TrainModel (FeatureVectorListPtr  _trainExamples,
                             bool                  _alreadyNormalized,
                             bool                  _takeOwnership,  /**< Model will take ownership of these examples */
                             VolConstBool&         _cancelFlag,
                             RunLog&               _log
                            )
{
  _log.Level (10) << "ModelDual::TrainModel  Model[" << param->FileName () << "]  _trainExamples->size()[" << _trainExamples->size() << "]" << endl;

  if  (param == NULL)
  {
    validModel = false;
    KKStr  errMsg = "ModelDual::TrainModel   (param == NULL)";
    _log.Level (-1) << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  DeleteExistingClassifiers ();

  try 
  {
    Model::TrainModel (_trainExamples, _alreadyNormalized, _takeOwnership, _cancelFlag, _log);
  }
  catch (const KKException&  e)
  {
    validModel = false;
    KKStr  errMsg = "ModelDual::TrainModel  ***ERROR*** Exception occurred calling 'Model::TrainModel'.";
    _log.Level (-1) << endl << errMsg << endl << e.ToString () << endl << endl;
    throw  KKException (errMsg, e);
  }
  catch (const exception& e2)
  {
    validModel = false;
    KKStr errMsg = "ModelDual::TrainModel  ***ERROR*** Exception occurred calling 'Model::TrainModel'.";
    _log.Level (-1) << endl << errMsg << endl << e2.what () << endl << endl;
    throw KKException (errMsg, e2);
  }
  catch (...)
  {
    validModel = false;
    KKStr errMsg = "ModelDual::TrainModel  ***ERROR*** Exception occurred calling 'Model::TrainModel'.";
    _log.Level (-1) << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }
    
  // 'Model::TrainModel'  Will have performed any BitReduction and Feature Encoding 
  // that needed to be done.  
  // Also the data structures 'classes', 'encoder', and 'fileDesc' will have been built.
  // 'classes' will already be sorted in name order.
  // The Prediction variables 'probabilities', 'votes', and 'crossClassProbTable' will
  // have been built.

  {
    // Make sure that the configuration files actually exist.
    if  (!TrainingConfiguration2::ConfigFileExists (param->ConfigFileName1 ()))
    {
      validModel = false;
      KKStr errMsg = "ModelDual::TrainModel  ***ERROR***  Could not find Configuration[" + param->ConfigFileName1 () + "].";
      _log.Level (-1) << endl << errMsg << endl << endl;
      throw KKException (errMsg);
    }

    if  (!TrainingConfiguration2::ConfigFileExists (param->ConfigFileName2 ()))
    {
      validModel = false;
      KKStr errMsg = "ModelDual::TrainModel  ***ERROR***  Could not find Configuration[" + param->ConfigFileName2 () + "].";
      _log.Level (-1) << endl << errMsg << endl << endl;
      throw KKException (errMsg);
    }
  }

  config1 = factoryFVProducer->ManufacturTrainingConfiguration ();
  config1->Load (param->ConfigFileName1 (),
                 false,  // false = Do NOT validate directories.
                 _log
                );
  if  (!config1->FormatGood ())
  {
    validModel = false;
    KKStr errMsg = "ModelDual::TrainModel  ***ERROR***  Configuration[" + param->ConfigFileName1 () + "] is not valid.";
    _log.Level (-1) << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  config2 = factoryFVProducer->ManufacturTrainingConfiguration ();
  config2->Load (param->ConfigFileName2 (), 
                 false,  // false = Do NOT validate directories.
                 _log
                );
  if  (!config2->FormatGood ())
  {
    validModel = false;
    KKStr errMsg = "ModelDual::TrainModel  ***ERROR***  Configuration[" + param->ConfigFileName2 () + "] is not valid.";
    _log.Level (-1) << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  KKStr  statusMsg;

  TrainingTimeStart ();
  trainer1 = TrainingProcess2::CreateTrainingProcessFromTrainingExamples 
                          (config1, 
                           trainExamples, 
                           false,         /**< false = DON'T take ownership of 'trainExamples'. */
                           true,          /**< true = Features are already normalized.  */
                           _cancelFlag,
                           _log
                          );
  TrainingTimeEnd ();

  if  ((!trainer1)  ||  trainer1->Abort ())
  {
    validModel = false;
    KKStr errMsg = "ModelDual::TrainModel  ***ERROR***  Error building TrainingProcess for [" + param->ConfigFileName1 () + "].";
    _log.Level (-1) << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  TrainingTimeStart ();
  trainer2 = TrainingProcess2::CreateTrainingProcessFromTrainingExamples 
                          (config2, 
                           trainExamples, 
                           false,         /**< false = DON'T take ownership of 'trainExamples'. */
                           true,          /**< true = Features are already normalized.  */
                           _cancelFlag,
                           _log
                          );
  TrainingTimeEnd ();

  if  ((!trainer2)  ||  trainer2->Abort ())
  {
    validModel = false;
    KKStr errMsg = "ModelDual::TrainModel  ***ERROR***  Error building TrainingProcess for [" + param->ConfigFileName1 () + "].";
    _log.Level (-1) << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  classifier1 = new Classifier2 (trainer1, _log);
  classifier2 = new Classifier2 (trainer2, _log);
}  /* TrainModel */



MLClassPtr  ModelDual::ReconcilePredictions (MLClassPtr  pred1, 
                                             MLClassPtr  pred2,
                                             RunLog&     log
                                            )
{
  if  (pred1 == pred2)
    return pred1;

  else if  (param->FullHierarchyMustMatch ())
  {
    if  (param->OtherClass ())
      return param->OtherClass ();
    else
      return MLClass::CreateNewMLClass ("NoAgreement");
  }

  if  (!pred1)
  {
    if  (!pred2)
    {
      log.Level (-1) << "ModelDual::ReconcilePredictions ***WARNING***  Classifier[" << param->ConfigFileName1 () << "] nor Classifier[" << param->ConfigFileName2 () << "]  return predictions." << endl;
      return  NULL;
    }
    else
    {
      log.Level (-1) << "ModelDual::ReconcilePredictions  ***WARNING***  Classifier[" << param->ConfigFileName1 () << "] did not return a prediction." << endl;
      return pred2;
    }
  }
  else
  {
    if  (!pred2)
    {
      log.Level (-1) << "ModelDual::ReconcilePredictions  ***WARNING***  Classifier[" << param->ConfigFileName2 () << "] did not return a prediction." << endl;
      return  pred1;
    }
  }

  // We need to find the common part of the predictions.
  KKStr  name1 = pred1->Name ();
  KKStr  name2 = pred2->Name ();
  kkint32 maxLen = Min (name1.Len (), name2.Len ());
  KKStr  commonPart (maxLen + 1);

  while  ((!name1.Empty ())  &&  (!name2.Empty ()))
  {
    KKStr  name1Token = name1.ExtractToken2 ("_");
    KKStr  name2Token = name2.ExtractToken2 ("_");
    if  (name1Token == name2Token)
    {
      if  (!commonPart.Empty ())
        commonPart.Append ('_');
      commonPart += name1Token;
    }
    else
    {
      break;
    }
  }

  if  (commonPart.Len () < 1)
  {
    if  (param->OtherClass ())
      return param->OtherClass ();
    else
      return MLClass::CreateNewMLClass ("NoAgreement");
  }
  else
  {
    return  MLClass::CreateNewMLClass (commonPart);
  }
}  /* ReconcilePredictions */




void  ModelDual::ReconcileProbAndVotes (Classifier2Ptr    classifier,
                                        MLClassPtr        predClass,
                                        FeatureVectorPtr  encodedExample,
                                        double&           predClassProb,
                                        kkint32&            predClassVotes
                                       )
{
  const KKStr&  name = predClass->Name ();

  predClassProb = 0.0;
  predClassVotes = 0;

  ClassProbListPtr  predictions = classifier->ProbabilitiesByClass (encodedExample);
  ClassProbList::iterator  idx;
  for  (idx = predictions->begin ();  idx != predictions->end ();  ++idx)
  {
    ClassProbPtr p = *idx;
    if  (p->classLabel->Name ().StartsWith (name))
    {
      predClassProb  += p->probability;
      predClassVotes += (kkint32)(p->votes);
    }
  }

  delete  predictions;
  predictions =  NULL;

  return;
}  /* ReconcileProbAndVotes */





MLClassPtr  ModelDual::Predict (FeatureVectorPtr  example,
                                RunLog&           log
                               )
{
  if  ((!classifier1)  ||  (!classifier2))
  {
    log.Level (-1) << endl << endl << "ModelDual::Predict   ***ERROR***      Both Classifiers are not defined." << endl << endl;
    return NULL;
  }

  bool  newExampleCreated = false;

  MLClassPtr      pred1 = NULL;
  MLClassPtr      pred2 = NULL;

  /**@todo   ModelDual::Predict    Make sure that the call to the first classifier does not modify the encodedExample. */
  FeatureVectorPtr  encodedExample = PrepExampleForPrediction (example, newExampleCreated);
  pred1 = classifier1->ClassifyAExample (*encodedExample);
  pred2 = classifier2->ClassifyAExample (*encodedExample);

  if  (newExampleCreated)
  {
    delete  encodedExample;
    encodedExample = NULL;
  }

  return  ReconcilePredictions (pred1, pred2, log);
}  /* Predict */




void  ModelDual::Predict (FeatureVectorPtr  example,
                          MLClassPtr        knownClass,
                          MLClassPtr&       predClass1,
                          MLClassPtr&       predClass2,
                          kkint32&          predClass1Votes,
                          kkint32&          predClass2Votes,
                          double&           probOfKnownClass,
                          double&           predClass1Prob,
                          double&           predClass2Prob,
                          kkint32&          numOfWinners,
                          bool&             knownClassOneOfTheWinners,
                          double&           breakTie,
                          RunLog&           log
                         )
{
  if  ((!classifier1)  ||  (!classifier2))
  {
    predClass1 = predClass2 = NULL;
    log.Level (-1) << endl << endl << "ModelDual::Predict   ***ERROR***      Both Classifiers are not defined." << endl << endl;
    return;
  }

  MLClassPtr       predClass1C1       = NULL;
  MLClassPtr       predClass2C1       = NULL;
  kkint32          predClass1VotesC1  = 0;
  kkint32          predClass2VotesC1  = 0;
  double           probOfKnownClassC1 = 0.0;
  double           predClass1ProbC1   = 0.0;
  double           predClass2ProbC1   = 0.0;
  kkint32          numOfWinnersC1     = 0;
  bool             knownClassOneOfTheWinnersC1 = false;
  double           breakTieC1         = 0.0;

  MLClassPtr       predClass1C2       = NULL;
  MLClassPtr       predClass2C2       = NULL;
  kkint32          predClass1VotesC2  = 0;
  kkint32          predClass2VotesC2  = 0;
  double           probOfKnownClassC2 = 0.0;
  double           predClass1ProbC2   = 0.0;
  double           predClass2ProbC2   = 0.0;
  kkint32          numOfWinnersC2     = 0;
  bool             knownClassOneOfTheWinnersC2 = false;
  double           breakTieC2         = 0.0;

  bool  newExampleCreated = false;
  FeatureVectorPtr  encodedExample = PrepExampleForPrediction (example, newExampleCreated);

  /**@todo   Make sure that 'classifier1->ClassifyAExample' does not modify the feature data. */

  classifier1->ClassifyAExample (*encodedExample, 
                               predClass1C1,       predClass2C1, 
                               predClass1VotesC1,  predClass2VotesC1,
                               probOfKnownClassC1,
                               predClass1ProbC1,   predClass2ProbC1,
                               numOfWinnersC1,
                               breakTieC1
                              );

  classifier2->ClassifyAExample (*encodedExample, 
                               predClass1C2,       predClass2C2, 
                               predClass1VotesC2,  predClass2VotesC2,
                               probOfKnownClassC2,
                               predClass1ProbC2,   predClass2ProbC2,
                               numOfWinnersC2,
                               breakTieC2
                              );

  predClass1 = ReconcilePredictions (predClass1C1, predClass1C2, log);
  predClass2 = ReconcilePredictions (predClass2C1, predClass2C2, log);

  if  (predClass1C1 != predClass1C2)
  {
    ReconcileProbAndVotes (classifier1,
                           predClass1,
                           encodedExample, 
                           predClass1ProbC1,
                           predClass1VotesC1
                          );

    ReconcileProbAndVotes (classifier2,
                           predClass1,
                           encodedExample, 
                           predClass1ProbC2,
                           predClass1VotesC2
                          );
  }


  if  (predClass2C1 != predClass2C2)
  {
    ReconcileProbAndVotes (classifier1,
                           predClass2,
                           encodedExample, 
                           predClass2ProbC1,
                           predClass2VotesC1
                          );

    ReconcileProbAndVotes (classifier2,
                           predClass2,
                           encodedExample, 
                           predClass2ProbC2,
                           predClass2VotesC2
                          );
  }

  predClass1Prob  = (predClass1ProbC1 + predClass1ProbC2) / 2.0;
  predClass2Prob  = (predClass2ProbC1 + predClass2ProbC2) / 2.0;
  predClass1Votes   = (kkint32)((0.5f + predClass1VotesC1 + predClass1VotesC2) / 2.0f);
  predClass2Votes   = (kkint32)((0.5f + predClass2VotesC1 + predClass2VotesC2) / 2.0f);
  

  probOfKnownClass  = (probOfKnownClassC1 + probOfKnownClassC2) / 2.0;
  breakTie          = (breakTieC1         + breakTieC2)         / 2.0;
  numOfWinners      = (numOfWinnersC1     + numOfWinnersC2)     / 2;
  knownClassOneOfTheWinners = (knownClass == predClass1C1)  ||  (knownClass == predClass1C2);

  if  (newExampleCreated)
  {
    delete  encodedExample;
    encodedExample = NULL;
  }

  return;
}  /* Predict */





ClassProbListPtr  ModelDual::ProbabilitiesByClass (FeatureVectorPtr  example,
                                                   RunLog&           log
                                                  )
{
  if  ((!classifier1)  ||  (!classifier2))
  {
    log.Level (-1) << endl << endl << "ModelDual::ProbabilitiesByClass   ***ERROR***      Both Classifiers are not defined." << endl << endl;
    return NULL;
  }

  ClassProbListPtr  predictions1 = classifier1->ProbabilitiesByClass (example);
  ClassProbListPtr  predictions2 = classifier1->ProbabilitiesByClass (example);

  if  (predictions1 == NULL)
    return predictions2;

  else if  (predictions2 == NULL)
    return predictions1;

  ClassProbList::iterator  idx;
  for  (idx = predictions2->begin ();  idx != predictions2->end ();  ++idx)
  {
    ClassProbPtr  cp = *idx;
    predictions1->MergeIn (cp);
  }
  delete  predictions2;
  predictions2 = NULL;

  predictions1->NormalizeToOne ();

  return  predictions1;
}  /* ProbabilitiesByClass */





void  ModelDual::ProbabilitiesByClassDual (FeatureVectorPtr   example,
                                           KKStr&             classifier1Desc,
                                           KKStr&             classifier2Desc,
                                           ClassProbListPtr&  classifier1Results,
                                           ClassProbListPtr&  classifier2Results,
                                           RunLog&            log
                                          )
{
  delete  classifier1Results;  classifier1Results = NULL;
  delete  classifier2Results;  classifier2Results = NULL;

  if  ((!classifier1)  ||  (!classifier2))
  {
    log.Level (-1) << endl << endl << "ModelDual::ProbabilitiesByClassDual   ***ERROR***      Both Classifiers are not defined." << endl << endl;
    return;
  }

  classifier1Desc = trainer1->ModelDescription ();
  classifier2Desc = trainer2->ModelDescription ();

  {
    bool  newExampleCreated = false;
    FeatureVectorPtr  encodedExample = PrepExampleForPrediction (example, newExampleCreated);
    classifier1Results = classifier1->ProbabilitiesByClass (encodedExample);
    if  (newExampleCreated)
    {
      delete encodedExample;
      encodedExample = NULL;
    }
  }

  {
    bool  newExampleCreated = false;
    FeatureVectorPtr  encodedExample = PrepExampleForPrediction (example, newExampleCreated);
    classifier2Results = classifier2->ProbabilitiesByClass (encodedExample);
    if  (newExampleCreated)
    {
      delete encodedExample;
      encodedExample = NULL;
    }
  }

  return;
}  /* ProbabilitiesByClass */




void  ModelDual::ProbabilitiesByClass (FeatureVectorPtr    example,
                                       const MLClassList&  _mlClasses,
                                       kkint32*            _votes,
                                       double*             _probabilities,
                                       RunLog&             _log
                                      )
{
  kkint32  numClasses = _mlClasses.QueueSize ();
  for  (kkint32 idx = 0;  idx < numClasses;  idx++)
  {
    _probabilities[idx] = 0.0;
    _votes        [idx] = 0;
  }

  if  ((!classifier1)  ||  (!classifier2))
  {
    _log.Level (-1) << endl << "ModelDual::ProbabilitiesByClass   ***ERROR***      Both Classifiers are not defined." << endl << endl;
    return;
  }

  ClassProbListPtr  predictions1 = classifier1->ProbabilitiesByClass (example);
  ClassProbListPtr  predictions2 = classifier2->ProbabilitiesByClass (example);

  if  (predictions1 == NULL)
  {
    predictions1 = predictions2;
    predictions2 = NULL;
  }

  else if  (predictions2 == NULL)
  {
    // 'predictions1' already has the total of both.
  }

  else
  {
    predictions1->AddIn (predictions2);
    delete  predictions2;
    predictions2 = NULL;
  }

  kkuint32 x = 0;
  predictions1->NormalizeToOne ();
  for  (auto c: _mlClasses)
  {
    auto  cp = predictions1->LookUp (c);
    if  (cp)
    {
      _votes        [x] = (kkint32)(0.5f + cp->votes);
      _probabilities[x] = cp->probability;
    }
    ++x;
  }

  delete  predictions1;
  predictions1 = NULL;

  return;
}  /* ProbabilitiesByClass */





void   ModelDual::ProbabilitiesByClass (FeatureVectorPtr    _example,
                                        const MLClassList&  _mlClasses,
                                        double*             _probabilities,
                                        RunLog&             _log
                                       )
{
  kkint32  numClasses = _mlClasses.QueueSize ();
  for  (kkint32 idx = 0;  idx < numClasses;  idx++)
  {
    _probabilities[idx] = 0.0;
  }

  if  ((!classifier1)  ||  (!classifier2))
  {
    _log.Level (-1) << endl << "ModelDual::ProbabilitiesByClass   ***ERROR***      Both Classifiers are not defined." << endl << endl;
    return;
  }

  ClassProbListPtr  predictions1 = classifier1->ProbabilitiesByClass (_example);
  ClassProbListPtr  predictions2 = classifier2->ProbabilitiesByClass (_example);

  if  (predictions1 == NULL)
  {
    predictions1 = predictions2;
    predictions2 = NULL;
  }

  else if  (predictions2 == NULL)
  {
    // 'predictions1' already has the total of both.
  }

  else
  {
    predictions1->AddIn (predictions2);
    delete  predictions2;
    predictions2 = NULL;
  }

  kkuint32 x = 0;
  predictions1->NormalizeToOne ();
  for  (auto c: _mlClasses)
  {
    auto  cp = predictions1->LookUp (c);
    if  (cp)
      _probabilities[x] = cp->probability;
    ++x;
  }

  delete  predictions1;
  predictions1 = NULL;

  return;
}  /* ProbabilitiesByClass */
  



void  ModelDual::RetrieveCrossProbTable (MLClassList&  _classes,
                                         double**      _crossProbTable,  /**< two dimension matrix that needs to be classes.QueueSize ()  squared. */
                                         RunLog&       _log
                                        )
{
  if  ((!classifier1)  ||  (!classifier2))
  {
    _log.Level (-1) << endl << "ModelDual::RetrieveCrossProbTable   ***ERROR***      Both Classifiers are not defined." << endl << endl;
    return;
  }

  kkint32 x = 0, y = 0;

  kkint32 numClasses = _classes.QueueSize ();

  double**  crossProbTableC1 = new double*[numClasses];
  double**  crossProbTableC2 = new double*[numClasses];

  for  (x = 0;  x < numClasses;  ++x)
  {
    crossProbTableC1[x] = new double[numClasses];
    crossProbTableC2[x] = new double[numClasses];
  }

  classifier1->RetrieveCrossProbTable (_classes, crossProbTableC1);
  classifier2->RetrieveCrossProbTable (_classes, crossProbTableC2);

  for  (x = 0;  x < numClasses;  ++x)
  {
    for  (y = 0;  y < numClasses;  ++y)
      _crossProbTable[x][y] = (crossProbTableC1[x][y] + crossProbTableC2[x][y]) / 2.0;
  }

  for  (x = 0;  x < numClasses;  ++x)
  {
    delete  crossProbTableC1[x];  crossProbTableC1[x] = NULL;
    delete  crossProbTableC2[x];  crossProbTableC2[x] = NULL;
  }

  delete[]  crossProbTableC1;  crossProbTableC1 = NULL;
  delete[]  crossProbTableC2;  crossProbTableC2 = NULL;

}  /* RetrieveCrossProbTable */




kkint32 ModelDual::NumOfSupportVectors ()  const
{
  float  totalSVs = 0.0;
  kkuint32 count = 0;

  if  (trainer1)
  {
    totalSVs += (float)(trainer1->NumOfSupportVectors ());
    ++count;
  }

  if  (trainer2)
  {
    totalSVs += (float)(trainer2->NumOfSupportVectors ());
    ++count;
  }

  if  (count > 0)
    return  (kkint32)(0.5f + totalSVs / (float)count);
  else
    return 0;

}  /* NumOfSupportVectors */





void  ModelDual::WriteXML (const KKStr&  varName,
                           ostream&      o
                          )  const
{
  XmlTag  startTag ("ModelDual",  XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);
  o << endl;

  WriteModelXMLFields (o);  // Write the base class data fields 1st.

  config1->WriteXML ("config1", o);
  config2->WriteXML ("config2", o);

  trainer1->WriteXML ("trainer1", o);
  trainer2->WriteXML ("trainer2", o);

  XmlTag  endTag ("ModelDual", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}  /* WriteXML */





void  ModelDual::ReadXML (XmlStream&      s,
                          XmlTagConstPtr  tag,
                          VolConstBool&   cancelFlag,
                          RunLog&         log
                         )
{
  delete  config1;  config1 = NULL;
  delete  config2;  config2 = NULL;

  KKStr  config1Name = "";
  KKStr  config2Name = "";

  XmlTokenPtr  t = s.GetNextToken (cancelFlag, log);
  while  (t  &&  (!cancelFlag))
  {
    t = ReadXMLModelToken (t, log);
    bool tokenFound = true;
    if  ((t != NULL)  &&  (t->TokenType () == XmlToken::TokenTypes::tokElement))
    {
      XmlElementPtr e = dynamic_cast<XmlElementPtr> (t);
      if  (!e)
      {
        tokenFound = false;
      }
      else
      {
        const KKStr&  varName     = e->VarName();
        const KKStr&  sectionName = e->SectionName ();

        if  ((varName.EqualIgnoreCase ("config1"))  &&  (typeid (*e) == typeid (XmlElementTrainingConfiguration2)))
        {
          delete config1;
          config1 = dynamic_cast<XmlElementTrainingConfiguration2Ptr>(e)->TakeOwnership ();
          config1Name = config1->ConfigRootName ();

          /**@todo   I need the Config1 configuration read from the XML file to be correct;  but to deal with this problem for now; will reload the original. */
          delete  config1;
          config1 = new TrainingConfiguration2 ();
          config1 ->Load (config1Name, false, log);
        }

        else if  ((varName.EqualIgnoreCase ("config2"))  &&  (typeid (*e) == typeid (XmlElementTrainingConfiguration2)))
        {
          delete config2;
          config2 = dynamic_cast<XmlElementTrainingConfiguration2Ptr>(e)->TakeOwnership ();
          config2Name = config1->ConfigRootName ();
          /**@todo   I need the Config1 configuration read from the XML file to be correct;  but to deal with this problem for now; will reload the original. */
          delete  config2;
          config2 = new TrainingConfiguration2 ();
          config2 ->Load (config2Name, false, log);
        }
      
        else if  ((varName.EqualIgnoreCase ("trainer1"))  &&  (typeid (*e) == typeid (XmlElementTrainingProcess2)))
        {
          delete trainer1;
          trainer1 = dynamic_cast<XmlElementTrainingProcess2Ptr>(e)->TakeOwnership ();
        }
      
        else if  ((varName.EqualIgnoreCase ("trainer2"))  &&  (typeid (*e) == typeid (XmlElementTrainingProcess2)))
        {
          delete trainer2;
          trainer2 = dynamic_cast<XmlElementTrainingProcess2Ptr>(e)->TakeOwnership ();
        }
        else
        {
          tokenFound = false;
        }
      }
    }
    if  (!tokenFound)
    {
      KKStr  errMsg (128);
      errMsg << "ModelDual::ReadXML   ***ERROR***   Unexpected Element: Section: " << t->SectionName () << " VarName:" << t->VarName ();
      log.Level (-1) << endl << errMsg << endl << endl;
      AddErrorMsg (errMsg, 0);
    }

    delete  t;
    t = s.GetNextToken (cancelFlag, log);
  }

  delete  t;
  t = NULL;

  if  (cancelFlag)
    validModel = false;

  if  (validModel)
  {
    if  (!trainer1)
    {
      validModel = false;
      KKStr  errMsg = "ModelDual::ReadXML   ***ERROR***   trainer1 not loaded!!!";
      log.Level (-1) << endl << errMsg << endl << endl;
      AddErrorMsg (errMsg, 0);
    }

    if  (!trainer2)
    {
      validModel = false;
      KKStr  errMsg = "ModelDual::ReadXML   ***ERROR***   trainer2 not loaded!!!";
      log.Level (-1) << endl << errMsg << endl << endl;
      AddErrorMsg (errMsg, 0);
    }

    if  (validModel)
    {
      delete  classifier1;   classifier1 = new Classifier2 (trainer1, log);
      delete  classifier2;   classifier2 = new Classifier2 (trainer2, log);
    }
  }

  if  (!param)
    param = dynamic_cast<ModelParamDualPtr> (Model::param);

  if  (Model::param == NULL)
  {
    KKStr errMsg (128);
    errMsg << "ModelDual::ReadXML  ***ERROR***  Base class 'Model' does not have 'param' defined.";
    AddErrorMsg (errMsg, 0);
    log.Level (-1) << endl << errMsg << endl << endl;
  }

  else if  (typeid (*Model::param) != typeid(ModelParamDual))
  {
    KKStr errMsg (128);
    errMsg << "ModelDual::ReadXML  ***ERROR***  Base class 'Model' param parameter is of the wrong type;  found: " << Model::param->ModelParamTypeStr ();
    AddErrorMsg (errMsg, 0);
    log.Level (-1) << endl << errMsg << endl << endl;
  }

  else
  {
    param = dynamic_cast<ModelParamDualPtr> (Model::param);
  }

  ReadXMLModelPost (log);
}  /* ReadXML */



XmlFactoryMacro(ModelDual)
