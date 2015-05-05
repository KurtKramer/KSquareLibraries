#include  "FirstIncludes.h"
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


#include "ModelUsfCasCor.h"
#include "ClassProb.h"
#include "FeatureNumList.h"
#include "FeatureVector.h"
#include "MLClass.h"
using namespace  KKMLL;



ModelUsfCasCor::ModelUsfCasCor ():
  Model (),
  param               (NULL),
  usfCasCorClassifier (NULL)
{
}



ModelUsfCasCor::ModelUsfCasCor (FileDescPtr  _fileDesc):
  Model (_fileDesc),
  param               (NULL),
  usfCasCorClassifier (NULL)
{
}


ModelUsfCasCor::ModelUsfCasCor (const KKStr&               _name,
                                const ModelParamUsfCasCor& _param,         // Create new model from
                                FileDescPtr                _fileDesc
                               ):
  Model (_name, _param, _fileDesc),
  param               (NULL),
  usfCasCorClassifier (NULL)
{
  param = dynamic_cast<ModelParamUsfCasCorPtr> (Model::param);
}




ModelUsfCasCor::ModelUsfCasCor (const ModelUsfCasCor&   _model):
  Model (_model),
  param               (NULL),
  usfCasCorClassifier (NULL)
{
  param = dynamic_cast<ModelParamUsfCasCorPtr> (Model::param);
  if  (_model.usfCasCorClassifier)
  {
    usfCasCorClassifier = new UsfCasCor (*_model.usfCasCorClassifier);
  }
}


/**
 *@brief Frees any memory allocated by, and owned by the ModelUsfCasCor
 */
ModelUsfCasCor::~ModelUsfCasCor ()
{
  // The base class owns param,  so we do not delete it.
  // delete  param;
  if  (usfCasCorClassifier)
  {
    delete  usfCasCorClassifier;
    usfCasCorClassifier = NULL;
  }
}


kkint32  ModelUsfCasCor::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = Model::MemoryConsumedEstimated () + 
                                   sizeof (usfCasCorClassifier);

  if  (usfCasCorClassifier)
    memoryConsumedEstimated += usfCasCorClassifier->MemoryConsumedEstimated ();
  return  memoryConsumedEstimated;
}



void   ModelUsfCasCor::CancelFlag (bool  _cancelFlag)
{
  Model::CancelFlag (_cancelFlag);
  if  (usfCasCorClassifier)
    usfCasCorClassifier->CancelFlag (_cancelFlag);
}




ModelUsfCasCorPtr  ModelUsfCasCor::Duplicate ()  const
{
  return new ModelUsfCasCor (*this);
}



ModelParamUsfCasCorPtr   ModelUsfCasCor::Param ()
{
  return param;
}



