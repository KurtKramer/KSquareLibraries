#include "FirstIncludes.h"

#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <set>
#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>
#include "MemoryDebug.h"
using namespace  std;


#include "KKBaseTypes.h"
#include "KKException.h"
#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace  KKB;


#include "ClassProb.h"
#include "FactoryFVProducer.h"
#include "FeatureEncoder2.h"
#include "FeatureNumList.h"
#include "FeatureVector.h"
#include "Model.h"
#include "ModelOldSVM.h"
#include "ModelSvmBase.h"
#include "ModelKnn.h"
#include "ModelUsfCasCor.h"
#include "ModelDual.h"
#include "ModelParam.h"
#include "ModelParamDual.h"
#include "ModelParamOldSVM.h"
#include "NormalizationParms.h"
using namespace  KKMLL;


Model::Model ():
    alreadyNormalized        (false),
    classes                  (NULL),
    classesIndex             (NULL),
    classProbs               (NULL),
    crossClassProbTable      (NULL),
    crossClassProbTableSize  (0),
    encoder                  (NULL),
    factoryFVProducer        (NULL),
    fileDesc                 (NULL),
    name                     (),
    normParms                (NULL),
    numOfClasses             (0),
    param                    (NULL),
    rootFileName             (),
    trainExamples            (NULL),
    trianingPrepTime         (0.0),
    trainingTime             (0.0),
    trainingTimeStart        (0.0),
    validModel               (true),
    votes                    (NULL),
    weOwnTrainExamples       (false)
{
}


Model::Model (const Model&  _model):
    alreadyNormalized       (false),
    classes                 (NULL),
    classesIndex            (NULL),
    classProbs              (NULL),
    crossClassProbTable     (NULL),
    crossClassProbTableSize (0),
    encoder                 (NULL),
    factoryFVProducer       (_model.factoryFVProducer),
    fileDesc                (_model.fileDesc),
    name                    (_model.name),
    normParms               (NULL),
    numOfClasses            (_model.numOfClasses),
    param                   (NULL),
    rootFileName            (_model.rootFileName),
    trainExamples           (NULL),
    trianingPrepTime        (0.0),
    trainingTime            (_model.trainingTime),
    trainingTimeStart       (_model.trainingTime),
    validModel              (_model.validModel),
    votes                   (NULL),
    weOwnTrainExamples      (false)
{
  numOfClasses = 0;
  if  (_model.param != NULL)
    param = _model.param->Duplicate ();

  if  (_model.classes)
  {
    classes = new MLClassList (*_model.classes);
    numOfClasses = classes->QueueSize ();

    AllocatePredictionVariables ();
  }

  if  (_model.classesIndex)
    classesIndex = new MLClassIndexList (*_model.classesIndex);


  if  (_model.encoder)
    encoder = new FeatureEncoder2 (*_model.encoder);
}


    
/**
 *@brief  Use this when you are planning on creating a empty model without parameters.
 */
Model::Model (FactoryFVProducerPtr  _factoryFVProducer):
    alreadyNormalized        (false),
    classes                  (NULL),
    classesIndex             (NULL),
    classProbs               (NULL),
    crossClassProbTable      (NULL),
    crossClassProbTableSize  (0),
    encoder                  (NULL),
    factoryFVProducer        (_factoryFVProducer),
    fileDesc                 (NULL),
    name                     (),
    normParms                (NULL),
    numOfClasses             (0),
    param                    (NULL),
    rootFileName             (),
    trainExamples            (NULL),
    trianingPrepTime         (0.0),
    trainingTime             (0.0),
    trainingTimeStart        (0.0),
    validModel               (true),
    votes                    (NULL),
    weOwnTrainExamples       (false)
{
  fileDesc = factoryFVProducer->FileDesc ();
}

    
    
/**
 *@brief Creates a new svm model from the provided example data
 *@param[in] _name
 *@param[in] _param Will make own local copy.
 *@param[in] _fileDesc A description of the data file.
 *@param[in] _log A log-file stream. All important events will be output to this stream
 */
