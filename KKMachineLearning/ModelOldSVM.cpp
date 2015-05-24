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
using namespace  std;


#include "GlobalGoalKeeper.h"
#include "KKBaseTypes.h"
#include "KKException.h"
#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace  KKB;


#include "ModelOldSVM.h"
#include "BinaryClassParms.h"
#include "ClassAssignments.h"
#include "ClassProb.h"
#include "FeatureEncoder2.h"
#include "FeatureNumList.h"
#include "FeatureVector.h"
#include "KKMLLTypes.h"
#include "ModelParamOldSVM.h"
#include "NormalizationParms.h"
#include "SVMparam.h"
using namespace KKMLL;



ModelOldSVM::ModelOldSVM ():
  Model (),
  assignments (NULL),
  svmModel    (NULL)
{
  Model::param = new ModelParamOldSVM ();
}




ModelOldSVM::ModelOldSVM (FileDescPtr  _fileDesc):
  Model (_fileDesc),
  assignments (NULL),
  svmModel    (NULL)
{
  Model::param = new ModelParamOldSVM ();
}



ModelOldSVM::ModelOldSVM (const KKStr&            _name,
                          const ModelParamOldSVM& _param,         // Create new model from
                          FileDescPtr             _fileDesc                         )
:
  Model   (_name, _param, _fileDesc),
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


KKStr  ModelOldSVM::Description ()  const
{
  KKStr  result = "SVM(" + Name () + ")";

  if  (svmModel)
  {
    SVMparam const *  p = svmModel->SVMParameters ();
    result << " " << MachineTypeToStr (p->MachineType ())
           << " " << SelectionMethodToStr (p->SelectionMethod ());
  }
  return  result;
}



const ClassAssignments&  ModelOldSVM::Assignments ()  const
{
  return svmModel->Assignments ();
}



FeatureNumListConstPtr  ModelOldSVM::GetFeatureNums (FileDescPtr filedesc,
                                                     MLClassPtr  class1,
                                                     MLClassPtr  class2
                                                    )
{
  return svmModel->GetFeatureNums (filedesc, class1, class2);
}  /* GetFeatureNums */




FeatureNumListConstPtr   ModelOldSVM::GetFeatureNums ()  const
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
    cerr << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  if  (param->ModelParamType () != ModelParam::ModelParamTypes::mptOldSVM)
  {
    KKStr errMsg = "ModelOldSVM::Param   ***ERROR***  param variable of wrong type.";
    cerr << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  return  dynamic_cast<ModelParamOldSVMPtr> (param);
}



SVM_SelectionMethod  ModelOldSVM::SelectionMethod () const
{
  return  svmModel->SelectionMethod ();
}




