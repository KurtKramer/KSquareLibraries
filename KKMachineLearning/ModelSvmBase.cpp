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


#include "ModelSvmBase.h"
#include "ClassProb.h"
#include "FeatureNumList.h"
#include "FeatureVector.h"
#include "svm2.h"
using namespace  KKMLL;




ModelSvmBase::ModelSvmBase ():
  Model (),
  param     (NULL),
  svmModel  (NULL)
{
}


ModelSvmBase::ModelSvmBase (FileDescPtr _fileDesc):
  Model (_fileDesc),
  param     (NULL),
  svmModel  (NULL)
{
}



ModelSvmBase::ModelSvmBase (const KKStr&             _name,
                            const ModelParamSvmBase& _param,         // Create new model from
                            FileDescPtr              _fileDesc
                           ):
  Model (_name, _param, _fileDesc),
  param     (NULL),
  svmModel  (NULL)
{
  param = dynamic_cast<ModelParamSvmBasePtr> (Model::param);
}




ModelSvmBase::ModelSvmBase (const ModelSvmBase&   _model):
  Model (_model),
  param     (NULL),
  svmModel  (NULL)
{
  param = dynamic_cast<ModelParamSvmBasePtr> (Model::param);
  if  (_model.svmModel)
  {
    svmModel = new SVM289_MFS::Svm_Model (*_model.svmModel, fileDesc);
  }
}





/**
 * @brief Frees any memory allocated by, and owned by the ModelSvmBase
 */
ModelSvmBase::~ModelSvmBase ()
{
  // The base class owns param,  so we do not delete it.
  // delete  param;
  if  (svmModel)
  {
    svm_destroy_model (svmModel);   // 'svm_destroy_model'  will also delete  'svmModel'.
    delete  svmModel;
    svmModel = NULL;
  }
}



kkint32  ModelSvmBase::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = Model::MemoryConsumedEstimated () + 
                                 sizeof (SVM289_MFS::Svm_Model*)   + 
                                 sizeof (ModelParamSvmBasePtr);

  if  (svmModel)
    memoryConsumedEstimated += svmModel->MemoryConsumedEstimated ();
  return  memoryConsumedEstimated;
}




void  ModelSvmBase::CancelFlag (bool  _cancelFlag)
{
  Model::CancelFlag (_cancelFlag);
  svmModel->CancelFlag (_cancelFlag);
}



ModelSvmBasePtr  ModelSvmBase::Duplicate ()  const
{
  return new ModelSvmBase (*this);
}




KKStr  ModelSvmBase::Description ()  const
{
  KKStr  result = "SvmBase(" + Name () + ")";

  if  (param)
  {
    const SVM289_MFS::svm_parameter&  svmParam = param->SvmParam ();
    
    //result << " " << MachineTypeToStr     (svmParam.MachineType ())
    //       << " " << SelectionMethodToStr (svmParam.SelectionMethod ());
  }
  return  result;
}



ModelParamSvmBasePtr   ModelSvmBase::Param ()
{
  return param;
}



