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
#include "KKBaseTypes.h"
#include "KKException.h"
using namespace  std;


#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace  KKB;


#include "ModelOldSVM.h"
#include "ModelParamOldSVM.h"
#include "BinaryClassParms.h"
#include "FeatureEncoder2.h"
#include "FeatureNumList.h"
#include "FeatureVector.h"
#include "SVMparam.h"
using namespace KKMachineLearning;




ModelOldSVM::ModelOldSVM (FileDescPtr    _fileDesc,
                          VolConstBool&  _cancelFlag,
                          RunLog&        _log
                         ):
  Model (_fileDesc, _cancelFlag, _log),
  assignments (NULL),
  svmModel    (NULL)
{
  Model::param = new ModelParamOldSVM (_fileDesc, _log);
}



ModelOldSVM::ModelOldSVM (const KKStr&            _name,
                          const ModelParamOldSVM& _param,         // Create new model from
                          FileDescPtr             _fileDesc,
                          VolConstBool&           _cancelFlag,
                          RunLog&                 _log
                         )
:
  Model   (_name, _param, _fileDesc, _cancelFlag, _log),
  assignments (NULL),
  svmModel    (NULL)

{
}



ModelOldSVM::ModelOldSVM (const ModelOldSVM& _model)
:
  Model (_model),
  assignments (NULL),
  svmModel    (NULL)

{
}




ModelOldSVM::~ModelOldSVM ()
{
  log.Level (20) << "ModelOldSVM::~ModelOldSVM   Starting Destructor for Model[" << rootFileName << "]" << endl;

  delete  svmModel;
  svmModel = NULL;

  delete  assignments;
  assignments = NULL;
}


kkint32  ModelOldSVM::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = Model::MemoryConsumedEstimated () + 
                                 sizeof (ClassAssignmentsPtr) + 
                                 sizeof (SVMModelPtr);

  if  (assignments)   memoryConsumedEstimated += assignments->MemoryConsumedEstimated ();
  if  (svmModel)      memoryConsumedEstimated += svmModel->MemoryConsumedEstimated ();
  return  memoryConsumedEstimated;
}


ModelOldSVMPtr  ModelOldSVM::Duplicate ()  const
{
  return new ModelOldSVM (*this);
}


const ClassAssignments&  ModelOldSVM::Assignments ()  const
{
  return svmModel->Assignments ();
}



FeatureNumList  ModelOldSVM::GetFeatureNums (MLClassPtr  class1,
                                             MLClassPtr  class2
                                            )
{
  return svmModel->GetFeatureNums (class1, class2);
}  /* GetFeatureNums */




const FeatureNumList&   ModelOldSVM::GetFeatureNums ()  const
{
  return svmModel->GetFeatureNums ();
}



kkint32  ModelOldSVM::NumOfSupportVectors () const
{
  return svmModel->NumOfSupportVectors ();
}



void  ModelOldSVM::SupportVectorStatistics (kkint32& numSVs,
                                            kkint32& totalNumSVs
                                           )
{
  return  svmModel->SupportVectorStatistics (numSVs, totalNumSVs);
}


