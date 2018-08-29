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
using namespace  std;


#include "GlobalGoalKeeper.h"
#include "KKBaseTypes.h"
#include "KKException.h"
#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace  KKB;


#include "Model.h"
#include "ClassProb.h"
#include "FeatureNumList.h"
#include "FeatureVector.h"
#include "ModelKnn.h"
using namespace  KKMLL;



ModelKnn::ModelKnn ():
  Model (),
  param (NULL)
{
}



ModelKnn::ModelKnn (FactoryFVProducerPtr  _factoryFVProducer):
  Model (_factoryFVProducer),
  param (NULL)
{
}



ModelKnn::ModelKnn (const KKStr&          _name,
                    const ModelParamKnn&  _param,         // Create new model from
                    FactoryFVProducerPtr  _factoryFVProducer
                   ):
  Model (_name, _param, _factoryFVProducer),
  param (NULL)
{
  param = dynamic_cast<ModelParamKnnPtr> (Model::param);
}



ModelKnn::ModelKnn (const ModelKnn&   _model):
  Model (_model),
  param (NULL)
{
  param = dynamic_cast<ModelParamKnnPtr> (Model::param);
}



ModelKnn::~ModelKnn ()
{

}



ModelKnnPtr  ModelKnn::Duplicate ()  const
{
  return  new ModelKnn (*this);
}



ModelParamKnnPtr  ModelKnn::Param ()
{
  return  param;
}



MLClassPtr  ModelKnn::Predict (FeatureVectorPtr  example,
                               RunLog&           log
                              )
{
  log.Level (-1) << "ModelKnn::Predict  ***ERROR***   Not Supported   example: " << example->ExampleFileName () << endl;
  return NULL;
}



void  ModelKnn::Predict (FeatureVectorPtr  example,
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
  log.Level (-1) << "ModelKnn::Predict  ***ERROR***   Not Supported" << endl
      << "example: " << example->ExampleFileName () << endl
      << (knownClass ? 1 : 0) << (predClass1 ? 1 : 0) << (predClass2 ? 1 : 0) << endl;

  predClass1Votes = 0;
  predClass2Votes = 0;
  probOfKnownClass = 0;
  predClass1Prob = 0.0;
  predClass2Prob = 0.0;
  numOfWinners = 0;
  knownClassOneOfTheWinners = false;
  breakTie = 0.0;
}  /* Predict */
                        


ClassProbListPtr  ModelKnn::ProbabilitiesByClass (FeatureVectorPtr  example,
                                                  RunLog&           log
                                                 )
{
  if  (!classesIndex)
  {
    KKStr errMsg = "ModelKnn::ProbabilitiesByClass   ***ERROR***      (classLabelAssigments == NULL)";
    log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  bool  newExampleCreated = false;
  FeatureVectorPtr  encodedExample = PrepExampleForPrediction (example, newExampleCreated);

  //  double  y = SVM289::svm_predict_probability (svmModel,  *encodedExample, classProbs, votes);

  if  (newExampleCreated)
  {
    delete encodedExample;
    encodedExample = NULL;
  }

  ClassProbListPtr  results = new ClassProbList ();
  kkint32 idx;
  for  (idx = 0;  idx < numOfClasses;  idx++)
  {
    MLClassPtr  ic = classesIndex->GetMLClass (idx);
    results->PushOnBack (new ClassProb (ic, 0.0, 0.0f));
  }

  return  results;
}  /* ProbabilitiesByClass */



void  ModelKnn::ProbabilitiesByClass (FeatureVectorPtr    example,
                                      const MLClassList&  _mlClasses,
                                      kkint32*            _votes,
                                      double*             _probabilities,
                                      RunLog&             log
                                     )
{
}



void  ModelKnn::ProbabilitiesByClass (FeatureVectorPtr    _example,
                                      const MLClassList&  _mlClasses,
                                      double*             _probabilities,
                                      RunLog&             log
                                     )
{
}  /* ProbabilitiesByClass */



void  ModelKnn::TrainModel (FeatureVectorListPtr  _trainExamples,
                            bool                  _alreadyNormalized,
                            bool                  _takeOwnership,  /**< Model will take ownership of these examples */
                            VolConstBool&         _cancelFlag,
                            RunLog&               _log
                           )
{
  _log.Level (20) << "ModelKnn::TrainModel    alreadyNormalized[" << _alreadyNormalized << "]  _takeOwnership[" << _takeOwnership << "]." << endl;

  try
  {
    Model::TrainModel (_trainExamples, _alreadyNormalized, _takeOwnership, _cancelFlag, _log);
  }
  catch  (const KKException& e)
  {
    validModel = false;
    throw e;
  }
}  /* TrainModel */



void  ModelKnn::WriteXML (const KKStr&  varName,
                          ostream&      o
                         )  const
{
  XmlTag  startTag ("ModelParamDual",  XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);
  o << endl;

  WriteModelXMLFields (o); // Write the base class data fields 1st.
  if  (param)
    param->WriteXML ("Param", o);

  XmlTag  endTag ("ModelParamDual", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}  /* WriteXML */



void  ModelKnn::ReadXML (XmlStream&      s,
                         XmlTagConstPtr  tag,
                         VolConstBool&   cancelFlag,
                         RunLog&         log
                        )
{
  XmlTokenPtr  t = s.GetNextToken (cancelFlag, log);
  while  (t  &&  (!cancelFlag))
  {
    t = ReadXMLModelToken (t, log);
    if  (t)
    {
      const KKStr&  varName = t->VarName ();

      if  ((varName.EqualIgnoreCase ("Param"))  &&  (typeid (*t) == typeid (XmlElementModelParamKnn)))
      {
        delete  param;
        param = dynamic_cast<XmlElementModelParamKnnPtr> (t)->TakeOwnership ();
      }
      else
      {
        KKStr  errMsg (256);
        errMsg << "Invalid XmlElement Encountered:  Section: " << t->SectionName () << "  VarName: " << varName;
        log.Level (-1) << endl << errMsg << endl << endl;
        AddErrorMsg (errMsg, 0);
      }
    }
    delete  t;
    t = s.GetNextToken (cancelFlag, log);
  }

  delete  t;
  t = NULL;

  if  (!param)
    param = dynamic_cast<ModelParamKnnPtr> (Model::param);

  if  (Model::param == NULL)
  {
    KKStr errMsg (128);
    errMsg << "ModelKnn::ReadXML  ***ERROR***  Base class 'Model' does not have 'param' defined.";
    AddErrorMsg (errMsg, 0);
    log.Level (-1) << endl << errMsg << endl << endl;
  }

  else if  (typeid (*Model::param) != typeid(ModelParamKnn))
  {
    KKStr errMsg (128);
    errMsg << "ModelKnn::ReadXML  ***ERROR***  Base class 'Model' param parameter is of the wrong type;  found: " << Model::param->ModelParamTypeStr ();
    AddErrorMsg (errMsg, 0);
    log.Level (-1) << endl << errMsg << endl << endl;
  }

  else
  {
    param = dynamic_cast<ModelParamKnnPtr> (Model::param);
  }

  ReadXMLModelPost (log);
}  /* ReadXML */


XmlFactoryMacro(ModelKnn)