void   ModelOldSVM::Predict (FeatureVectorPtr  example,
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
  bool  newExampleCreated = false;
  FeatureVectorPtr  encodedExample = PrepExampleForPrediction (example, newExampleCreated);
  svmModel->Predict (encodedExample, knownClass, predClass1, predClass2,
                     predClass1Votes,  predClass2Votes,
                     probOfKnownClass, 
                     predClass1Prob,    predClass2Prob,
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





MLClassPtr  ModelOldSVM::Predict (FeatureVectorPtr  example,
                                  RunLog&           log
                                 )
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






void  ModelOldSVM::PredictRaw (FeatureVectorPtr  example,
                               MLClassPtr     &  predClass,
                               double&           dist
                              )
{
  svmModel->PredictRaw (example, predClass, dist);
}  /* PredictRaw */





ClassProbListPtr  ModelOldSVM::ProbabilitiesByClass (FeatureVectorPtr  example,
                                                     RunLog&           log
                                                    )
{
  if  (!svmModel)
  {
    KKStr  errMsg = "ModelOldSVM::ProbabilitiesByClass   ***ERROR***      (svmModel == NULL)";
    log.Level (-1) << endl << errMsg << endl;
    throw KKException (errMsg);
  }

  bool  newExampleCreated = false;
  FeatureVectorPtr  encodedExample = PrepExampleForPrediction (example, newExampleCreated);
  svmModel->ProbabilitiesByClass (encodedExample, *classes, votes, classProbs, log);

  if  (newExampleCreated)
  {
    delete encodedExample;
    encodedExample = NULL;
  }
  
  ClassProbListPtr  results = new ClassProbList ();
  kkint32 idx = 0;
  for  (idx = 0;  idx < numOfClasses;  idx++)
  {
    MLClassPtr  ic = classes->IdxToPtr (idx);
    results->PushOnBack (new ClassProb (ic, classProbs[idx], (float)votes[idx]));
  }

  if  (svmModel->SVMParameters ()->SelectionMethod () == SVM_SelectionMethod::SelectByVoting)
    results->SortByVotes (true);
  else
    results->SortByProbability (true);

  return  results;
}  /* ProbabilitiesByClass */






void  ModelOldSVM::ProbabilitiesByClass (FeatureVectorPtr    _example,
                                         const MLClassList&  _mlClasses,
                                         double*             _probabilities,
                                         RunLog&             _log
                                        )
{
  bool  newExampleCreated = false;
  FeatureVectorPtr  encodedExample = PrepExampleForPrediction (_example, newExampleCreated);

  svmModel->ProbabilitiesByClass (encodedExample, _mlClasses, votes, _probabilities, _log);
  if  (newExampleCreated)
  {
    delete encodedExample;
    encodedExample = NULL;
  }

  return;
}




void  ModelOldSVM::ProbabilitiesByClass (FeatureVectorPtr    example,
                                         const MLClassList&  _mlClasses,
                                         kkint32*            _votes,
                                         double*             _probabilities,
                                         RunLog&             _log
                                        )
{
  bool  newExampleCreated = false;
  FeatureVectorPtr  encodedExample = PrepExampleForPrediction (example, newExampleCreated);
  svmModel->ProbabilitiesByClass (encodedExample, _mlClasses, _votes, _probabilities, _log);
  if  (newExampleCreated)
  {
    delete encodedExample;
    encodedExample = NULL;
  }

  return;
}  /* ProbabilitiesByClass */







vector<KKStr>  ModelOldSVM::SupportVectorNames (MLClassPtr  c1,
                                                MLClassPtr  c2
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
                                                            MLClassPtr        c1,
                                                            MLClassPtr        c2
                                                           )
{
  return  svmModel->FindWorstSupportVectors (example, numToFind, c1, c2);
}  /* FindWorstSupportVectors */






vector<ProbNamePair>  ModelOldSVM::FindWorstSupportVectors2 (FeatureVectorPtr  example,
                                                             kkint32           numToFind,
                                                             MLClassPtr        c1,
                                                             MLClassPtr        c2
                                                            )
{
  return  svmModel->FindWorstSupportVectors2 (example, numToFind, c1, c2);
}  /* FindWorstSupportVectors2 */





bool  ModelOldSVM::NormalizeNominalAttributes ()  const
{
  return  svmModel->NormalizeNominalAttributes ();
}  /* NormalizeNominalAttributes */




void  ModelOldSVM::RetrieveCrossProbTable (MLClassList&   classes,
                                           double**       crossProbTable,  /**< two dimension matrix that needs to be classes.QueueSize ()  squared. */
                                           RunLog&        log
                                          )
{
  svmModel->RetrieveCrossProbTable (classes, crossProbTable, log);
  return;
}  /* RetrieveCrossProbTable */




void  ModelOldSVM::TrainModel (FeatureVectorListPtr  _trainExamples,
                               bool                  _alreadyNormalized,
                               bool                  _takeOwnership,  /*!< Model will take ownership of these examples */
                               VolConstBool&         _cancelFlag,
                               RunLog&               _log
                              )
{
  _log.Level (20) << "ModelOldSVM::TrainModel - Constructing From Training Data, Model[" << rootFileName << "]" << endl;
  // We do not bother with the base class 'TrainModel' like we do with other models.
  // 'ModelOldSVM' is a special case.  All we are trying to do is create a pass through
  // for 'svmModel'.

  delete  svmModel;
  svmModel = NULL;

  Model::TrainModel (_trainExamples, _alreadyNormalized, _takeOwnership, _cancelFlag, _log);
  // The "Model::TrainModel" may have manipulated the '_trainExamples'.  It will have also 
  // updated 'Model::trainExamples.  So from this point forward we use 'trainExamples'.
  _trainExamples = NULL;

  if  ((!fileDesc)  &&  trainExamples)
    fileDesc = trainExamples->FileDesc ();

  SVMparamPtr svmParam = Param ()->SvmParameters ();
  if  (!svmParam)
  {
    validModel = false;
    KKStr errMsg = " ModelOldSVM::TrainModel  ***ERROR***    (svmParam == NULL).";
    _log.Level (-1) << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  {
    delete  classes;
    classes = trainExamples->ExtractListOfClasses ();
    classes->SortByName ();
    numOfClasses = classes->QueueSize ();
    delete  assignments;
    assignments = new ClassAssignments (*classes);
  }

  try
  {
    TrainingTimeStart ();
    svmModel = new SVMModel (*svmParam, *trainExamples, *assignments, fileDesc, _log);
    TrainingTimeEnd ();
  }
  catch (...)
  {
    _log.Level (-1) << endl << "ModelOldSVM::TrainModel  Exception occurred building training model." << endl << endl;
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






void  ModelOldSVM::WriteXML (const KKStr&  varName,
                             ostream&      o
                            )  const
{
  XmlTag  startTag ("ModelOldSVM",  XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);
  o << endl;

  WriteModelXMLFields (o);  // Write the PArent class fields 1st.

  if  (assignments)
    assignments->ToString ().WriteXML ("Assignments", o);

  if  (svmModel)
    svmModel->WriteXML ("svmModel", o);

  XmlTag  endTag ("ModelOldSVM", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}  /* WriteXML */





void  ModelOldSVM::ReadXML (XmlStream&      s,
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
      if  ((t->VarName ().EqualIgnoreCase ("Assignments"))  &&  (typeid(*t) == typeid(XmlElementKKStr)))
      {
        XmlElementKKStrPtr s = dynamic_cast<XmlElementKKStrPtr> (t);
        delete  assignments;
        assignments = new ClassAssignments ();
        assignments->ParseToString (*(s->Value ()), log);
      }

      else if  ((t->VarName ().EqualIgnoreCase ("SvmModel"))  &&  (typeid(*t) == typeid(XmlElementSVMModel)))
      {
        delete  svmModel;
        svmModel = dynamic_cast<XmlElementSVMModelPtr> (t)->TakeOwnership ();
      }

      else
      {
        KKStr errMsg (128);
        errMsg << "ModelOldSVM::ReadXML  ***ERROR***  Unexpected Token;  Section:" << t->SectionName () << "  VarName: " << t->VarName ();
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
    errMsg << "ModelOldSVM::ReadXML  ***ERROR***  Base class 'Model' does not have 'param' defined.";
    AddErrorMsg (errMsg, 0);
    log.Level (-1) << endl << errMsg << endl << endl;
  }

  else if  (typeid (*Model::param) != typeid(ModelParamOldSVM))
  {
    KKStr errMsg (128);
    errMsg << "ModelOldSVM::ReadXML  ***ERROR***  Base class 'Model' param parameter is of the wrong type;  found: " << param->ModelParamTypeStr ();
    AddErrorMsg (errMsg, 0);
    log.Level (-1) << endl << errMsg << endl << endl;
  }

  if  ((!svmModel)  ||  (!svmModel->ValidModel ()))
  {
    KKStr errMsg (128);
    errMsg << "ModelOldSVM::ReadXML  ***ERROR***  'SvmModel' was not defined or is not valid.";
    AddErrorMsg (errMsg, 0);
    log.Level (-1) << endl << errMsg << endl << endl;
  }

  else
  {
    param = dynamic_cast<ModelParamOldSVMPtr> (Model::param);
  }

  ReadXMLModelPost (log);
}  /* ReadXML */




class  XmlFactoryModelOldSVM: public XmlFactory                           
{                                                                         
public:                                                                   
  XmlFactoryModelOldSVM (): XmlFactory ("ModelOldSVM") {}                  
                                                                          
  virtual  XmlElementModelOldSVM*  ManufatureXmlElement (XmlTagPtr   tag, 
                                                         XmlStream&  s, 
                                                         RunLog&     log 
                                                        )                
  {                                                                        
    return new XmlElementModelOldSVM(tag, s, log);                         
  }                                                                        
                                                                           
  static   XmlFactoryModelOldSVM*   factoryInstance;                       
                                                                           
  static   XmlFactoryModelOldSVM*   FactoryInstance ()                     
  {                                                                        
    if  (factoryInstance == NULL)                                          
    {                                                                      
      GlobalGoalKeeper::StartBlock ();                                     
      if  (!factoryInstance)                                               
      {                                                                    
        factoryInstance = new XmlFactoryModelOldSVM ();                    
        XmlFactory::RegisterFactory (factoryInstance);                     
      }                                                                    
      GlobalGoalKeeper::EndBlock ();                                       
     }                                                                     
    return  factoryInstance;                                               
  }                                                                        
};                                                                         
                                                                           
XmlFactoryModelOldSVM*   XmlFactoryModelOldSVM::factoryInstance 
              = XmlFactoryModelOldSVM::FactoryInstance ();