ModelParamOldSVMPtr   ModelOldSVM::Param () const
{
  if  (param == NULL)
  {
    KKStr errMsg = "ModelOldSVM::Param   ***ERROR***  param not defined (param == NULL).";
    log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  if  (param->ModelParamType () != ModelParam::mptOldSVM)
  {
    KKStr errMsg = "ModelOldSVM::Param   ***ERROR***  param variable of wrong type.";
    log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  return  dynamic_cast<ModelParamOldSVMPtr> (param);
}



SVM_SelectionMethod  ModelOldSVM::SelectionMethod () const
{
  return  svmModel->SelectionMethod ();
}




void  ModelOldSVM::WriteSpecificImplementationXML (ostream& o)
{
  log.Level (40) << "ModelOldSVM::WriteSpecificImplementationXML  Saving Model in File." << endl;

  bool successful = true;
  o << "<ModelOldSVM>" << endl;
  svmModel->Write (o, rootFileName, successful);
  o << "</ModelOldSVM>" << endl;

} /* WriteSpecificImplementationXML */





void  ModelOldSVM::ReadSpecificImplementationXML (istream&  i,
                                                  bool&     _successful
                                                 )
{
  log.Level (40) << "ModelOldSVM::ReadSpecificImplementationXML" << endl;
  char  buff[20480];

  delete  svmModel;  svmModel = NULL;

  while (i.getline (buff, sizeof (buff)))
  {
    KKStr  ln (buff);
    if  (ln.Len () < 1)
      continue;

    if  ((ln[(kkint16)0] == '/')  &&  (ln[(kkint16)1] == '/'))
      continue;

    KKStr  lineName = ln.ExtractToken2 ("\t\n\r");
    if  (lineName.EqualIgnoreCase ("</ModelOldSVM>"))
      break;

    if  (lineName.EqualIgnoreCase ("<SVMModel>"))
    {
      delete  svmModel;
      svmModel = NULL;

      try
      {
        svmModel = new SVMModel (i, _successful, fileDesc, log, cancelFlag);
      }
      catch (...)
      {
        log.Level (-1) << endl << endl << "ModelOldSVM::ReadSpecificImplementationXML   ***ERROR***  Exception occured calling 'new SVMModel'." << endl << endl;
        validModel = false;
        _successful = false;
      }

      if  (!_successful)
      {
        validModel = false;
        log.Level (-1) << endl << endl << "ModelOldSVM::ReadSpecificImplementationXML   ***ERROR***  Could not loadf model in SVMModel::SVMModel." << endl << endl;
      }
    }
  }


  if  (_successful  ||  validModel)
  {
    if  (svmModel == NULL)
    {
      _successful = false;
      validModel  = false;

      log.Level (-1) << endl << endl
        << "ModelOldSVM::ReadSpecificImplementationXML  ***ERROR***   'svmModel' was not defined." << endl
        << endl;
    }
  }

  log.Level (40) << "ModelOldSVM::ReadSpecificImplementationXML   Exiting" << endl;

}  /* ReadSpecificImplementationXML */




void   ModelOldSVM::Predict (FeatureVectorPtr  example,
                             MLClassPtr     knownClass,
                             MLClassPtr&    predClass,
                             MLClassPtr&    predClass2,
                             kkint32&          predClass1Votes,
                             kkint32&          predClass2Votes,
                             double&           probOfKnownClass,
                             double&           probOfPredClass,
                             double&           probOfPredClass2,
                             kkint32&          numOfWinners,
                             bool&             knownClassOneOfTheWinners,
                             double&           breakTie
                            )
{
  bool  newExampleCreated = false;
  FeatureVectorPtr  encodedExample = PrepExampleForPrediction (example, newExampleCreated);
  svmModel->Predict (encodedExample, knownClass, predClass, predClass2,
                     predClass1Votes,  predClass2Votes,
                     probOfKnownClass, 
                     probOfPredClass,  probOfPredClass2,
                     numOfWinners,
                     knownClassOneOfTheWinners,
                     breakTie
                    );
  if  (newExampleCreated)
  {
    delete encodedExample;
    encodedExample = NULL;
  }
  
  return;
}  /* Predict */





MLClassPtr  ModelOldSVM::Predict (FeatureVectorPtr  example)
{
  bool  newExampleCreated = false;
  FeatureVectorPtr  encodedExample = PrepExampleForPrediction (example, newExampleCreated);
  MLClassPtr  c = svmModel->Predict (example);

  if  (newExampleCreated)
  {
    delete encodedExample;
    encodedExample = NULL;
  }

  return c;
}  /* Predict */




ClassProbListPtr  ModelOldSVM::ProbabilitiesByClass (FeatureVectorPtr  example)
{
  if  (!svmModel)
  {
    KKStr  errMsg = "ModelOldSVM::ProbabilitiesByClass   ***ERROR***      (svmModel == NULL)";
    log.Level (-1) << endl << errMsg << endl;
    throw KKException (errMsg);
  }

  bool  newExampleCreated = false;
  FeatureVectorPtr  encodedExample = PrepExampleForPrediction (example, newExampleCreated);
  svmModel->ProbabilitiesByClass (encodedExample, *classes, votes, classProbs);

  if  (newExampleCreated)
  {
    delete encodedExample;
    encodedExample = NULL;
  }
  

  ClassProbListPtr  results = new ClassProbList ();
  kkuint32  idx;
  for  (idx = 0;  idx < numOfClasses;  idx++)
  {
    MLClassPtr  ic = classes->IdxToPtr (idx);
    results->PushOnBack (new ClassProb (ic, classProbs[idx], (float)votes[idx]));
  }

  return  results;
}  /* ProbabilitiesByClass */






void  ModelOldSVM::ProbabilitiesByClass (FeatureVectorPtr       _example,
                                         const MLClassList&  _mlClasses,
                                         double*                _probabilities
                                        )
{
  bool  newExampleCreated = false;
  FeatureVectorPtr  encodedExample = PrepExampleForPrediction (_example, newExampleCreated);

  svmModel->ProbabilitiesByClass (encodedExample, _mlClasses, votes, _probabilities);
  if  (newExampleCreated)
  {
    delete encodedExample;
    encodedExample = NULL;
  }

  return;
}




void  ModelOldSVM::ProbabilitiesByClass (FeatureVectorPtr        example,
                                         const MLClassList&  _mlClasses,
                                         kkint32*                 _votes,
                                         double*                _probabilities
                                        )
{
  bool  newExampleCreated = false;
  FeatureVectorPtr  encodedExample = PrepExampleForPrediction (example, newExampleCreated);
  svmModel->ProbabilitiesByClass (encodedExample, _mlClasses, _votes, _probabilities);
  if  (newExampleCreated)
  {
    delete encodedExample;
    encodedExample = NULL;
  }

  return;
}  /* ProbabilitiesByClass */







vector<KKStr>  ModelOldSVM::SupportVectorNames (MLClassPtr     c1,
                                                MLClassPtr     c2
                                               )  const
{
  return  svmModel->SupportVectorNames (c1, c2);
}  /* SupportVectorNames */







vector<KKStr>  ModelOldSVM::SupportVectorNames () const
{
  return  svmModel->SupportVectorNames ();
}  /* SupportVectorNames */







vector<ProbNamePair>  ModelOldSVM::FindWorstSupportVectors (FeatureVectorPtr  example,
                                                            kkint32           numToFind,
                                                            MLClassPtr     c1,
                                                            MLClassPtr     c2
                                                           )
{
  return  svmModel->FindWorstSupportVectors (example, numToFind, c1, c2);
}  /* FindWorstSupportVectors */






vector<ProbNamePair>  ModelOldSVM::FindWorstSupportVectors2 (FeatureVectorPtr  example,
                                                             kkint32           numToFind,
                                                             MLClassPtr     c1,
                                                             MLClassPtr     c2
                                                            )
{
  return  svmModel->FindWorstSupportVectors2 (example, numToFind, c1, c2);
}  /* FindWorstSupportVectors2 */





bool  ModelOldSVM::NormalizeNominalAttributes ()  const
{
  return  svmModel->NormalizeNominalAttributes ();
}  /* NormalizeNominalAttributes */




void  ModelOldSVM::RetrieveCrossProbTable (MLClassList&   classes,
                                           double**          crossProbTable  // two dimension matrix that needs to be classes.QueueSize ()  squared.
                                          )
{
  svmModel->RetrieveCrossProbTable (classes, crossProbTable);
  return;
}  /* RetrieveCrossProbTable */




void  ModelOldSVM::TrainModel (FeatureVectorListPtr  _trainExamples,
                               bool                  _alreadyNormalized,
                               bool                  _takeOwnership  /*!< Model will take ownership of these examples */
                              )
{
  log.Level (20) << "ModelOldSVM::TrainModel - Constructing From Training Data, Model[" << rootFileName << "]" << endl;
  // We do not bother with the base class 'TrainModel' like we do with other models.
  // 'ModelOldSVM' is a special case.  All we are trying to do is create a pass through
  // for 'svmModel'.

  delete  svmModel;
  svmModel = NULL;

  Model::TrainModel (_trainExamples, _alreadyNormalized, _takeOwnership);
  // The "Model::TrainModel" may have manipulated the '_trainExamples'.  It will have also 
  // updated 'Model::trainExamples.  So from this point forward we use 'trainExamples'.
  _trainExamples = NULL;


  SVMparamPtr svmParam = Param ()->SvmParameters ();
  if  (!svmParam)
  {
    validModel = false;
    KKStr errMsg = " ModelOldSVM::TrainModel  ***ERROR***    (svmParam == NULL).";
    log.Level (-1) << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  {
    delete  classes;
    classes = trainExamples->ExtractListOfClasses ();
    classes->SortByName ();
    numOfClasses = classes->QueueSize ();
    delete  assignments;
    assignments = new ClassAssignments (*classes, log);
  }

  try
  {
    TrainingTimeStart ();
    svmModel = new SVMModel (*svmParam, *trainExamples, *assignments, fileDesc, log, cancelFlag);
    TrainingTimeEnd ();
  }
  catch (...)
  {
    log.Level (-1) << endl << "ModelOldSVM::TrainModel  Exception occured building training model." << endl << endl;
    validModel = false;
    delete  svmModel;
    svmModel = NULL;
  }

  if  (weOwnTrainExamples)
  {
    // We are done with the training examples and since we were to take ownership,  we can delete them.
    delete trainExamples;
    trainExamples = NULL;
  }
}  /* TrainModel */






FeatureVectorPtr  ModelOldSVM::PrepExampleForPrediction (FeatureVectorPtr  fv,
                                                         bool&             newExampleCreated
                                                        )
{
  FeatureVectorPtr  oldFV = NULL;
  newExampleCreated = false;
  if  ((!alreadyNormalized)  &&  (normParms))
  {
    oldFV = fv;
    fv = normParms->ToNormalized (fv);
    if  (newExampleCreated)
    {
      delete  oldFV;
      oldFV = NULL;
    }
    newExampleCreated = true;
  }

  // Since 'SvmModel' will be doing the encoding we do not need to do it here.

  return  fv;
}  /* PrepExampleForPrediction */