Model::Model (const KKStr&          _name,
              const ModelParam&     _param,      // Create new model from
              FactoryFVProducerPtr  _factoryFVProducer
             ):
    alreadyNormalized        (false),
    classes                  (NULL),
    classesIndex             (NULL),
    classProbs               (NULL),
    crossClassProbTable      (NULL),
    crossClassProbTableSize  (0),
    encoder                  (NULL),
    factoryFVProducer        (_factoryFVProducer),
    fileDesc                 (NULL),
    normParms                (NULL),
    numOfClasses             (0),
    param                    (NULL),
    rootFileName             (),
    trainExamples            (NULL),
    validModel               (true),
    votes                    (NULL),
    weOwnTrainExamples       (false),

    trianingPrepTime         (0.0),
    trainingTime             (0.0),
    trainingTimeStart        (0.0),
    name                     (_name),
	timeSaved                ()
{
  fileDesc = factoryFVProducer->FileDesc ();
  param = _param.Duplicate ();
}




/**
 * @brief Frees any memory allocated by, and owned by the Model
 */
Model::~Model ()
{
  DeAllocateSpace ();

  delete  classesIndex;  classesIndex = NULL;
  delete  classes;       classes = NULL;
  delete  encoder;       encoder = NULL;
  delete  normParms;     normParms = NULL;

  if  (weOwnTrainExamples)
  {
    delete  trainExamples;
    trainExamples = NULL;
  }

  delete  param;
  param = NULL;
}


kkMemSize  Model::MemoryConsumedEstimated ()  const
{
  kkMemSize  memoryConsumedEstimated = sizeof (Model) + rootFileName.MemoryConsumedEstimated ();
  if  (classes)              memoryConsumedEstimated += classes->MemoryConsumedEstimated ();
  if  (classesIndex)         memoryConsumedEstimated += classesIndex->MemoryConsumedEstimated ();
  if  (classProbs)           memoryConsumedEstimated += numOfClasses * sizeof (double);
  if  (crossClassProbTable)  memoryConsumedEstimated += (numOfClasses * sizeof (double*)  +  numOfClasses * numOfClasses * sizeof (double));
  if  (encoder)              memoryConsumedEstimated += encoder->MemoryConsumedEstimated ();
  if  (normParms)            memoryConsumedEstimated += normParms->MemoryConsumedEstimated ();
  if  (param)                memoryConsumedEstimated += param->MemoryConsumedEstimated ();
  if  (votes)                memoryConsumedEstimated += numOfClasses * sizeof (kkint32);

  if  (weOwnTrainExamples  &&  (trainExamples != NULL))
    memoryConsumedEstimated += trainExamples->MemoryConsumedEstimated ();

  return  memoryConsumedEstimated;
}



MLClassListPtr  Model::MLClassesNewInstance () const
{
  if  (classes)
    return new MLClassList (*classes);
  else
    return NULL;
}  /* MLClassesNewInstance */




void  Model::AddErrorMsg (const KKStr&  errMsg,
                          kkint32       lineNum
                         )
{
  errors.push_back (errMsg);
}



KKStr  Model::Description ()  const
{
  return ModelTypeStr () + "(" + Name () + ")";
}




KKStr  Model::ModelTypeToStr (ModelTypes  _modelingType)
{
  if       (_modelingType == ModelTypes::Null)      return "NULL";
  else if  (_modelingType == ModelTypes::OldSVM)    return "OldSVM";
  else if  (_modelingType == ModelTypes::SvmBase)   return "SvmBase";
  else if  (_modelingType == ModelTypes::KNN)       return "KNN";
  else if  (_modelingType == ModelTypes::UsfCasCor) return "UsfCasCor";
  else if  (_modelingType == ModelTypes::Dual)      return "Dual";
  else
    return "NULL";
}  /* ModelingMethodToStr */




