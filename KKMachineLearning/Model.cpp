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


#include  "MemoryDebug.h"

using namespace  std;


#include "KKBaseTypes.h"
#include "KKException.h"
#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace  KKB;


#include "Model.h"
#include "ModelOldSVM.h"
#include "ModelSvmBase.h"
#include "ModelKnn.h"
#include "ModelUsfCasCor.h"

#include "FeatureEncoder2.h"
#include "FeatureNumList.h"
#include "FeatureVector.h"
#include "ModelParam.h"
using namespace  KKMachineLearning;



Model::Model (const Model&  _model):
    alreadyNormalized       (false),
    cancelFlag              (_model.cancelFlag),
    classes                 (NULL),
    classesIndex            (NULL),
    classProbs              (NULL),
    crossClassProbTable     (NULL),
    crossClassProbTableSize (0),
    encoder                 (NULL),
    fileDesc                (_model.fileDesc),
    log                     (_model.log),
    name                    (_model.name),
    numOfClasses            (_model.numOfClasses),
    normParms               (NULL),
    param                   (NULL),
    rootFileName            (_model.rootFileName),
    trainExamples           (NULL),
    validModel              (_model.validModel),
    votes                   (NULL),
    weOwnTrainExamples      (false),
    trianingPrepTime        (0.0),
    trainingTime            (_model.trainingTime),
    trainingTimeStart       (_model.trainingTime)
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
    classesIndex = new ClassIndexList (*_model.classesIndex);


  if  (_model.encoder)
    encoder = new FeatureEncoder2 (*_model.encoder);
}


    
/**
 @brief  Use this when you are planning on creating a empty model without parameters.
 */
Model::Model (FileDescPtr    _fileDesc,
              VolConstBool&  _cancelFlag,
              RunLog&        _log
             ):
    alreadyNormalized        (false),
    cancelFlag               (_cancelFlag),
    classes                  (NULL),
    classesIndex             (NULL),
    crossClassProbTable      (NULL),
    encoder                  (NULL),
    fileDesc                 (_fileDesc),
    log                      (_log),
    name                     (),
    normParms                (NULL),
    numOfClasses             (0),
    param                    (NULL),
    classProbs               (NULL),
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

    
    
/**
 *@brief Creates a new svm model from the provided example data
 *@param[in] _name
 *@param[in] _param Will make own local copy.
 *@param[in] _fileDesc A description of the data file.
 *@param[in] _cancelFlag  This field will be monitored and if it goes true then any processing 
 *           going on will quit and exit back to caller.
 *@param[in] _log A logfile stream. All important events will be output to this stream
 */
Model::Model (const KKStr&       _name,
              const ModelParam&  _param,      // Create new model from
              FileDescPtr        _fileDesc,
              VolConstBool&      _cancelFlag,
              RunLog&            _log
             ):
    alreadyNormalized     (false),
    cancelFlag            (_cancelFlag),
    classes               (NULL),
    classesIndex          (NULL),
    crossClassProbTable   (NULL),
    encoder               (NULL),
    fileDesc              (_fileDesc),
    log                   (_log),
    name                  (_name),
    normParms             (NULL),
    numOfClasses          (0),
    param                 (NULL),
    classProbs            (NULL),
    rootFileName          (),
    trainExamples         (NULL),
    trianingPrepTime      (0.0),
    trainingTime          (0.0),
    trainingTimeStart     (0.0),
    validModel            (true),
    votes                 (NULL),
    weOwnTrainExamples    (false)

{
  param = _param.Duplicate ();

  log.Level (20) << "ModelKnn::ModelKnn - Constructing From Training Data." << endl;
}




/**
 * @brief Frees any memory allocated by, and owned by the Model
 */
Model::~Model ()
{
  DeAllocateSpace ();
  delete  classesIndex;
  classesIndex = NULL;

  delete  classes;
  classes = NULL;

  delete  encoder;
  encoder = NULL;

  delete  normParms;
  normParms = NULL;

  if  (weOwnTrainExamples)
  {
    delete  trainExamples;
    trainExamples = NULL;
  }

  delete  param;
  param = NULL;
}


kkint32  Model::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = sizeof (Model) + rootFileName.MemoryConsumedEstimated ();
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



