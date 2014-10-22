#include  "FirstIncludes.h"

#include  <stdio.h>
#include  <string>
#include  <iostream>
#include  <fstream>
#include  <math.h>
#include  <vector>
#include  <sstream>
#include  <iomanip>

#include  <set>
#include  <vector>


#include  "MemoryDebug.h"

using namespace  std;


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

using namespace  KKMachineLearning;



ModelKnn::ModelKnn (FileDescPtr    _fileDesc,
                    VolConstBool&  _cancelFlag,
                    RunLog&        _log
                   ):
  Model (_fileDesc, _cancelFlag, _log),
  param (NULL)
{
}




ModelKnn::ModelKnn (const KKStr&          _name,
                    const ModelParamKnn&  _param,         // Create new model from
                    FileDescPtr           _fileDesc,
                    VolConstBool&         _cancelFlag,
                    RunLog&               _log
                   ):
  Model (_name, _param, _fileDesc, _cancelFlag, _log),
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


MLClassPtr  ModelKnn::Predict (FeatureVectorPtr  example)
{
  return NULL;
}



void  ModelKnn::Predict (FeatureVectorPtr  example,
                         MLClassPtr        knownClass,
                         MLClassPtr&       predClass1,
                         MLClassPtr&       predClass2,
                         kkint32&          predClass1Votes,
                         kkint32&          predClass2Votes,
                         double&           probOfKnownClass,
                         double&           probOfPredClass1,
                         double&           probOfPredClass2,
                         kkint32&          numOfWinners,
                         bool&             knownClassOneOfTheWinners,
                         double&           breakTie
                        )
{
}  /* Predict */
                        



ClassProbListPtr  ModelKnn::ProbabilitiesByClass (FeatureVectorPtr  example)
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
  kkuint32  idx;
  for  (idx = 0;  idx < numOfClasses;  idx++)
  {
    MLClassPtr  ic = classesIndex->GetMLClass (idx);
    results->PushOnBack (new ClassProb (ic, 0.0, 0.0f));
  }

  return  results;
}  /* ProbabilitiesByClass */




void  ModelKnn::ProbabilitiesByClass (FeatureVectorPtr       example,
                                      const MLClassList&  _mlClasses,
                                      kkint32*                 _votes,
                                      double*                _probabilities
                                     )
{
}


void  ModelKnn::ProbabilitiesByClass (FeatureVectorPtr       _example,
                                      const MLClassList&  _mlClasses,
                                      double*                _probabilities
                                     )
{
}  /* ProbabilitiesByClass */




void  ModelKnn::ReadSpecificImplementationXML (istream&  i,
                                               bool&     _successful
                                              )
{
  char  buff[20480];
  KKStr  field;

  KKStr  modelFileName;

  kkint32  numOfModels = 0;

  while  (i.getline (buff, sizeof (buff)))
  {
    KKStr  ln (buff);
    field = ln.ExtractQuotedStr ("\n\r\t", true);
    field.Upper ();

    if  (field.EqualIgnoreCase ("</ModelKnn>"))
    {
      break;
    }

    else if  (field.EqualIgnoreCase ("<Model>"))
    {
      Model::ReadXML (i, _successful);
    }

    else
    {
      // Add code tro deal with items that are specific to 'ModelSvmBase'
    }
  }

  if  (!_successful)
    validModel = false;

  return;
}  /* ReadSpecificImplementationXML */



void  ModelKnn::TrainModel (FeatureVectorListPtr  _trainExamples,
                            bool                  _alreadyNormalized,
                            bool                  _takeOwnership  /**< Model will take ownership of these examples */
                           )
{
  log.Level (20) << "ModelKnn::TrainModel    alreadyNormalized[" << _alreadyNormalized << "]  _takeOwnership[" << _takeOwnership << "]." << endl;

  try
  {
    Model::TrainModel (_trainExamples, _alreadyNormalized, _takeOwnership);
  }
  catch  (const KKException& e)
  {
    validModel = false;
    throw e;
  }
}  /* TrainModel */



void  ModelKnn::WriteSpecificImplementationXML (ostream&  o)
{
  log.Level (20) << "ModelKnn::WriteSpecificImplementationXML  Saving Model in File." << endl;

  o << "<ModelKnn>" << endl;

  o << "</ModelKnn>" << endl;
}  /* WriteSpecificImplementationXML */