Model::ModelTypes  Model::ModelTypeFromStr (const KKStr&  _modelingTypeStr)
{
  if       (_modelingTypeStr.EqualIgnoreCase ("OldSVM")  ||  
            _modelingTypeStr.EqualIgnoreCase ("One_Level"))   return ModelTypes::OldSVM;
  else if  (_modelingTypeStr.EqualIgnoreCase ("SvmBase"))     return ModelTypes::SvmBase;
  else if  (_modelingTypeStr.EqualIgnoreCase ("KNN"))         return ModelTypes::KNN;
  else if  (_modelingTypeStr.EqualIgnoreCase ("UsfCasCor"))   return ModelTypes::UsfCasCor;
  else if  (_modelingTypeStr.EqualIgnoreCase ("Dual"))        return ModelTypes::Dual;

  else
    return ModelTypes::Null;
}  /* ModelingMethodFromStr */




ModelPtr  Model::CreateAModel (ModelTypes            _modelType,
                               const KKStr&          _name,
                               const ModelParam&     _param,  
                               FactoryFVProducerPtr  _factoryFVProducer,
                               VolConstBool&         _cancelFlag,
                               RunLog&               _log
                              )
{
  ModelPtr  model = NULL;
  try
  {
    switch  (_modelType)
    {
    case  ModelTypes::OldSVM:    
          model = new ModelOldSVM    (_name, dynamic_cast<const ModelParamOldSVM&>    (_param), _factoryFVProducer);
          break;

    case  ModelTypes::SvmBase:
          model = new ModelSvmBase   (_name, dynamic_cast<const ModelParamSvmBase&>   (_param), _factoryFVProducer);
          break;

    case  ModelTypes::KNN:
          model = new ModelKnn       (_name, dynamic_cast<const ModelParamKnn&>       (_param), _factoryFVProducer);
          break;

    case  ModelTypes::UsfCasCor:
          model = new ModelUsfCasCor (_name, dynamic_cast<const ModelParamUsfCasCor&> (_param), _factoryFVProducer);
          break;

    case  ModelTypes::Dual:
          model = new ModelDual      (_name, dynamic_cast<const ModelParamDual&>      (_param), _factoryFVProducer);
          break;

    default:
          KKStr errMsg = "Model::CreateAModel   ***ERROR***  Invalid _modelType[" + KKB::StrFromInt16((kkint16)_modelType) + "].";
          _log.Level (-1) << endl << errMsg << endl << endl;
          throw KKException(errMsg);
    }  /* end of switch */
  }
  catch  (const KKException&  e)
  {
    delete  model; model = NULL;
    throw  KKException ("Model::CreateAModel  Exception calling constructor.", e);
  }
  catch  (...)
  {
    delete  model;  model = NULL;
    throw  KKException ("Model::CreateAModel  Exception calling constructor.  No info provided.");
  }

  return  model;
}  /*  CreateAModel */




void  Model::AllocatePredictionVariables ()
{
  kkint32 x = 0;
  kkint32 y = 0;

  DeAllocateSpace ();

  if  (classes == NULL)
  {
    cerr << endl << endl 
      << "Model::AllocatePredictionVariables   ***ERROR***      (classes == NULL)"  << endl
      << endl;

    validModel = false;
    return;
  }

  numOfClasses = classes->QueueSize ();

  if  (numOfClasses > 0)
  {
    crossClassProbTableSize = numOfClasses;
    classProbs    = new double[numOfClasses];
    votes         = new kkint32 [numOfClasses];
    crossClassProbTable = new double*[numOfClasses];
    for  (x = 0;  x < numOfClasses;  x++)
    {
      classProbs         [x] = 0.0;
      votes              [x] = 0;
      crossClassProbTable[x] = new double[numOfClasses];
      for  (y = 0;  y < numOfClasses;  y++)
        crossClassProbTable[x][y] = 0.0f;
    }
  }
}  /* AllocatePredictionVariables*/





void  Model::DeAllocateSpace ()
{
  kkint32 x = 0;
  if  (crossClassProbTable)
  {
    for  (x = 0;  x < numOfClasses;  x++)
    {
      delete  [] crossClassProbTable[x];
      crossClassProbTable[x] = NULL;
    }
    delete[]  crossClassProbTable;
    crossClassProbTable = NULL;
  }

  delete[] classProbs;
  classProbs = NULL;

  delete[] votes;
  votes = NULL;
}