ModelPtr  Model::CreateFromStream (istream&       i,
                                   FileDescPtr    _fileDesc,
                                   VolConstBool&  _cancelFlag,
                                   RunLog&        _log
                                  )
{
  istream::pos_type startPos = i.tellg ();

  char  buff[20480];
  KKStr  ln;

  ModelTypes  modelType = mtNULL;


  // First we need to determine which type of model this is.  We will
  // scan through the file until we locate a ModelParamType field.
  while  (i.getline (buff, sizeof (buff)))
  {
    ln = buff;
    KKStr  lineName = ln.ExtractToken2 ("\t");

    if  (lineName.EqualIgnoreCase ("</Model>"))
    {
      // We did not find the parameter type 
      break;
    }

    if  (lineName.EqualIgnoreCase ("ModelType"))
    {
      KKStr  modelTypeStr = ln.ExtractToken2 ("\t");
      modelType = ModelTypeFromStr (modelTypeStr);
      if  (modelType == mtNULL)
      {
        _log.Level (-1) << endl
          << "ModelParam::CreateFromStream  ***ERROR***   Invalid ModelType[" << modelTypeStr << "]." << endl
          << endl;
      }
      break;
    }
  }

  if  (modelType == mtNULL)
  {
    // We never found the type of parameter we are looking for.
    _log.Level (-1) << endl
      << "Model::CreateFromStream   ***ERROR***   No Parameter Type was defined." << endl
      << endl;
    return NULL;
  }

  i.seekg (startPos);

  ModelPtr model = NULL;
  switch  (modelType)
  {
  case  mtKNN:      model = new ModelKnn     (_fileDesc, _cancelFlag, _log);
                    break;

  case  mtOldSVM:   model = new ModelOldSVM  (_fileDesc, _cancelFlag, _log);
                    break;
 
  case  mtSvmBase:  model = new ModelSvmBase (_fileDesc, _cancelFlag, _log);
                    break;

  }

  if  (!model)
    return  NULL;

  bool  successful = false;
  try
  {
    model->ReadXML (i, successful);
  }
  catch  (const KKException& e)
  {
    _log.Level (-1) << endl
      << "Model::CreateFromStream    ***ERROR***  Exception occured in executing 'ReadXML'" << endl
      << "      " << e.ToString ()  << endl
      << endl;
    successful = false;
    delete  model;
    model = NULL;
  }
  catch (...)
  {
    _log.Level (-1) << endl
      << "Model::CreateFromStream    ***ERROR***  Exception occured in executing 'ReadXML'" << endl
      << endl;
    successful = false;
    delete  model;
    model = NULL;
  }

  if  (!successful)
  {
    _log.Level (-1) << endl
      << "Model::CreateFromStream    ***ERROR***  Loading Model from file." << endl
      << endl;
    delete  model;
    model = NULL;
  }

  return  model;
}  /* CreateFromStream */




KKStr  Model::ModelTypeToStr (ModelTypes   _modelingType)
{
  if  (_modelingType == mtNULL)
    return "NULL";
  
  else if  (_modelingType == mtOldSVM)
    return "OldSVM";

  else if  (_modelingType == mtSvmBase)
    return "SvmBase";
  
  else if  (_modelingType == mtKNN)
    return "KNN";

  else if  (_modelingType == mtUsfCasCor)
    return "UsfCasCor";

  else
    return "NULL";
}  /* ModelingMethodToStr */




Model::ModelTypes  Model::ModelTypeFromStr (const KKStr&  _modelingTypeStr)
{
  if  (_modelingTypeStr.EqualIgnoreCase ("OldSVM")  ||  _modelingTypeStr.EqualIgnoreCase ("One_Level"))
    return mtOldSVM;

  else if  (_modelingTypeStr.EqualIgnoreCase ("SvmBase"))
    return mtSvmBase;

  else if  (_modelingTypeStr.EqualIgnoreCase ("KNN"))
    return mtKNN;

  else if  (_modelingTypeStr.EqualIgnoreCase ("UsfCasCor"))
    return mtUsfCasCor;

  else
    return mtNULL;
}  /* ModelingMethodFromStr */