void  ModelSvmBase::TrainModel (FeatureVectorListPtr  _trainExamples,
                                bool                  _alreadyNormalized,
                                bool                  _takeOwnership,  /*!< Model will take ownership of these examples */
                                RunLog&               _log
                               )
{
  _log.Level (10) << "ModelSvmBase::TrainModel[" << param->FileName () << "]." << endl;

  if  (param == NULL)
  {
    validModel = false;
    KKStr  errMsg = "ModelSvmBase::TrainModel   (param == NULL)";
    _log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  if  (svmModel)
  {
    svm_destroy_model (svmModel);
    delete  svmModel;
    svmModel = NULL;
  }

  try 
  {
    Model::TrainModel (_trainExamples, _alreadyNormalized, _takeOwnership, _log);
  }
  catch (const KKException&  e)
  {
    validModel = false;
    KKStr  errMsg = "ModelSvmBase::TrainModel  ***ERROR*** Exception occurred calling 'Model::TrainModel'.";
    _log.Level (-1) << endl << errMsg << endl << e.ToString () << endl << endl;
    throw  KKException (errMsg, e);
  }
  catch (const exception& e2)
  {
    validModel = false;
    KKStr errMsg = "ModelSvmBase::TrainModel  ***ERROR*** Exception occurred calling 'Model::TrainModel'.";
    _log.Level (-1) << endl << errMsg << endl << e2.what () << endl << endl;
    throw KKException (errMsg, e2);
  }
  catch (...)
  {
    validModel = false;
    KKStr errMsg = "ModelSvmBase::TrainModel  ***ERROR*** Exception occurred calling 'Model::TrainModel'.";
    _log.Level (-1) << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }
    

  // 'Model::TrainModel'  Will have performed any Feature Encoding that needed to be done.  
  // Also the data structures 'classes', 'encoder', and 'fileDesc' will have been built.
  // 'classes' will already be sorted in name order.
  // The Prediction variables 'probabilities', 'votes', and 'crossClassProbTable' will
  // have been built.


  // Build the Label array that libSVM expects.
  float*  y = new float[trainExamples->QueueSize ()];
  {
    for  (kkint32 labelIndex = 0;  labelIndex < trainExamples->QueueSize ();  labelIndex++)
    {
      kkint16  label = classesIndex->GetClassIndex (trainExamples->IdxToPtr (labelIndex)->MLClass ());
      if  (label < 0)
      {
        _log.Level (-1) << endl << " ModelSvmBase::TrainModel   ***ERROR***   Label computed to -1; should not be able to happen." << endl << endl;
      }
      y[labelIndex] = (float)label;
    }
  }

  SVM289_MFS::svm_problem  prob (*trainExamples, y, *(param->SelectedFeatures ()));
  delete  y;  y = NULL;

  try
  {
    TrainingTimeStart ();
    svmModel = SVM289_MFS::svm_train (prob, param->SvmParam (), _log);
    TrainingTimeEnd ();
  }
  catch (const std::exception&  e)
  {
    validModel = false;
    KKStr  errMsg = "ModelSvmBase::TrainModel   ***ERROR*** Exception occurred in 'SVM289_MFS::svm_train' building training model[" + rootFileName + "].";
    errMsg << endl << "        Exception[" << e.what () << "]";
    _log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }
  catch (...)
  {
    validModel = false;
    KKStr  errMsg = "ModelSvmBase::TrainModel   ***ERROR*** Exception occurred in 'SVM289_MFS::svm_train' building training model[" + rootFileName + "].";
    _log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  if  (svmModel == NULL)
  {
    validModel = false;
    KKStr  errMsg = "ModelSvmBase::TrainModel   ***ERROR*** Building 'LibSVM' training model[" + rootFileName + "].";
    _log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }
}  /* TrainModel */



MLClassPtr  ModelSvmBase::Predict (FeatureVectorPtr  example,
                                   RunLog&           log
                                  )
{
  if  (!svmModel)
  {
    log.Level (-1) << endl << endl << "ModelSvmBase::Predict   ***ERROR***      (svmModel == NULL)" << endl << endl;
    return NULL;
  }

  if  (!classesIndex)
  {
    log.Level (-1) << endl << endl << "ModelSvmBase::Predict   ***ERROR***      (classesIndex == NULL)" << endl << endl;
    return NULL;
  }

  bool  newExampleCreated = false;
  FeatureVectorPtr  encodedExample = PrepExampleForPrediction (example, newExampleCreated);
  double  y = (kkint32)SVM289_MFS::svm_predict  (svmModel, *encodedExample);
  if  (newExampleCreated)
  {
    delete encodedExample;
    encodedExample = NULL;
  }

  kkint16  label = (kkint16)y;

  MLClassPtr  ic = classesIndex->GetMLClass (label);
  if  (!ic)
    log.Level (-1) << endl << endl << "ModelSvmBase::Predict   ***ERROR***      Label[" << y << "] Returned from the SVM was not in the ClassAssignments list." << endl << endl;

  return  ic;
}  /* Predict */




void  ModelSvmBase::Predict (FeatureVectorPtr  example,
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
  if  (!svmModel)
  {
    KKStr errMsg = "ModelSvmBase::Predict   ***ERROR***      (svmModel == NULL)";
    log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  if  (!classesIndex)
  {
    KKStr errMsg = "ModelSvmBase::Predict   ***ERROR***      (classesIndex == NULL)";
    log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  kkint32  knownClassIdx = classesIndex->GetClassIndex (knownClass);

  bool  newExampleCreated = false;
  FeatureVectorPtr  encodedExample = PrepExampleForPrediction (example, newExampleCreated);

  double  y = SVM289_MFS::svm_predict_probability (svmModel,  *encodedExample, classProbs, votes);

  if  (newExampleCreated)
  {
    delete encodedExample;
    encodedExample = NULL;
  }

  kkint32  maxIndex1 = 0;
  kkint32  maxIndex2 = 0;

  double  maxProb1 = -1.0;
  double  maxProb2 = -1.0;

  numOfWinners = 0;
  kkint32  winnerNumVotes = 0;

  for  (kkint32 idx = 0;  idx < numOfClasses;  ++idx)
  {
    if  (classProbs[idx] > maxProb1)
    {
      maxProb2  = maxProb1;
      maxIndex2 = maxIndex1;
      maxProb1  = classProbs[idx];
      maxIndex1 = idx;
    }
    else if  (classProbs[idx] > maxProb2)
    {
      maxProb2 = classProbs[idx];
      maxIndex2 = idx;
    }

    if  (votes[idx] > winnerNumVotes)
    {
      // We have a new winner.
      knownClassOneOfTheWinners = false;
      numOfWinners = 1;
      winnerNumVotes = votes[idx];
      if  (knownClassIdx == idx) 
        knownClassOneOfTheWinners = true;
    }
    else if  (votes[idx] == winnerNumVotes)
    {
      numOfWinners++;
      if  (knownClassIdx == idx) 
        knownClassOneOfTheWinners = true;
    }
  }

  predClass1 = classesIndex->GetMLClass (maxIndex1);
  predClass2 = classesIndex->GetMLClass (maxIndex2);
  predClass1Votes = votes[maxIndex1];
  predClass2Votes = votes[maxIndex2];
  predClass1Prob = maxProb1;
  predClass2Prob = maxProb2;

  breakTie = predClass1Prob - predClass2Prob;

  if  ((knownClassIdx < 0)  ||  (knownClassIdx >= numOfClasses))
  {
    probOfKnownClass = 0.0;
    knownClassOneOfTheWinners = false;
  }
  else
  {
    probOfKnownClass = (float)classProbs[knownClassIdx];
  }

  return;
}  /* Predict */





ClassProbListPtr  ModelSvmBase::ProbabilitiesByClass (FeatureVectorPtr  example,
                                                      RunLog&           log
                                                     )
{
  if  (!svmModel)
  {
    KKStr errMsg = "ModelSvmBase::ProbabilitiesByClass   ***ERROR***      (svmModel == NULL)";
    log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  if  (!classesIndex)
  {
    KKStr errMsg = "ModelSvmBase::ProbabilitiesByClass   ***ERROR***      (classesIndex == NULL)";
    log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  bool  newExampleCreated = false;
  FeatureVectorPtr  encodedExample = PrepExampleForPrediction (example, newExampleCreated);

  double  y = SVM289_MFS::svm_predict_probability (svmModel,  *encodedExample, classProbs, votes);

  if  (newExampleCreated)
  {
    delete encodedExample;
    encodedExample = NULL;
  }

  ClassProbListPtr  results = new ClassProbList ();
  kkint32 idx = 0;
  for  (idx = 0;  idx < numOfClasses;  idx++)
  {
    MLClassPtr  ic = classesIndex->GetMLClass (idx);
    results->PushOnBack (new ClassProb (ic, classProbs[idx], (float)votes[idx]));
  }

  results->SortByVotes (true);

  return  results;
}  /* ProbabilitiesByClass */




void  ModelSvmBase::ProbabilitiesByClass (FeatureVectorPtr    example,
                                          const MLClassList&  _mlClasses,
                                          kkint32*            _votes,
                                          double*             _probabilities,
                                          RunLog&             _log
                                         )
{
  if  (!svmModel)
  {
    KKStr errMsg = "ModelSvmBase::ProbabilitiesByClass   ***ERROR***      (svmModel == NULL)";
    _log.Level (-1) << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  if  (!classesIndex)
  {
    KKStr errMsg = "ModelSvmBase::ProbabilitiesByClass   ***ERROR***      (classesIndex == NULL)";
    _log.Level (-1) << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  bool  newExampleCreated = false;
  FeatureVectorPtr  encodedExample = PrepExampleForPrediction (example, newExampleCreated);

  double  y = SVM289_MFS::svm_predict_probability (svmModel,  *encodedExample, classProbs, votes);

  if  (newExampleCreated)
  {
    delete encodedExample;
    encodedExample = NULL;
  }

  kkuint32  idx;
  for  (idx = 0;  idx < _mlClasses.size ();  idx++)
  {
    MLClassPtr  ic = _mlClasses.IdxToPtr (idx);
    kkint32 classIndex = classesIndex->GetClassIndex (ic);
    if  ((classIndex < 0)  ||  (classIndex >= (kkint32)numOfClasses))
    {
      KKStr  errMsg = "ModelSvmBase::Predict  ***ERROR***   ";
      errMsg << "Class[" << ic->Name () << "] was asked for but is not defined in this instance of 'ModelSvmBase'.";
      _log.Level (-1) << endl << errMsg << endl << endl;
      _votes         [idx] = 0;
      _probabilities [idx] = 0.0f;
    }
    else
    {
      _votes         [idx] = votes      [classIndex];
      _probabilities [idx] = classProbs [classIndex];
    }
  }

  return;
}  /* ProbabilitiesByClass */





void   ModelSvmBase::ProbabilitiesByClass (FeatureVectorPtr    _example,
                                           const MLClassList&  _mlClasses,
                                           double*             _probabilities,
                                           RunLog&             _log
                                          )
{
  if  (!svmModel)
  {
    KKStr errMsg = "ModelSvmBase::ProbabilitiesByClass   ***ERROR***      (svmModel == NULL)";
    _log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  if  (!classesIndex)
  {
    KKStr errMsg = "ModelSvmBase::ProbabilitiesByClass   ***ERROR***      (classesIndex == NULL)";
    _log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  bool  newExampleCreated = false;
  FeatureVectorPtr  encodedExample = PrepExampleForPrediction (_example, newExampleCreated);

  double  y = SVM289_MFS::svm_predict_probability (svmModel,  *encodedExample, classProbs, votes);

  if  (newExampleCreated)
  {
    delete encodedExample;
    encodedExample = NULL;
  }

  kkuint32  idx;
  for  (idx = 0;  idx < _mlClasses.size ();  idx++)
  {
    MLClassPtr  ic = _mlClasses.IdxToPtr (idx);
    kkint32 classIndex = classesIndex->GetClassIndex (ic);
    if  ((classIndex < 0)  ||  (classIndex >= (kkint32)numOfClasses))
    {
      KKStr  errMsg = "ModelSvmBase::Predict  ***ERROR***   ";
      errMsg << "Class[" << ic->Name () << "] was asked for but is not defined in this instance of 'ModelSvmBase'.";
      _log.Level (-1) << endl << endl << errMsg << endl << endl;
      _probabilities [idx] = 0.0f;
    }
    else
    {
      _probabilities [idx] = classProbs [classIndex];
    }
  }

  return;
}  /* ProbabilitiesByClass */
  



void  ModelSvmBase::RetrieveCrossProbTable (MLClassList&  _classes,
                                            double**      _crossProbTable,  /**< Two dimension matrix that needs to be classes.QueueSize ()  squared. */
                                            RunLog&       _log
                                           )
{
  kkuint32  idx1, idx2;
  VectorInt  pairWiseIndexes (_classes.size (), 0);
  for  (idx1 = 0;  idx1 < _classes.size ();  idx1++)
  {
    MLClassPtr  ic = _classes.IdxToPtr (idx1);
    kkint32 pairWiseIndex = classesIndex->GetClassIndex (ic);
    if  ((pairWiseIndex < 0)  ||  (pairWiseIndex >= (kkint32)numOfClasses))
    {
      KKStr  errMsg = "ModelSvmBase::RetrieveCrossProbTable  ***ERROR***   ";
      errMsg << "Class[" << ic->Name () << "] was asked for but is not defined in this instance of 'ModelSvmBase'.";
      _log.Level (-1) << endl << endl << errMsg << endl << endl;
      pairWiseIndexes[idx1] = 0;
    }
    else
    {
      pairWiseIndexes[idx1] = pairWiseIndex;
    }

    for  (idx2 = 0;  idx2 < _classes.size ();  idx2++)
      _crossProbTable[idx1][idx2] = 0.0;
  }

  double** pairWiseProb = svmModel->PairwiseProb ();
  if  (!pairWiseProb)
    return;

  for  (idx1 = 0;  idx1 < (_classes.size () - 1);  idx1++)
  {
    kkint32 pairWiseIndex1 = pairWiseIndexes [idx1];
    if  ((pairWiseIndex1 >= 0)  &&  (pairWiseIndex1 < (kkint32)numOfClasses))
    {
      for  (idx2 = idx1 + 1;  idx2 < _classes.size ();  idx2++)
      {
        kkint32 pairWiseIndex2 = pairWiseIndexes [idx2];
        if  ((pairWiseIndex2 >= 0)  &&  (pairWiseIndex2 < (kkint32)numOfClasses))
        {
          _crossProbTable[idx1][idx2] = pairWiseProb[pairWiseIndex1][pairWiseIndex2];
          _crossProbTable[idx2][idx1] = pairWiseProb[pairWiseIndex2][pairWiseIndex1];
        }
      }
    }
  }
}  /* RetrieveCrossProbTable */



void  ModelSvmBase::ReadSpecificImplementationXML (istream&  i,
                                                   bool&     _successful,
                                                   RunLog&   log
                                                  )
{
  /**@todo  Make sure that 'param' != NULL */

  if  (param == NULL)
  {
    param = dynamic_cast<ModelParamSvmBasePtr> (Model::param);
  }
  else
  {
    log.Level (20) << "ModelSvmBase::ReadSpecificImplementationXML    param != NULL." << endl;
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

    if  (field.EqualIgnoreCase ("</ModelSvmBase>"))
    {
      break;
    }

    else if  (field.EqualIgnoreCase ("<Model>"))
    {
      Model::ReadXML (i, _successful, log);
    }

    else if  (field.EqualIgnoreCase ("<LibSvmModel>"))
    {
      delete  svmModel;
      svmModel = SVM289_MFS::svm_load_model_XML (i, fileDesc, log);
      if  (!svmModel)
      {
        log.Level (-1) << endl << endl << "ModelSvmBase::ReadSpecificImplementationXML   ***ERROR***  Could not load LibSVM model." << endl << endl;
        validModel = false;
        _successful = false;
      }
    }

    else
    {
      // Add code to deal with items that are specific to 'ModelSvmBase'
    }
  }

  if  (!_successful)
    validModel = false;


  return;
}  /* ReadSpecificImplementationXML */





void  ModelSvmBase::WriteSpecificImplementationXML (ostream&  o,
                                                    RunLog&   log
                                                   )
{
  log.Level (20) << "ModelSvmBase::WriteSpecificImplementationXML  Saving Model in File." << endl;

  o << "<ModelSvmBase>" << endl;

  if  (svmModel)
    svm_save_model_XML (o, *svmModel);


  o << "</ModelSvmBase>" << endl;
} /* WriteSpecificImplementationXML */




kkint32  ModelSvmBase::NumOfSupportVectors ()  const
{
  kkint32  numOfSupportVectors = 0;
  if  (svmModel)
    return  0;

  return  svmModel->numSVs;
}  /* NumOfSupportVectors */





void  ModelSvmBase::WriteXML (const KKStr&  varName,
                              ostream&      o
                             )  const
{
  XmlTag  startTag ("ModelSvmBase",  XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);

  WriteModelXMLFields (o);  // Write the PArent class fields 1st.

  if  (svmModel)
    svmModel->WriteXML ("SvmModel", o);

  //  Turns out the base class "Model" owns this data field and will also be writing it
  // out so no point in re-reading again.
  //if  (param)  param->WriteXML ("Param", o);

  XmlTag  endTag ("ModelSvmBase", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}  /* WriteXML */





void  ModelSvmBase::ReadXML (XmlStream&      s,
                             XmlTagConstPtr  tag,
                             RunLog&         log
                            )
{
  delete  svmModel;
  svmModel = NULL;
  XmlTokenPtr  t = s.GetNextToken (log);
  while  (t)
  {
    t = ReadXMLModelToken (t, log);  // 1st see if the base class has this data field.
    if  (t)
    {
      if  ((t->VarName ().EqualIgnoreCase ("SvmModel"))  &&  (typeid(*t) == typeid(XmlElementSvm_Model)))
      {
        delete  svmModel;
        svmModel = dynamic_cast<XmlElementSvm_ModelPtr> (t)->TakeOwnership ();
      }
      else
      {
        KKStr errMsg (128);
        errMsg << "ModelSvmBase::ReadXML  ***ERROR***  Unexpected Token;  Section:" << t->SectionName () << "  VarName: " << t->VarName ();
        AddErrorMsg (errMsg, 0);
        log.Level (-1) << endl << errMsg << endl << endl;
      }
    }

    delete  t;
    t = s.GetNextToken (log);
  }

  if  (Model::param == NULL)
  {
    KKStr errMsg (128);
    errMsg << "ModelSvmBase::ReadXML  ***ERROR***  Base class 'Model' does not have 'param' defined.";
    AddErrorMsg (errMsg, 0);
    log.Level (-1) << endl << errMsg << endl << endl;
  }

  else if  (typeid (*Model::param) != typeid(ModelParamSvmBase))
  {
    KKStr errMsg (128);
    errMsg << "ModelSvmBase::ReadXML  ***ERROR***  Base class 'Model' param parameter is of the wrong type;  found: " << param->ModelParamTypeStr ();
    AddErrorMsg (errMsg, 0);
    log.Level (-1) << endl << errMsg << endl << endl;
  }

  else
  {
    param = dynamic_cast<ModelParamSvmBasePtr> (Model::param);
  }
}  /* ReadXML */

 

XmlFactoryMacro(ModelSvmBase)