void  ModelUsfCasCor::TrainModel (FeatureVectorListPtr  _trainExamples,
                                  bool                  _alreadyNormalized,
                                  bool                  _takeOwnership, /*!< Model will take ownership of these examples */
                                  RunLog&               _log
                                 )
{
  _log.Level (10) << "ModelUsfCasCor::TrainModel[" << Name () << "]." << endl;

  if  (param == NULL)
  {
    validModel = false;
    KKStr  errMsg = "ModelUsfCasCor::TrainModel   (param == NULL)";
    _log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  if  (usfCasCorClassifier)
  {
    delete  usfCasCorClassifier;
    usfCasCorClassifier = NULL;
  }

  try 
  {
    Model::TrainModel (_trainExamples, _alreadyNormalized, _takeOwnership, _log);
  }
  catch (const KKException&  e)
  {
    validModel = false;
    KKStr  errMsg = "ModelUsfCasCor::TrainModel  ***ERROR*** Exception occurred calling 'Model::TrainModel'.";
    _log.Level (-1) << endl << errMsg << endl << e.ToString () << endl << endl;
    throw  KKException (errMsg, e);
  }
  catch (const exception& e2)
  {
    validModel = false;
    KKStr errMsg = "ModelUsfCasCor::TrainModel  ***ERROR*** Exception occurred calling 'Model::TrainModel'.";
    _log.Level (-1) << endl << endl << errMsg << endl << e2.what () << endl << endl;
    throw KKException (errMsg, e2);
  }
  catch (...)
  {
    validModel = false;
    KKStr errMsg = "ModelUsfCasCor::TrainModel  ***ERROR*** Exception occurred calling 'Model::TrainModel'.";
    _log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  // 'Model::TrainModel'  Will have performed any BitReduction and Feature Encoding 
  // that needed to be done.  
  // Also the data structures 'classes', 'encoder', and 'fileDesc' will have been built.
  // 'classes' will already be sorted in name order.
  // The Prediction variables 'probabilities', 'votes', and 'crossClassProbTable' will
  // have been built.

  TrainingTimeStart ();

  usfCasCorClassifier = new UsfCasCor ();

  usfCasCorClassifier->TrainNewClassifier (param->In_limit         (),
                                           param->Out_limit        (),
                                           param->Number_of_rounds (),
                                           param->Number_of_trials (),
                                           param->Random_seed      (),
                                           param->UseCache         (),
                                           trainExamples,
                                           SelectedFeatures (),
                                           _log
                                          );

  TrainingTimeEnd ();

  _log.Level (10) << "ModelUsfCasCor::TrainModel  Completed." << endl;
}  /* TrainModel */




MLClassPtr  ModelUsfCasCor::Predict (FeatureVectorPtr  example,
                                     RunLog&           log
                                    )
{
  if  (!usfCasCorClassifier)
  {
    log.Level (-1) << endl << endl << "ModelUsfCasCor::Predict   ***ERROR***      (usfCasCorClassifier == NULL)" << endl << endl;
    return NULL;
  }
  bool  newExampleCreated = false;
  FeatureVectorPtr  encodedExample = PrepExampleForPrediction (example, newExampleCreated);

  MLClassPtr  predictedClass = usfCasCorClassifier->PredictClass (encodedExample);
  if  (newExampleCreated)
  {
    delete encodedExample;
    encodedExample = NULL;
  }

  return  predictedClass;
}  /* Predict */




void  ModelUsfCasCor::Predict (FeatureVectorPtr example,
                               MLClassPtr       knownClass,
                               MLClassPtr&      predClass1,
                               MLClassPtr&      predClass2,
                               kkint32&         predClass1Votes,
                               kkint32&         predClass2Votes,
                               double&          probOfKnownClass,
                               double&          predClass1Prob,
                               double&          predClass2Prob,
                               kkint32&         numOfWinners,
                               bool&            knownClassOneOfTheWinners,
                               double&          breakTie,
                               RunLog&          log
                              )
{
  if  (!usfCasCorClassifier)
  {
    KKStr errMsg = "ModelUsfCasCor::Predict   ***ERROR***      (usfCasCorClassifier == NULL)";
    log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }


  VectorFloat  probabilities;

  float  pc1p = 0.0f;
  float  pc2p = 0.0f;
  float  kcp = 0.0f;

  bool  newExampleCreated = false;
  FeatureVectorPtr  encodedExample = PrepExampleForPrediction (example, newExampleCreated);
  usfCasCorClassifier->PredictConfidences (encodedExample,
                                           knownClass,
                                           predClass1, pc1p, 
                                           predClass2, pc2p,
                                           kcp,
                                           *classes,
                                           probabilities
                                          );

  if  (newExampleCreated)
  {
    delete encodedExample;
    encodedExample = NULL;
  }

  predClass1Prob   = pc1p;
  predClass2Prob   = pc2p;
  probOfKnownClass = kcp;

  breakTie = pc1p - pc2p;
  numOfWinners = 1;

  knownClassOneOfTheWinners = (predClass1 == knownClass);

  return;
}  /* Predict */





ClassProbListPtr  ModelUsfCasCor::ProbabilitiesByClass (FeatureVectorPtr  example,
                                                        RunLog&           log
                                                       )
{
  if  (!usfCasCorClassifier)
  {
    KKStr errMsg = "ModelUsfCasCor::ProbabilitiesByClass   ***ERROR***      (usfCasCorClassifier == NULL)";
    log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  bool  newExampleCreated = false;
  FeatureVectorPtr  encodedExample = PrepExampleForPrediction (example, newExampleCreated);

  ClassProbListPtr  results = usfCasCorClassifier->PredictClassConfidences (encodedExample);

  results->SortByProbability (true);  // 'true' = Sort High to Low.

  if  (newExampleCreated)
  {
    delete encodedExample;
    encodedExample = NULL;
  }

  return  results;
}  /* ProbabilitiesByClass */




void  ModelUsfCasCor::ProbabilitiesByClass (FeatureVectorPtr    example,
                                            const MLClassList&  _mlClasses,
                                            kkint32*            _votes,
                                            double*             _probabilities,
                                            RunLog&             log
                                           )
{
  if  (!usfCasCorClassifier)
  {
    KKStr errMsg = "ModelUsfCasCor::ProbabilitiesByClass   ***ERROR***      (usfCasCorClassifier == NULL)";
    log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  VectorFloat  probabilities;

  MLClassPtr  pc1 = NULL;
  MLClassPtr  pc2 = NULL;
  MLClassPtr  kc  = NULL;

  float  pc1p = 0.0f;
  float  pc2p = 0.0f;
  float  kcp = 0.0f;

  bool  newExampleCreated = false;
  FeatureVectorPtr  encodedExample = PrepExampleForPrediction (example, newExampleCreated);
  usfCasCorClassifier->PredictConfidences (encodedExample,
                                           kc,
                                           pc1, pc1p, 
                                           pc2, pc2p,
                                           kcp,
                                           _mlClasses,
                                           probabilities
                                          );
  if  (newExampleCreated)
  {
    delete encodedExample;
    encodedExample = NULL;
  }

  if  (_mlClasses.size () != probabilities.size ())
  {
    log.Level (-1) << endl << "ModelUsfCasCor::ProbabilitiesByClass   ***ERROR***"  << endl
      << "\"_mlClasses.size () != probabilities.size ()\"   This should not ever be able to happen." << endl
      << endl;
    for  (int x = 0;  x < _mlClasses.QueueSize ();  ++x)
    {
      _votes[x] = 0;
      _probabilities[x] = 0.0;
    }
  }
  else
  {
    float  highProb = -9999.99f;
    int   highProbIdx = -1;
    for  (kkuint32 x = 0;  x < probabilities.size ();  ++x)
    {
      float  p = probabilities[x];
      _votes[x] = 0;
      _probabilities[x] = p;
      if  (p > highProb)
      {
        highProbIdx = x;
        highProb = p;
      }
    }
    if  (highProbIdx >= 0)
      votes[highProbIdx] = 1;
  }

  return;
}  /* ProbabilitiesByClass */





void   ModelUsfCasCor::ProbabilitiesByClass (FeatureVectorPtr    _example,
                                             const MLClassList&  _mlClasses,
                                             double*             _probabilities,
                                             RunLog&             _log
                                            )
{
  if  (!usfCasCorClassifier)
  {
    KKStr errMsg = "ModelUsfCasCor::ProbabilitiesByClass   ***ERROR***      (usfCasCorClassifier == NULL)";
    _log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  VectorFloat  probabilities;

  MLClassPtr  pc1 = NULL;
  MLClassPtr  pc2 = NULL;
  MLClassPtr  kc  = NULL;

  float  pc1p = 0.0f;
  float  pc2p = 0.0f;
  float  kcp  = 0.0f;

  bool  newExampleCreated = false;
  FeatureVectorPtr  encodedExample = PrepExampleForPrediction (_example, newExampleCreated);
  usfCasCorClassifier->PredictConfidences (encodedExample,
                                           kc,
                                           pc1, pc1p, 
                                           pc2, pc2p,
                                           kcp,
                                           _mlClasses,
                                           probabilities
                                          );
  if  (newExampleCreated)
  {
    delete encodedExample;
    encodedExample = NULL;
  }

  if  (_mlClasses.size () != probabilities.size ())
  {
    _log.Level (-1) << endl << "ModelUsfCasCor::ProbabilitiesByClass   ***ERROR***"  << endl
      << "\"_mlClasses.size () != probabilities.size ()\"   This should not ever be able to happen." << endl
      << endl;
    for  (int x = 0;  x < _mlClasses.QueueSize ();  ++x)
    {
      _probabilities[x] = 0.0;
    }
  }
  else
  {
    for  (kkuint32 x = 0;  x < probabilities.size ();  ++x)
      _probabilities[x] = probabilities[x];
  }

  return;
}  /* ProbabilitiesByClass */
  



void  ModelUsfCasCor::ReadSpecificImplementationXML (istream&  i,
                                                     bool&     _successful,
                                                     RunLog&   log
                                                    )
{
  /**@todo  Make sure that 'param' != NULL */

  if  (param == NULL)
  {
    param = dynamic_cast<ModelParamUsfCasCorPtr> (Model::param);
  }
  else
  {
    log.Level (20) << "ModelUsfCasCor::ReadSpecificImplementationXML    param != NULL." << endl;
  }

  char  buff[20480];
  KKStr  field;

  KKStr  modelFileName;

  kkint32  numOfModels = 0;

  while  (i.getline (buff, sizeof (buff)))
  {
    KKStr  ln (buff);
    field = ln.ExtractQuotedStr ("\n\r\t", true);
    field.Upper ();

    if  (field.EqualIgnoreCase ("</ModelUsfCasCor>"))
    {
      break;
    }

    else if  (field.EqualIgnoreCase ("<Model>"))
    {
      Model::ReadXML (i, _successful, log);
    }

    else if  (field.EqualIgnoreCase ("<UsfCasCor>"))
    {
      delete  usfCasCorClassifier;
      usfCasCorClassifier = new UsfCasCor ();
      bool  usfCasCorSuccessful = false;
      usfCasCorClassifier->LoadExistingClassifier (i, usfCasCorSuccessful, log);
      if  (!usfCasCorSuccessful)
        _successful = false;
    }

    else
    {
      // Add code to deal with items that are specific to 'ModelUsfCasCor'
    }
  }

  if  (!_successful)
    validModel = false;

  return;
}  /* ReadSpecificImplementationXML */




void  ModelUsfCasCor::WriteSpecificImplementationXML (ostream&  o,
                                                      RunLog&   log
                                                     )
{
  log.Level (20) << "ModelUsfCasCor::WriteSpecificImplementationXML  Saving Model in File." << endl;

  o << "<ModelUsfCasCor>" << endl;

  if  (usfCasCorClassifier)
    usfCasCorClassifier->WriteXML (o);

  o << "</ModelUsfCasCor>" << endl;
} /* WriteSpecificImplementationXML */




kkint32  ModelUsfCasCor::NumOfSupportVectors ()  const
{
  return  0;
}  /* NumOfSupportVectors */






void  ModelUsfCasCor::WriteXML (const KKStr&  varName,
                                ostream&      o
                               )  const
{
  XmlTag  startTag ("ModelDual",  XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);
  o << endl;

  WriteModelXMLFields (o);  // Write the base class data fields 1st.

  usfCasCorClassifier->WriteXML ("UsfCasCorClassifier", o);

  XmlTag  endTag ("ModelDual", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}  /* WriteXML */





void  ModelUsfCasCor::ReadXML (XmlStream&      s,
                               XmlTagConstPtr  tag,
                               RunLog&         log
                              )
{
  delete  usfCasCorClassifier;
  usfCasCorClassifier = NULL;

  XmlTokenPtr  t = s.GetNextToken (log);
  while  (t)
  {
    t = ReadXMLModelToken (t, log);
    if  (t)
    {
      if  (t->VarName ().EqualIgnoreCase ("UsfCasCorClassifier")  &&  (typeid (*t) == typeid(XmlElementUsfCasCor)))
      {
        delete usfCasCorClassifier;
        usfCasCorClassifier = dynamic_cast<XmlElementUsfCasCorPtr>(t)->TakeOwnership ();
      }
      else
      {
        KKStr  errMsg (128);
        errMsg << "ModelUsfCasCor::ReadXML   ***ERROR***   Unexpected Element: Section: " << t->SectionName () << " VarName:" << t->VarName ();
        log.Level (-1) << endl << errMsg << endl << endl;
        AddErrorMsg (errMsg, 0);
      }
    }
    delete  t;
    t = s.GetNextToken (log);
  }

  if  (Model::param == NULL)
  {
    KKStr errMsg (128);
    errMsg << "ModelUsfCasCor::ReadXML  ***ERROR***  Base class 'Model' does not have 'param' defined.";
    AddErrorMsg (errMsg, 0);
    log.Level (-1) << endl << errMsg << endl << endl;
  }

  else if  (typeid (*Model::param) != typeid(ModelParamUsfCasCor))
  {
    KKStr errMsg (128);
    errMsg << "ModelUsfCasCor::ReadXML  ***ERROR***  Base class 'Model' param parameter is of the wrong type;  found: " << param->ModelParamTypeStr ();
    AddErrorMsg (errMsg, 0);
    log.Level (-1) << endl << errMsg << endl << endl;
  }

  else
  {
    param = dynamic_cast<ModelParamUsfCasCorPtr> (Model::param);
  }


}  /* ReadXML */


XmlFactoryMacro(ModelUsfCasCor)