const FeatureEncoder2&  Model::Encoder () const
{
  if  (!encoder)
    throw KKException ("Model::GetFeatureNums  'encoder == NULL'.");
  return *encoder;
}



FeatureNumListConstPtr   Model::GetFeatureNums ()  const
{
  if  (!param)
    throw KKException ("Model::GetFeatureNums  'param == NULL'.");
  return  param->SelectedFeatures ();
}


bool  Model::NormalizeNominalAttributes ()  const
{
  if  (!param)
    throw KKException ("Model::NormalizeNominalAttributes  'param == NULL'.");

  if  (param->EncodingMethod () == ModelParam::EncodingMethodType::NoEncoding)
    return  true;
  else
    return  false;
}  /* NormalizeNominalAttributes */



FeatureNumListConstPtr  Model::SelectedFeatures () const
{
  if  (!param)
    throw KKException ("Model::GetFeatureNums  'param'.");
  return  param->SelectedFeatures ();
}



void  Model::TrainingTimeStart ()
{
  trainingTimeStart = osGetSystemTimeUsed ();
}



void  Model::TrainingTimeEnd ()
{
  double  trainingTimeEnd = osGetSystemTimeUsed ();
  trainingTime = trainingTimeEnd - trainingTimeStart;
  if  (trainingTime < 0.0)
    trainingTime += (24.0 * 60.0 * 60.0);
}




/**
 *@brief Performs operations such as FeatureEncoding, and  Normalization; the actual training 
 *       of models occurs in the specific implementation of 'Model'.
 */