ModelPtr  Model::CreateAModel (ModelTypes        _modelType,
                               const KKStr&      _name,
                               const ModelParam& _param,  
                               FileDescPtr       _fileDesc,
                               VolConstBool&     _cancelFlag,
                               RunLog&           _log
                              )
{
  ModelPtr  model = NULL;
  try
  {
    switch  (_modelType)
    {
    case  mtOldSVM:    model = new ModelOldSVM    (_name, dynamic_cast<const ModelParamOldSVM&>    (_param), _fileDesc, _cancelFlag, _log);
                       break;

    case  mtSvmBase:   model = new ModelSvmBase   (_name, dynamic_cast<const ModelParamSvmBase&>   (_param), _fileDesc, _cancelFlag, _log);
                       break;

    case  mtKNN:       model = new ModelKnn       (_name, dynamic_cast<const ModelParamKnn&>       (_param), _fileDesc, _cancelFlag, _log);
                       break;

    case  mtUsfCasCor: model = new ModelUsfCasCor (_name, dynamic_cast<const ModelParamUsfCasCor&> (_param), _fileDesc, _cancelFlag, _log);
                       break;

    }  /* end of switch */
  }
  catch  (const std::exception&  e)
  {
    delete  model;
    model = NULL;
    throw  KKException ("Model::CreateAModel  Exception calling constructor.", e);
  }
  catch  (const char*  e2)
  {
    delete  model;
    model = NULL;

    KKStr  exceptionStr = "Model::CreateAModel  Exception calling constructor[";
    exceptionStr << e2 << "]."; 
    throw  KKException (exceptionStr);
  }

  catch  (...)
  {
    delete  model;
    model = NULL;
    throw  KKException ("Model::CreateAModel  Exception calling constructor.  No info provided.");
  }

  return  model;
}  /*  CreateAModel */