void  Model::TrainModel (FeatureVectorListPtr  _trainExamples,
                         bool                  _alreadyNormalized,
                         bool                  _takeOwnership,  /*!< True = Model will take ownership of these examples */
                         VolConstBool&         _cancelFlag,
                         RunLog&               _log
                        )
{
  _log.Level (10) << "Model::TrainModel   Preparing for training of Model[" << Name () << "]  Examples[" << _trainExamples->QueueSize () << "]" << endl;

  double  prepStartTime = osGetSystemTimeUsed ();

  if  (_trainExamples == NULL)
  {
    KKStr  errMsg = "ModelSvmBase::TrainModel   (_trainExamples == NULL)";
    _log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  if  (_trainExamples->QueueSize () < 2)
  {
    KKStr  errMsg = "ModelSvmBase::TrainModel   (_trainExamples == NULL)";
    _log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  weOwnTrainExamples = _takeOwnership;
  trainExamples      = _trainExamples;
  alreadyNormalized  = _alreadyNormalized;
  _trainExamples     = NULL;

  delete  classes;
  classes = trainExamples->ExtractListOfClasses ();
  classes->SortByName ();

  delete  classesIndex;
  classesIndex = new MLClassIndexList (*classes);

  if  (param->ExamplesPerClass () < int32_max)
    ReduceTrainExamples (_log);

  if  (!_alreadyNormalized)
  {
    if  (!weOwnTrainExamples)
    {
      // Since we do not own the WE will have to duplicate the trainExamples list and its contents before we normalize the data.
      FeatureVectorListPtr  temp = trainExamples->Duplicate (true);
      weOwnTrainExamples = true;
      trainExamples = temp;
    }
    else if  (!trainExamples->Owner ())
    {
      // Even though we own 'trainExamples' we do not own its contents; so we will need to create a new list and own those contents.
      FeatureVectorListPtr  temp = trainExamples->Duplicate (true);
      weOwnTrainExamples = true;
      delete  trainExamples;
      trainExamples = temp;
    }
    delete  normParms;
    normParms = new NormalizationParms (*param, *trainExamples, _log);
    normParms->NormalizeExamples (trainExamples, _log);
  }

  if  (param->EncodingMethod () == ModelParam::EncodingMethodType::Null)
  {
    // There is nothing for us to do.
    return;
  }

  if  ((param->EncodingMethod () != ModelParam::EncodingMethodType::Null)  &&   (param->EncodingMethod () != ModelParam::EncodingMethodType::NoEncoding))
  {
    if  (!encoder)
      encoder = new FeatureEncoder2 (*param, fileDesc);

    FeatureVectorListPtr  encodedImages = encoder->EncodeAllExamples (trainExamples);
    if  (weOwnTrainExamples)
    {
      delete  trainExamples;
      trainExamples = NULL;
    }

    trainExamples = encodedImages;
    weOwnTrainExamples = true;
  }

  AllocatePredictionVariables ();

  double  prepEndTime = osGetSystemTimeUsed ();
  trianingPrepTime = prepEndTime - prepStartTime;
  if  (trianingPrepTime < 0.0)
    trianingPrepTime += (24.0 * 60.0 * 60.0);

  _log.Level (40) << "Model::TrainModel   Exiting." << endl;
}  /* TrainModel */





/**
 *@brief  Every prediction  method in every class that is inherited from this class should call
 *        this method before performing there prediction. Such things as Normalization and
 *        Feature Encoding.
 *@param[in]  fv  Feature vector of example that needs to be prepared.
 *@param[out]  newExampleCreated  Indicates if either Feature Encoding and/or Normalization needed
 *             to be done. If neither then the original instance is returned. If Yes then
 *             a new instance which the caller will have to be delete will be returned.
 */
FeatureVectorPtr  Model::PrepExampleForPrediction (FeatureVectorPtr  fv,
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

  // I do not believe we need the encoder at this point. At least not for the Features Selected part. Maybe the conversion from minimal fields will make sense.
  if  (encoder)
  {
    oldFV = fv;
    fv = encoder->EncodeAExample (fv);
    if  (newExampleCreated)
    {
      delete  oldFV;
      oldFV = NULL;
    }
    newExampleCreated = true;
  }

  return  fv;
}  /* PrepExampleForPrediction */




/**
 * Will normalize probabilities such that the sum of all equal 1.0 and no one probability will be less than 'minProbability'.
 */
void  Model::NormalizeProbabilitiesWithAMinumum (kkint32  numClasses,
                                                 double*  probabilities,
                                                 double   minProbability
                                                )
{
  double  sumGreaterOrEqualMin = 0.0;
  kkint32 numLessThanMin = 0;

  kkint32 x = 0;
  for  (x = 0;  x < numClasses;  ++x)
  {
    if  (probabilities[x] < minProbability)
      ++numLessThanMin;
    else
      sumGreaterOrEqualMin += probabilities[x];
  }

  double probLessMinTotal = numLessThanMin * minProbability;
  double probLeftToAllocate  = 1.0 - probLessMinTotal;  

  for  (x = 0;  x < numClasses;  ++x)
  {
    if  (probabilities[x] < minProbability)
      probabilities[x] = minProbability;
    else
      probabilities[x] = (probabilities[x] / sumGreaterOrEqualMin) * probLeftToAllocate;
  }
}  /* NormalizeProbabilitiesWithAMinumum */




/**
 @brief  Reduces the Training Images down to the size dictated by the 'examplesPerClass' parameter.
 */
void  Model::ReduceTrainExamples (RunLog&  log)
{
  kkint32  examplesPerClass = param->ExamplesPerClass ();
  kkuint32  zed = 0;

  if  (examplesPerClass < 0)
    examplesPerClass = int32_max;

  bool  reductionNeeded = false;

  {
    // First lets see if reduction is even necessary.
    ClassStatisticListPtr  stats = trainExamples->GetClassStatistics ();
    if  (!stats)
    {
      log.Level (-1) << endl
        << "Model::ReduceTrainExamples   ***ERROR***  can not retrieve Class Stat's for training data." << endl
        << endl;
      validModel = false;
      return;
    }

    for  (zed = 0;  (zed < stats->size ())  &&  (!reductionNeeded);  zed++)
    {
      if  (stats->IdxToPtr (zed)->Count () > (kkuint32)examplesPerClass)
        reductionNeeded  = true;
    }

    delete  stats;
    stats = NULL;
  }

  if  (!reductionNeeded)
  {
    log.Level (20) << "Model::ReduceTrainExamples    Was not needed.  No classes exceeded 'examplesPerClass'." << endl;
    return;
  }

  FeatureVectorListPtr  reducedSet = trainExamples->ManufactureEmptyList (false);
  FeatureVectorListPtr  deleteSet  = trainExamples->ManufactureEmptyList (false);  // Examples that we do not use will need to be deleted.
  MLClassList::iterator  idx;

  for  (idx = classes->begin ();  idx != classes->end ();  idx++)
  {
    MLClassPtr  ic = *idx;
    FeatureVectorListPtr  examplesThisClass = trainExamples->ExtractExamplesForAGivenClass (ic);
    if  ((!examplesThisClass)  ||  (examplesThisClass->size () < 1))
    {
      log.Level (-1) << endl
        << "Model::ReduceTrainExamples   ***ERROR***   No Training Examples for class[" << ic->Name () << "]." << endl
        << endl;
      continue;
    }

    if  (examplesThisClass->size () <= (kkuint32)examplesPerClass)
    {
      reducedSet->AddQueue (*examplesThisClass);
    }
    else
    {
      examplesThisClass->RandomizeOrder ();
      zed = 0;
      while  (zed < (kkuint32)examplesPerClass)
      {
        reducedSet->PushOnBack (examplesThisClass->IdxToPtr (zed));
        zed++;
      }
      while  (zed < examplesThisClass->size ())
      {
        deleteSet->PushOnBack (examplesThisClass->IdxToPtr (zed));
        zed++;
      }
    }

    delete  examplesThisClass;
    examplesThisClass = NULL;
  }

  if  (weOwnTrainExamples)
  {
    trainExamples->Owner (false);
    delete trainExamples;
    trainExamples = reducedSet;
    reducedSet = NULL;

    trainExamples->Owner (true);
    deleteSet->Owner (true);
    delete  deleteSet;
    deleteSet = NULL;
  }
  else
  {
    // Since we are replacing 'trainExamples' with 'reducedSet' we will now own 'trainExamples' but not its contents.
    reducedSet->Owner (false);
    weOwnTrainExamples = true;
    trainExamples = reducedSet;
    reducedSet = NULL;
    deleteSet->Owner (false);
    delete  deleteSet;
    deleteSet = NULL;
  }
}  /* ReduceTrainExamples */




void  Model::RetrieveCrossProbTable (MLClassList&   classes,
                                     double**       crossClassProbTable,  /**< two dimension matrix that needs to be classes.QueueSize ()  squared. */
                                     RunLog&        log
                                    )
{
  if  (classes.QueueSize () != crossClassProbTableSize)
  {
    // There Class List does not have the same number of entries as our 'CrossProbTable'
    log.Level (-1) << endl
                   << "SVMModel::RetrieveCrossProbTable   ***ERROR***" << endl
                   << "            classes.QueueSize ()[" << classes.QueueSize () << "] != crossClassProbTableSize[" << crossClassProbTableSize << "]" << endl
                   << endl;
    return;
  }

  kkint32*  indexTable = new kkint32[classes.QueueSize ()];
  kkint32  x, y;
  for  (x = 0;  x < classes.QueueSize ();  x++)
  {
    for  (y = 0;  y < classes.QueueSize ();  y++)
       crossClassProbTable[x][y] = 0.0;

    indexTable[x] = classesIndex->GetClassIndex (classes.IdxToPtr (x));
    if  (indexTable[x] < 0)
    {
      log.Level (-1) << endl
                     << "SVMModel::RetrieveCrossProbTable   ***WARNING***" << endl
                     << endl
                     << "      Class Index[" << x << "]  Name[" << classes[x].Name () << "]" << endl
                     << "      will populate this index with zeros."                         << endl
                     << endl;
    }
  }

  if  (classes.QueueSize () != crossClassProbTableSize)
  {
    log.Level (-1) << endl
                   << "SVMModel::RetrieveCrossProbTable   ***ERROR***"                                       << endl
                   << "                                  'classes.QueueSize () != crossClassProbTableSize'"  << endl
                   << endl;
    return;
  }


  // x,y         = 'Callers'   Class Indexes..
  // xIdx, yIdx  = 'SVMNodel'  Class Indexed.
  for  (x = 0;  x < classes.QueueSize ();  x++)
  {
    kkint32 xIdx = indexTable[x];
    if  (xIdx >= 0)
    {
      for  (y = 0;  y < classes.QueueSize ();  y++)
      {
        kkint32  yIdx = indexTable[y];
        if  (yIdx >= 0)
        {
          if  ((x != xIdx)  ||  (y != yIdx))
          {
            //kak  I just added this check to see when this situation actually occurs.
            kkint32 zed = 111;
          }

          crossClassProbTable[x][y] = this->crossClassProbTable[xIdx][yIdx];
        }
      }
    }
  }

  delete[]  indexTable;  indexTable = NULL;
  return;
}  /* RetrieveCrossProbTable */




void  Model::ProbabilitiesByClassDual (FeatureVectorPtr   example,
                                       KKStr&             classifier1Desc,
                                       KKStr&             classifier2Desc,
                                       ClassProbListPtr&  classifier1Results,
                                       ClassProbListPtr&  classifier2Results,
                                       RunLog&            log
                                      )
{
  delete classifier1Results;  classifier1Results = NULL;
  delete classifier2Results;  classifier2Results = NULL;

  classifier1Desc = Description ();
  classifier2Desc = Description ();

  classifier1Results = ProbabilitiesByClass (example, log);
  if  (classifier1Results)
    classifier2Results = new ClassProbList (*classifier1Results);
}  /* ProbabilitiesByClassDual */






void  Model::WriteModelXMLFields (ostream&  o)  const
{
  //timeSaved = osGetLocalDateTime ();
  ModelTypeStr ().WriteXML ("ModelType", o);
  Name ().WriteXML ("Name", o);
  rootFileName.WriteXML ("RootFileName", o);
  XmlElementMLClassNameList::WriteXML (*classes, "classes", o);
  if  (classesIndex)
    classesIndex->WriteXML ("ClassesIndex", o);


  if  (factoryFVProducer)
    XmlElementKKStr::WriteXML (factoryFVProducer->Name (), "FvFactoryProducer", o);

  if  (fileDesc)
    fileDesc->WriteXML ("FileDesc", o);

  if  (param)
    param->WriteXML ("Param", o);

  timeSaved.YYYY_MM_DD_HH_MM_SS ().WriteXML ("TimeSaved", o);
  XmlElementDouble::WriteXML (trainingTime, "TrainingTime", o);
  XmlElementBool::WriteXML (alreadyNormalized, "AlreadyNormalized", o);
  if  (normParms)
    normParms->WriteXML ("NormParms", o);
} /* WriteModelXMLFields */




XmlTokenPtr  Model::ReadXMLModelToken (XmlTokenPtr  t,
                                       RunLog&      log
                                      )
{
  const KKStr&  varName = t->VarName ();
  if  (t->TokenType () == XmlToken::TokenTypes::tokElement)
  {
    bool  tokenFound = true;

    XmlElementPtr  e = dynamic_cast<XmlElementPtr> (t);

    const KKStr&  varName = e->VarName ();

    if  (varName.EqualIgnoreCase ("ModelType"))
    {
      if  (ModelType ()  != Model::ModelTypeFromStr (e->ToKKStr ()))
      {
        KKStr errMsg (128);
        errMsg << "Model::ReadXMLModelToken   ***ERROR***   Wrong ModelType encountered;  Expected[" << ModelTypeStr () << "] "
               << "ModelType Specified[" << e->ToKKStr () << "].";

        log.Level (-1) << endl << errMsg << endl << endl;
        AddErrorMsg (errMsg, 0);
      }
    }

    else if  (varName.EqualIgnoreCase ("Name"))
      name = e->ToKKStr ();

    else if  (varName.EqualIgnoreCase ("RootFileName"))
      rootFileName = e->ToKKStr ();

    else if  ((varName.EqualIgnoreCase ("Classes"))  &&  (typeid(*e) == typeid (XmlElementMLClassNameList)))
    {
      delete classes;
      classes = dynamic_cast<XmlElementMLClassNameListPtr> (t)->TakeOwnership ();
    }

    else if  (varName.EqualIgnoreCase ("ClassesIndex")  &&  (typeid(*e) == typeid (XmlElementMLClassIndexList)))
    {
      delete classesIndex;
      classesIndex = dynamic_cast<XmlElementMLClassIndexListPtr>(t)->TakeOwnership ();
    }

    else if  (varName.EqualIgnoreCase ("FvFactoryProducer"))
    {
      factoryFVProducer = FactoryFVProducer::LookUpFactory (e->ToKKStr ());
    }

    else if  (varName.EqualIgnoreCase ("FileDesc")  &&  (typeid(*e) == typeid (XmlElementFileDesc)))
    {
      fileDesc = dynamic_cast<XmlElementFileDescPtr>(t)->Value ();
    }

    else if  (varName.EqualIgnoreCase ("Param")) 
    {
      XmlElementModelParamPtr  xmlParameterElement = dynamic_cast<XmlElementModelParamPtr> (e);
      if  (xmlParameterElement)
      {
        delete param;
        param = xmlParameterElement->TakeOwnership ();
      }
      else
      {
        KKStr errMsg (128);
        errMsg << "Model::ReadXMLModelToken   ***ERROR***   ModelParam variable 'param' not defined correctly.";
        log.Level (-1) << endl << errMsg << endl << endl;
        AddErrorMsg (errMsg, 0);
      }
    }

    else if  (varName.EqualIgnoreCase ("TimeSaved"))
      timeSaved = DateTime (e->ToKKStr ());

    else if  (varName.EqualIgnoreCase ("TrainingTime"))
      trainingTime = e->ToDouble ();

    else if  (varName.EqualIgnoreCase ("AlreadyNormalized"))
      alreadyNormalized = e->ToBool ();

    else if  ((varName.EqualIgnoreCase ("NormParms"))  &&  (typeid (*e) == typeid (XmlElementNormalizationParms)))
    {
      delete  normParms;
      normParms = dynamic_cast<XmlElementNormalizationParmsPtr> (e)->TakeOwnership ();
    }

    else
    {
      tokenFound = false;
    }

    if  (tokenFound)
    {
      delete t;
      t = NULL;
    }
  }

  return  t;
}  /* ReadXMLModelToken */



void  Model::ReadXMLModelPost (RunLog&  log)
{
  if  (!param)
  {
    KKStr errMsg = "Model::ReadXMLModelPost   ***ERROR***   'param' not defined.";
    AddErrorMsg (errMsg, 0);
    log.Level (-1) << endl << errMsg << endl << endl;
  }

  else if  (!param->ValidParam ())
  {
    KKStr errMsg = "Model::ReadXMLModelPost   ***ERROR***   'Param' is NOT Valid .";
    AddErrorMsg (errMsg, 0);
    log.Level (-1) << endl << errMsg << endl << endl;
  }

  if  (!classes)
  {
    KKStr errMsg = "Model::ReadXMLModelPost   ***ERROR***   'classes' not defined.";
    AddErrorMsg (errMsg, 0);
    log.Level (-1) << endl << errMsg << endl << endl;
  }

  if  (!fileDesc)
  {
    KKStr errMsg = "Model::ReadXMLModelPost   ***ERROR***   'fileDesc' not defined.";
    AddErrorMsg (errMsg, 0);
    log.Level (-1) << endl << errMsg << endl << endl;
  }

  if  (errors.size () > 0)
  {
    log.Level (-1) << "Model::ReadXMLModelPost    Errors were detected;  model is INVALID." << endl;
    validModel = false;
  }
  else
  {
    AllocatePredictionVariables ();
  }

}  /* ReadXMLModelPost */



void  Model::PredictRaw (FeatureVectorPtr  example,
  MLClassPtr&       predClass,
  double&           dist
)
{
  example->ExampleFileName ();
  predClass = NULL;
  dist = 0.0;
}