void  Model::AllocatePredictionVariables ()
{
  kkuint32  x, y;

  DeAllocateSpace ();

  if  (classes == NULL)
  {
    log.Level (-1) << endl << endl 
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
  kkuint32  x;
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



const FeatureNumList&   Model::GetFeatureNums ()  const
{
  if  (!param)
    throw KKException ("Model::GetFeatureNums  'param == NULL'.");
  return  param->SelectedFeatures ();
}


bool  Model::NormalizeNominalAttributes ()  const
{
  if  (!param)
    throw KKException ("Model::NormalizeNominalAttributes  'param == NULL'.");

  if  (param->EncodingMethod () == ModelParam::NoEncoding)
    return  true;
  else
    return  false;
}  /* NormalizeNominalAttributes */



const FeatureNumList&  Model::SelectedFeatures () const
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




void  Model::Load (const KKStr&  _rootFileName,
                   bool&         _successful
                  )
{
  log.Level (20) << "Model::Load  Load Model in File[" << _rootFileName << "]." << endl;
  rootFileName = _rootFileName;
  KKStr  fileName = rootFileName + ".Model";

  ifstream  i (fileName.Str (), ios_base::in);
  if  (!i.is_open ())
  {
    log.Level (-1) << endl << endl << "Model::Load   Failed to open file[" << fileName << "]" << endl;
    _successful = false;
    return;
  }

  ReadXML (i, _successful);

  i.close ();
}  /* Load */




void  Model::Save (const KKStr&  _rootFileName,
                   bool&         _successful
                  )
{
  log.Level (20) << "Model::Save  Saving Model in File["
                 << _rootFileName << "]."
                 << endl;

  _successful = true;

  rootFileName = _rootFileName;

  KKStr  fileName = rootFileName + ".Model";

  ofstream  o (fileName.Str ());
  if  (!o.is_open ())
  {
    _successful = false;
    log.Level (-1) << endl << endl 
      << "Model::Save    ***ERROR***   Opening[" << fileName << "]." << endl;
    return;
  }

  WriteXML (o);

  o.close ();
}  /* Save */




void  Model::WriteXML (ostream&  o)
{
  log.Level (20) << "Model::WriteXML  Saving Model in File." << endl;

  o << "<Model>" << endl;
  o << "ModelType"          << "\t" << ModelTypeStr ()                                  << endl;
  o << "Name"               << "\t" << Name ()                                          << endl;

  o << "RootFileName"       << "\t" << rootFileName                                     << endl;
  o << "Classes"            << "\t" << classes->ToCommaDelimitedStr ()                  << endl;
  if  (classesIndex)
    o << "ClassesIndex"       << "\t" << classesIndex->ToCommaDelString ()              << endl;

  if  (param)
  {
    o << "<Parameters>"  << endl;
    param->WriteXML (o);
    o << "</Parameters>" << endl;
  }
  o << "Time"               << "\t" << osGetLocalDateTime ()                            << endl;
  o << "TrainingTime"       << "\t" << trainingTime                                     << endl;
  o << "AlreadyNormalized"  << "\t" << (alreadyNormalized ? "Yes" : "No")               << endl;
  if  (normParms)
    normParms->Write (o);

  o << "<SpecificImplementation>"  << endl;
  WriteSpecificImplementationXML (o);
  o << "</SpecificImplementation>"  << endl;

  o << "</Model>" << endl;

} /* WriteXML */




void  Model::ReadXML (istream&  i,
                      bool&     _successful
                     )
{
  validModel = true;
  delete  normParms;
  normParms = NULL;
  _successful = true;

  delete  classesIndex;
  classesIndex = NULL;


  char  buff[40960];

  while  (i.getline (buff, sizeof (buff)))
  {
    KKStr  ln (buff);

    KKStr  field = ln.ExtractQuotedStr ("\n\r\t", true);
    field.Upper ();

    if  (field.Empty ())
      continue;

    if  (field.EqualIgnoreCase ("</Model>"))
      break;

    if  (field.EqualIgnoreCase ("CLASSES"))
    {
      delete  classes;
      classes = MLClassList::BuildListFromDelimtedStr (ln, ',');
      delete  classesIndex;
      classesIndex = new ClassIndexList (*classes);
    }

    else if  (field.EqualIgnoreCase ("ClassesIndex"))
    {
      delete  classesIndex;  classesIndex = NULL;
      classesIndex = new ClassIndexList ();
      classesIndex->ParseClassIndexList (ln);
    }

    else if  (field.EqualIgnoreCase ("Name"))
    {
      name = ln.ExtractToken2 ("\n\r\t");
    }

    else if  (field == "TIME")
    {
    }

    else if  (field.EqualIgnoreCase ("<Parameters>"))
    {
      delete  param;
      param = NULL;

      try
      {
        param = ModelParam::CreateModelParam (i, fileDesc, log);
      }
      catch  (const exception&  e)
      {
        _successful = false;
        validModel = false;
        KKStr  errMsg;
        errMsg << "Exception executing fuction 'ModelParam::CreateModelParam'.  Exception[" << e.what () << "]";
        log.Level (-1) << endl << "Model::ReadXML    ***ERROR***    "  << errMsg << endl << endl;
        throw KKException (errMsg);
      }

      if  (!param)
      {
        _successful = false;
        validModel = false;
        KKStr  errMsg = "Model::ReadXML    ***ERROR***    (param == NULL)";
        log.Level (-1) << errMsg << endl;
      }

      else if  (!param->ValidParam ())
      {
        _successful = false;
        validModel = false;
        log.Level (-1) << endl << endl << "Model::ReadXML   ***ERROR***   <ModelParam>  was invalid." << endl << endl;
      }
    }

    else if  (field == "TRAININGTIME")
      trainingTime = ln.ExtractTokenDouble ("\n\r\t");

    else if  (field.EqualIgnoreCase ("AlreadyNormalized"))
      alreadyNormalized = ln.ExtractTokenBool ("\t");

    else if  (field.EqualIgnoreCase ("<NormalizationParms>"))
    {
      delete  normParms;
      normParms = NULL;
      _successful = true;
      normParms = new NormalizationParms (fileDesc, i, _successful, log);
      if  (!_successful)
      {
        KKStr  errMsg = "Model::ReadXML    ***ERROR***     Reading in <NormalizationParms>";
        log.Level (-1) << endl << endl
                       << errMsg << endl
                       << endl;
        throw KKException (errMsg);
      }
    }
    else if  (field.EqualIgnoreCase ("<SpecificImplementation>"))
    {
      ReadSpecificImplementationXML (i, _successful);
    }
  }

  if  (classes == NULL)
  {
    _successful = false;
    log.Level (-1) << endl << endl 
                   << "Model::ReadXML    Class List was not defined." << endl
                   << endl;
  }

  else if  ((normParms == NULL)  &&  (!alreadyNormalized))
  {
    _successful = false;
    log.Level (-1) << endl << endl 
                   << "Model::ReadXML    Normalization Parmameters was not defined." << endl
                   << endl;
  }

  else
  {
    numOfClasses = classes->QueueSize ();
    AllocatePredictionVariables ();
  }
}  /* ReadXML */





void  Model::ReadSkipToSection (istream& i, 
                                KKStr    sectName,
                                bool&    sectionFound
                               )
{
  sectionFound = false;
  char  buff[10240];
  sectName.Upper ();


  KKStr  field;

  // Skip to start of OneVsOne section
  while  (i.getline (buff, sizeof (buff)))
  {
    KKStr  ln (buff);
    field = ln.ExtractQuotedStr ("\n\r\t", true);
    field.Upper ();
    if  (field == sectName)
    {
      sectionFound = true;
      break;
    }
  }

  if  (!sectionFound)
  {
    log.Level (-1) << endl << endl
                   << "Model::ReadSkipToSection    *** Could not find section[" << sectName << "]" <<endl
                   << endl;
  }
}  /* ReadSkipToSection */




/**
 *@brief Performs operations such as FeatureEncoding, and  Normailization.  The actual training 
 *       of models occurs in the specific implementation of 'Model'.
 */
void  Model::TrainModel (FeatureVectorListPtr  _trainExamples,
                         bool                  _alreadyNormalized,
                         bool                  _takeOwnership  /*!< True = Model will take ownership of these examples */
                        )
{
  log.Level (40) << "Model::TrainModel" << endl;

  double  prepStartTime = osGetSystemTimeUsed ();

  if  (_trainExamples == NULL)
  {
    KKStr  errMsg = "ModelSvmBase::TrainModel   (_trainExamples == NULL)";
    log.Level (-1) << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  if  (_trainExamples->QueueSize () < 2)
  {
    KKStr  errMsg = "ModelSvmBase::TrainModel   (_trainExamples == NULL)";
    log.Level (-1) << endl << endl << errMsg << endl << endl;
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
  classesIndex = new ClassIndexList (*classes);


  if  (param->ExamplesPerClass () < int32_max)
    ReduceTrainExamples ();

  if  (!_alreadyNormalized)
  {
    if  (!weOwnTrainExamples)
    {
      FeatureVectorListPtr  temp = new FeatureVectorList (*trainExamples, true);
      weOwnTrainExamples = true;
      trainExamples = temp;
    }
    delete  normParms;
    normParms = new NormalizationParms (*param, *trainExamples, log);
    normParms->NormalizeExamples (trainExamples);
  }

  if  (param->EncodingMethod () == ModelParam::Encoding_NULL)
  {
    // There is nothing for us to do.
    return;
  }

  if  ((param->EncodingMethod () != ModelParam::Encoding_NULL)  &&   (param->EncodingMethod () != ModelParam::NoEncoding))
  {
    if  (!encoder)
      encoder = new FeatureEncoder2 (*param, fileDesc, log);

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

  log.Level (40) << "Model::TrainModel   Exiting." << endl;
}  /* TrainModel */





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

  // I do not believe we need the encoder at this point. At least not for the Features Selected part.  Maybe the conversion from niminal fields will make sense.
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
 @brief  Reduces the Training Images down to the size dictated by the 'examplesPerClass' parameter.
 */
void  Model::ReduceTrainExamples ()
{
  kkint32  examplesPerClass = param->ExamplesPerClass ();
  kkuint32  zed = 0;

  if  (examplesPerClass < 0)
    examplesPerClass = int32_max;

  bool  reductionNeeded = false;

  {
    // First lets see if reduction is even nessasary.
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
  }

  if  (!reductionNeeded)
  {
    log.Level (20) << "Model::ReduceTrainExamples    Was not needed.  No classes exceeded 'examplesPerClass'." << endl;
    return;
  }

  FeatureVectorListPtr  reducedSet = new FeatureVectorList (fileDesc, false, log);
  FeatureVectorListPtr  deleteSet  = new FeatureVectorList (fileDesc, false, log);  // Examples taht we do not use will need to be deleted.
  MLClassList::iterator  idx;

  for  (idx = classes->begin ();  idx != classes->end ();  idx++)
  {
    MLClassPtr  ic = *idx;
    FeatureVectorListPtr  examplesThisClass = trainExamples->ExtractImagesForAGivenClass (ic);
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
    trainExamples = reducedSet;
    reducedSet = NULL;
    trainExamples->Owner (false);
    deleteSet->Owner (false);
    delete  deleteSet;
    deleteSet = NULL;
  }
}  /* ReduceTrainExamples */




void  Model::RetrieveCrossProbTable (MLClassList&   classes,
                                     double**          crossClassProbTable  // two dimension matrix that needs to be classes.QueueSize ()  squared.
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

  delete  indexTable;  indexTable = NULL;
  return;
}  /* RetrieveCrossProbTable */





