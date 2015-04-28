#include "FirstIncludes.h"
#include <stdio.h>
#include <string>
#include <sstream>
#include <ctype.h>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#ifdef  WIN32
#include <ostream>
#include <windows.h>
#else
#include <fstream>
#endif
#include "MemoryDebug.h"
using namespace  std;


#include "KKBaseTypes.h"
#include "KKException.h"
#include "KKStr.h"
#include "OSservices.h"
#include "RBTree.h"
using namespace  KKB;


#include "TrainingProcess2.h"
#include "ClassAssignments.h"
#include "ClassProb.h"
#include "DuplicateImages.h"
#include "FactoryFVProducer.h"
#include "FeatureFileIO.h"
#include "FeatureNumList.h"
#include "FeatureVector.h"
#include "GrayScaleImagesFVProducer.h"
#include "MLClass.h"
#include "ImageFeaturesDataIndexed.h"
#include "ImageFeaturesNameIndexed.h"
#include "Model.h"
#include "ModelDual.h"
#include "ModelOldSVM.h"
#include "ModelKnn.h"
#include "ModelSvmBase.h"
#include "TrainingClass.h"
#include "TrainingConfiguration2.h"
using namespace  KKMLL;


TrainingProcess2::TrainingProcess2 (TrainingConfiguration2Const*  _config,
                                    FeatureVectorListPtr          _excludeList,
                                    RunLog&                       _log,
                                    ostream*                      _report,
                                    bool                          _forceRebuild,
                                    bool                          _checkForDuplicates,
                                    VolConstBool&                 _cancelFlag,
                                    KKStr&                        _statusMessage
                                   ):
  abort                     (false),
  buildDateTime             (DateTime (1900, 1, 1, 0, 0, 0)),
  cancelFlag                (_cancelFlag),
  config                    (_config),
  configOurs                (NULL),
  configFileName            (""),
  configFileNameSpecified   (""),
  duplicateCount            (0),
  duplicateDataCount        (0),
  excludeList               (_excludeList),
  featuresAlreadyNormalized (false),
  fileDesc                  (NULL),
  fvFactoryProducer         (NULL),
  mlClasses                 (NULL),
  log                       (_log),
  model                     (NULL),
  priorProbability          (NULL),
  report                    (_report),
  savedModelName            (),
  statusMessage             (_statusMessage),
  subTrainingProcesses      (NULL),
  trainingExamples          (NULL),
  weOwnTrainingExamples     (true),
  weOwnMLClasses            (true)
{
  log.Level (10) << "TrainingProcess2::TrainingProcess2   9 Parameters." << endl;

  fvFactoryProducer = config->FvFactoryProducer ();
  fileDesc = fvFactoryProducer->FileDesc ();

  configFileName          = config->FileName ();
  configFileNameSpecified = config->ConfigFileNameSpecified ();
  trainingExamples = fvFactoryProducer->ManufacturFeatureVectorList (true,  log);

  configFileName = TrainingConfiguration2::GetEffectiveConfigFileName (configFileName);

  savedModelName = osRemoveExtension (configFileName) + ".Save";

  bool      changesMadeToTrainingLibrary = false;
  DateTime  configTimeStamp;
  DateTime  savedModelTimeStamp;
  DateTime  latestTrainingImageTimeStamp;

  bool  useExistingSavedModel = true;
  if  (_forceRebuild)
  {
    useExistingSavedModel = false;
    savedModelTimeStamp = osGetLocalDateTime ();
  }

  else if  (osFileExists (savedModelName))
  {
    savedModelTimeStamp   = osGetFileDateTime (savedModelName);
  }

  else
  {
    // Setting 'savedModelTimeStamp' to current time,  so later in function when trying to determine if 
    // it is ok to use saved version, it will fail test and rebuild new model.
    savedModelTimeStamp = osGetLocalDateTime ();
    useExistingSavedModel = false;
  }

  if  (!config->FormatGood ())
  {
    if  (report)
    {
      *report << endl << endl
              << "****** Invalid Configuration File[" << configFileName << "]  ******" << endl
              << endl;
    }

    log.Level (-1) << "TrainingProcess2  Invalid Configuration File Specified." << endl
      << endl
      << config->FormatErrors () << endl
      << endl;

    Abort (true);
    return;
  }

  if  (weOwnMLClasses)
    delete  mlClasses;
  mlClasses = config->ExtractClassList ();
  weOwnMLClasses = true;
  mlClasses->SortByName ();

  configTimeStamp = osGetFileDateTime (configFileName);
  if  (configTimeStamp > savedModelTimeStamp)
    useExistingSavedModel = false;

  cout  << "*PHASE_MSG* Extracting Training Class Data" << endl;

  try
  {ExtractTrainingClassFeatures (latestTrainingImageTimeStamp, changesMadeToTrainingLibrary);}
  catch (std::exception& e1)
  {
    log.Level (-1) << "TrainingProcess2    *** EXCEPTION *** occurred calling 'ExtractTrainingClassFeatures'." << endl
                   << "   Exception[" << e1.what () << "]" << endl;
    Abort (true);
  }
  catch (...)
  {
    log.Level (-1) << "TrainingProcess2    *** EXCEPTION *** occurred calling 'ExtractTrainingClassFeatures'." << endl;
    Abort (true);
  }

  if  (cancelFlag)
    Abort (true);

  if  (Abort ())
  {
    log.Level (-1) << "TrainingProcess2    *** Aborted During Feature Extraction ***" << endl;
    return;
  }

  if  ((latestTrainingImageTimeStamp > savedModelTimeStamp)  ||  (changesMadeToTrainingLibrary))
    useExistingSavedModel = false;

  if  (excludeList)
  {
    useExistingSavedModel = false;

    kkint32  origSizeOfExamples = trainingExamples->QueueSize ();
    RemoveExcludeListFromTrainingData ();
    if  (origSizeOfExamples != trainingExamples->QueueSize ())
       useExistingSavedModel = false;
  }

  if  (useExistingSavedModel)
  {
    statusMessage = "Loading existing trained model[" + osGetRootNameWithExtension (savedModelName) + "]";

    // We are not going to need the training trainingExamples we loaded so we can go ahead and delete them now.
    delete  trainingExamples;  trainingExamples = NULL;

    buildDateTime = savedModelTimeStamp;  // Set to timestamp of save file for now.  Will be overwriting by the
                                          // 'BuildDateTime' field in the save file if it exists.

    ifstream  in (savedModelName.Str ());
    if  (!in.is_open ())
    {
      log.Level (-1) << endl << endl 
                     << "TrainingProcess2    ***ERROR***    Could not existing Training Model[" << savedModelName << "]" << endl
                     << endl;

      useExistingSavedModel = false;
    }
    else
    {
      try
      {
        bool  successful = false;
        Read (in, successful);
        if  (!successful)
        {
          log.Level (-1) << endl << endl << endl
              << "TrainingProcess2      *** Invalid Format in SaveFile[" << savedModelName << "]  we will have to rebuild model." << endl
              << endl;

          useExistingSavedModel = false;
          delete  model;             model            = NULL;
          delete  trainingExamples;  trainingExamples = NULL;
          if  (weOwnMLClasses)
          {
            delete  mlClasses;
            mlClasses = NULL;
          }

          trainingExamples = fvFactoryProducer->ManufacturFeatureVectorList (true,  log);

          if  (!config->FormatGood())
          {
            log.Level (-1) << "TrainingProcess2  Invalid Configuration File Specified." << endl;
            Abort (true);
            return;
          }

          if  (weOwnMLClasses)
            delete  mlClasses;
          mlClasses = config->ExtractClassList ();
          mlClasses->SortByName ();
          weOwnMLClasses = true;

          ExtractTrainingClassFeatures (latestTrainingImageTimeStamp, changesMadeToTrainingLibrary);
          if  (cancelFlag)
            Abort (true);
        }

        in.close ();
      }
      catch (std::exception e2)
      {
        log.Level (-1) << "TrainingProcess2    *** EXCEPTION *** occurred while processing existing model." << endl
                       << "   Exception[" << e2.what () << ":]" << endl;
        Abort (true);
      }
      catch (...)
      {
        log.Level (-1) << "TrainingProcess2    *** EXCEPTION *** occurred while processing existing model." << endl;
        Abort (true);
      }
    }
  }

  if  ((!useExistingSavedModel)  &&  (!cancelFlag))
  {
    statusMessage = "Building Support Vector Machine";

    if  (_checkForDuplicates)
      CheckForDuplicates (true);

    // trainer->ReportTraningClassStatistics (*report);
    CreateModelsFromTrainingData ();
    if  (Abort ())
    {
      log.Level (-1) << "TrainingProcess2    *** Aborted During Model Creation ***" << endl;
      return;
    }

    if  (!excludeList)
    {
      SaveResults ();
      if  (Abort ())
      {
        log.Level (-1) << "TrainingProcess2    *** Aborted ***" << endl;
        return;
      }
    }
  }

  if  (false)
  {
    kkuint32  numExamplesWritten = 0;
    bool  successful = false;
    KKStr  fn = "C:\\Larcos\\Classifier\\TrainingModels\\TrainingData\\" + KKB::osGetRootName (configFileName) + ".data";

    FeatureFileIOPtr  driver = NULL;
    if  (fvFactoryProducer)
      driver = fvFactoryProducer->DefaultFeatureFileIO ();

    if  (driver)
      driver->SaveFeatureFile (fn, trainingExamples->AllFeatures (), *trainingExamples, numExamplesWritten, cancelFlag, successful, log);
  }

  //  At this point we should no longer need the training trainingExamples.  
  delete  trainingExamples;  trainingExamples = NULL;

  log.Level (20) << "TrainingProcess2::TrainingProcess2  Exiting Constructor" << endl;
}



//*****************************************************************************************
//*   Will build a new model from scratch for the specified class level. Will also remove *
//*   duplicates.                                                                         *
//*****************************************************************************************
TrainingProcess2::TrainingProcess2 (TrainingConfiguration2Const*  _config,
                                    FeatureVectorListPtr          _excludeList,
                                    RunLog&                       _log,
                                    kkuint32                      _level,
                                    VolConstBool&                 _cancelFlag,
                                    KKStr&                        _statusMessage
                                   ):

  abort                     (false),
  buildDateTime             (DateTime (1900,1,1,0, 0, 0)),
  cancelFlag                (_cancelFlag),
  config                    (_config),
  configOurs                (NULL),
  configFileName            (""),
  configFileNameSpecified   (""),
  duplicateCount            (0),
  duplicateDataCount        (0),
  excludeList               (_excludeList),
  featuresAlreadyNormalized (false),
  fileDesc                  (NULL),
  fvFactoryProducer         (NULL),
  mlClasses                 (NULL),
  log                       (_log),
  model                     (NULL),
  priorProbability          (NULL),
  report                    (NULL),
  savedModelName            (),
  statusMessage             (_statusMessage),
  subTrainingProcesses      (NULL),
  trainingExamples          (NULL),
  weOwnTrainingExamples     (true),
  weOwnMLClasses            (true)
{
  log.Level (20) << "TrainingProcess2::TrainingProcess2" << endl;
  if  (config == NULL)
  {
    statusMessage = "Configuration file was not provided.";
    log.Level (-1) << "TrainingProcess2  ***ERROR***   Configuration file was not provided." << endl;
    Abort (true);
    return;
  }

  fvFactoryProducer = config->FvFactoryProducer ();

  if  ((!config->FormatGood ()))
  {
    statusMessage = "Configuration file is invalid.";
    log.Level (-1) << "TrainingProcess2  Invalid Configuration File Specified." << endl;
    Abort (true);
    return;
  }

  fileDesc = fvFactoryProducer->FileDesc ();
  configFileName          = config->FileName ();
  configFileNameSpecified = config->ConfigFileNameSpecified ();
  trainingExamples = fvFactoryProducer->ManufacturFeatureVectorList (true,  log);
  configFileName = TrainingConfiguration2::GetEffectiveConfigFileName (configFileName);

  mlClasses = config->ExtractClassList ();

  {
    DateTime  latestTrainingImageTimeStamp;
    bool      changesMadeToTrainingLibrary;
  
    ExtractTrainingClassFeatures (latestTrainingImageTimeStamp, changesMadeToTrainingLibrary);
    if  (cancelFlag)
      Abort (true);

    if  (Abort ())
    {
      statusMessage = "*** Aborted During Feature Extraction ***";
      log.Level (-1) << "TrainingProcess2    *** Aborted During Feature Extraction ***" << endl;
      return;
    }
  }

  if  (excludeList)
    RemoveExcludeListFromTrainingData ();

  statusMessage = "Building Support Vector Machine";

  CheckForDuplicates (true);

  if  (_level < mlClasses->NumHierarchialLevels ())
  {
    // We need to build data that is specific to the specified _level.
    FeatureVectorListPtr  newExamples = trainingExamples->ExtractExamplesForHierarchyLevel (_level);
    delete  trainingExamples;
    trainingExamples = newExamples;
    newExamples = NULL;

    TrainingConfiguration2Ptr  newConfig = config->GenerateAConfiguraionForAHierarchialLevel (_level, log);
    delete configOurs;
    configOurs = newConfig;
    newConfig = NULL;
    config = configOurs;

    if  (weOwnMLClasses)
      delete  mlClasses;
    mlClasses = config->ExtractClassList ();
    mlClasses->SortByName ();
    weOwnMLClasses = true;
  }

  CreateModelsFromTrainingData ();
  if  (Abort ())
  {
    statusMessage = "*** Aborted During Model Creation ***";
    log.Level (-1) << "TrainingProcess2    *** Aborted During Model Creation ***" << endl;
    return;
  }

  log.Level (20) << "TrainingProcess2::TrainingProcess2  Exiting Constructor" << endl;
}




TrainingProcess2::TrainingProcess2 (const KKStr&         _configFileName,
                                    RunLog&              _log,
                                    bool                 _featuresAlreadyNormalized,
                                    VolConstBool&        _cancelFlag,
                                    KKStr&               _statusMessage
                                   ):

  abort                     (false),
  buildDateTime             (DateTime (1900,1,1,0, 0, 0)),
  cancelFlag                (_cancelFlag),
  config                    (NULL),
  configOurs                (NULL),
  configFileName            (_configFileName),
  configFileNameSpecified   (_configFileName),
  duplicateCount            (0),
  duplicateDataCount        (0),
  excludeList               (NULL),
  featuresAlreadyNormalized (_featuresAlreadyNormalized),
  fileDesc                  (NULL),
  fvFactoryProducer         (NULL),
  mlClasses                 (new MLClassList ()),
  log                       (_log),
  model                     (NULL),
  priorProbability          (NULL),
  report                    (NULL),
  savedModelName            (),
  statusMessage             (_statusMessage),
  subTrainingProcesses      (NULL),
  trainingExamples          (NULL),
  weOwnTrainingExamples     (true),
  weOwnMLClasses            (true)

{
  log.Level (20) << "TrainingProcess2::TrainingProcess2     Loading an existing trained model" << endl;

  configFileName = TrainingConfiguration2::GetEffectiveConfigFileName (configFileName);
  savedModelName = osRemoveExtension (configFileName) + ".Save";
  KKB::DateTime  savedModelTimeStamp  = osGetFileDateTime (savedModelName);

  ifstream  in (savedModelName.Str ());
  if  (!in.is_open ())
  {
    log.Level (-1) << endl << endl 
                   << "TrainingProcess2    *** ERROR ***  Training Model Save File[" << savedModelName << "] can not be opened." << endl
                   << endl;
    Abort (true);
    return;
  }

  buildDateTime = savedModelTimeStamp;

  bool  successful = false;

  Read (in, successful);

  if  ((!successful)  ||  (model && (!model->ValidModel ())))
  {
    log.Level (-1) << endl << endl 
                   << "TrainingProcess2    *** ERROR ***  Training Model Save File[" << savedModelName << "] Invalid Format." << endl
                   << endl;
    Abort (true);
  }

  in.close ();
  if  (config)
  {
    fvFactoryProducer = config->FvFactoryProducer ();
    fileDesc = fvFactoryProducer->FileDesc ();
  }

  log.Level (20) << "TrainingProcess2::TrainingProcess2(5 parameters)   Exiting." << endl;

  statusMessage = "All Done";
  return;
}





TrainingProcess2::TrainingProcess2 (istream&       _in,
                                    RunLog&        _log,
                                    bool           _featuresAlreadyNormalized,
                                    VolConstBool&  _cancelFlag,
                                    KKStr&         _statusMessage
                                   ):

  abort                     (false),
  buildDateTime             (DateTime (1900,1,1,0, 0, 0)),
  cancelFlag                (_cancelFlag),
  config                    (NULL),
  configOurs                (NULL),
  configFileName            (),
  configFileNameSpecified   (),
  duplicateCount            (0),
  duplicateDataCount        (0),
  excludeList               (NULL),
  featuresAlreadyNormalized (_featuresAlreadyNormalized),
  fvFactoryProducer         (NULL),
  fileDesc                  (NULL),
  mlClasses                 (new MLClassList ()),
  log                       (_log),
  model                     (NULL),
  priorProbability          (NULL),
  report                    (NULL),
  savedModelName            (),
  statusMessage             (_statusMessage),
  subTrainingProcesses      (NULL),
  trainingExamples          (NULL),
  weOwnTrainingExamples     (true),
  weOwnMLClasses            (true)
{
  log.Level (20) << "TrainingProcess2::TrainingProcess2     Loading an existing trained model" << endl;

  KKB::DateTime  savedModelTimeStamp  = osGetFileDateTime (savedModelName);

  fileDesc = fvFactoryProducer->FileDesc ();

  bool  successful = false;

  Read (_in, successful);

  if  ((!successful)  ||  (model && (!model->ValidModel ())))
  {
    log.Level (-1) << endl << endl 
                   << "TrainingProcess2    *** ERROR ***  Training Model Save File[" << savedModelName << "] Invalid Format." << endl
                   << endl;
    Abort (true);
  }

  if  (config)
    fvFactoryProducer = config->FvFactoryProducer ();

  log.Level (20) << "TrainingProcess2::TrainingProcess2(6 parameters)   Exiting." << endl;

  statusMessage = "All Done";
  return;
}





TrainingProcess2::TrainingProcess2 (TrainingConfiguration2Const* _config, 
                                    FeatureVectorListPtr         _trainingExamples,
                                    MLClassListPtr               _mlClasses,
                                    ostream*                     _report,
                                    RunLog&                      _log,
                                    bool                         _featuresAlreadyNormalized,
                                    VolConstBool&                _cancelFlag,
                                    KKStr&                       _statusMessage
                                   )
:

    abort                     (false),
    buildDateTime             (DateTime (1900,1,1,0, 0, 0)),
    cancelFlag                (_cancelFlag),
    config                    (_config),
    configOurs                (NULL),
    configFileName            (),
    configFileNameSpecified   (),
    duplicateCount            (0),
    duplicateDataCount        (0),
    excludeList               (NULL),
    featuresAlreadyNormalized (_featuresAlreadyNormalized),
    fileDesc                  (NULL),
    fvFactoryProducer         (NULL),
    mlClasses                 (NULL),
    log                       (_log),
    model                     (NULL),
    priorProbability          (NULL),
    report                    (_report),
    savedModelName            (),
    statusMessage             (_statusMessage),
    subTrainingProcesses      (NULL),
    trainingExamples          (_trainingExamples),
    weOwnTrainingExamples     (false),
    weOwnMLClasses            (false)
{
  log.Level (20) << "TrainingProcess2::TrainingProcess2" << endl;

  fvFactoryProducer = _config->FvFactoryProducer ();

  fileDesc = fvFactoryProducer->FileDesc ();

  if  (!config)
  {
    log.Level (-1) << "TrainingProcess2  No Configuration File Specified." << endl;
    Abort (true);
  }

  else  if  (!config->FormatGood ())
  {
    log.Level (-1) << "TrainingProcess2  Format error in Configuration File["
                   << config->FileName () << "]."
                   << endl; 
    Abort (true);
  }

  else
  {
    configFileName = config->FileName ();
    configFileNameSpecified = config->ConfigFileNameSpecified ();
    if  (_mlClasses)
      mlClasses = new MLClassList (*_mlClasses);
    else
      mlClasses = config->ExtractClassList ();
    weOwnMLClasses = true;
  }
}



TrainingProcess2::~TrainingProcess2 ()
{
  log.Level (20) << "TrainingProcess2::~TrainingProcess2  for Config[" << configFileName << "]" << endl;

  try
  {
    delete  model; 
    model = NULL;
  }
  catch (...)
  {
    log.Level(-1) << "TrainingProcess2::~TrainingProcess2   Exception deleting model." << endl;
  }
  model = NULL;


  delete  subTrainingProcesses;  subTrainingProcesses = NULL;
  delete  priorProbability;      priorProbability     = NULL;

  if  (weOwnTrainingExamples)
  {
    delete  trainingExamples;
    trainingExamples  = NULL;
  }

  if  (weOwnMLClasses)
  {
    delete mlClasses;  mlClasses = NULL;
  }

  delete  configOurs;
  configOurs = NULL;
}




kkint32  TrainingProcess2::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = sizeof (TrainingProcess2)
    +  configFileName.MemoryConsumedEstimated ()
    +  configFileNameSpecified.MemoryConsumedEstimated ()
    +  savedModelName.MemoryConsumedEstimated ();

  if  (configOurs)
    memoryConsumedEstimated += configOurs->MemoryConsumedEstimated ();

  if  (weOwnTrainingExamples  &&  (trainingExamples != NULL))
    memoryConsumedEstimated += trainingExamples->MemoryConsumedEstimated ();

  if  ((mlClasses != NULL)  &&  weOwnMLClasses)
    memoryConsumedEstimated += mlClasses->MemoryConsumedEstimated ();

  if  (model)
    memoryConsumedEstimated += model->MemoryConsumedEstimated ();

  if  (priorProbability)
    memoryConsumedEstimated += priorProbability->MemoryConsumedEstimated ();

  if  (subTrainingProcesses)
    memoryConsumedEstimated += subTrainingProcesses->MemoryConsumedEstimated ();

  return  memoryConsumedEstimated;
}  /* MemoryConsumedEstimated */



ModelOldSVMPtr  TrainingProcess2::OldSVMModel ()  const
{
  if  (model->ModelType () == Model::ModelTypes::mtOldSVM)
    return dynamic_cast<ModelOldSVMPtr> (model);
  else
    return NULL;
}



KKStr  TrainingProcess2::ModelDescription  ()  const
{
  if  (model)
    return model->Description ();
  else
    return "No Model";
}



Model::ModelTypes   TrainingProcess2::ModelType ()  const
{
  if  (model)
    return model->ModelType ();
  else
    return Model::ModelTypes::mtNULL;
}



KKStr   TrainingProcess2::ModelTypeStr ()  const
{
  if  (model)
    return model->ModelTypeStr ();
  else
    return "No_Model";
}



SVMModelPtr  TrainingProcess2::Model3 ()
{
  ModelOldSVMPtr  oldSvmModel = OldSVMModel ();
  if  (oldSvmModel)
    return oldSvmModel->SvmModel ();
  else
    return NULL;
}



void  TrainingProcess2::WriteXml (ostream&  o)
{
  log.Level (20) << "TrainingProcess2::WriteXml" << endl;

  bool  successful = true;

  o << "<TrainingProcess2>" << endl;

  if  (config)
    o << "TrainingConfiguration2" << "\t" << configFileNameSpecified << "\t" << config->FactoryName () << endl;

  if  (fvFactoryProducer)
    o << "fvFactoryProducer" << "\t" << fvFactoryProducer->Name () << endl;
  
  o << "ConfigFileName"          << "\t" << configFileName                  << endl;
  o << "ConfigFileNameSpecified" << "\t" << configFileNameSpecified         << endl;

  o << "BuildDateTime"           << "\t" << buildDateTime                   << endl;
  o << "ClassList"               << "\t" << mlClasses->ToTabDelimitedStr () << endl;

  if  (model)
  {
    try
    {
      model->WriteXML (o);
    }
    catch  (KKException e)
    {
      log.Level (-1) << endl 
              << "TrainingProcess2::WriteXml   ***ERROR***   Exception SavingModel" << endl
              << "       Exception[" << e.ToString () << "]" << endl
              << endl;
    }
  }

  if  (priorProbability)
    priorProbability->WriteXML (o, "PriorProbability");

  o << "</TrainingProcess2>" << endl;

  log.Level  (20) << "TrainingProcess2::SaveResults   Exiting" << endl;
}  /* WriteXml */





void  TrainingProcess2::Read (istream&  in,
                              bool&     successful
                             )
                             
{
  log.Level (20) << "TrainingProcess2::Read" << endl;

  fvFactoryProducer = NULL;

  successful = true;
 
  if  (weOwnMLClasses)
  {
    delete  mlClasses;
    mlClasses = NULL;
  }
  
  char  buff[50000];
  memset (buff, 0, sizeof (buff));
  while  ((in.getline (buff, sizeof (buff) - 1))  &&  successful)
  {
    buff[sizeof (buff) - 1] = 0;
    KKStr  line (buff);
    line.TrimRight ();
    KKStr  lineName = line.ExtractToken2 ("\t");
    lineName.Upper ();
    lineName.TrimLeft ();
    lineName.TrimRight ();

    if  (lineName.EqualIgnoreCase ("<TrainingProcess2>"))
      continue;

    else if  (lineName.EqualIgnoreCase ("</TrainingProcess2>"))
      break;

    else if  (lineName.EqualIgnoreCase ("<SVMModel>"))
    {
      // We are looking at an old 'Save' file format.  We need to rebuild a new model and save it.
      log.Level (-1) << endl << endl 
        << "TrainingProcess2::Read   ***WARNING***   Save file is of Old Format;  it will need to be rebuilt."  << endl
        << endl;
     successful = false;
     break;
    }

    else if  (lineName == "BUILDDATETIME")
    {
      line.TrimRight (" \n\r\t");
      buildDateTime = DateTime (line);
    }

    else if  (lineName.EqualIgnoreCase ("ConfigFileName"))
    {
      configFileName = line.ExtractToken2 ("\n\t\r");
      // KKKK  LoadConfigurationFile
    }

    else if  (lineName.EqualIgnoreCase ("ConfigFileNameSpecified"))
      configFileNameSpecified = line.ExtractToken2 ("\n\t\r");


    else if  (lineName.EqualIgnoreCase ("TrainingConfiguration2"))
    {
      configFileNameSpecified = line.ExtractToken2 ("\t");
      KKStr  factoryName = line.ExtractToken2 ("\t");

      if  (!factoryName.Empty ())
      {
        fvFactoryProducer = FactoryFVProducer::LookUpFactory (factoryName);
        if  (fvFactoryProducer)
          fileDesc = fvFactoryProducer->FileDesc ();
      }

      delete  configOurs;
      configOurs = TrainingConfiguration2::Manufacture (factoryName, 
                                                        configFileNameSpecified, 
                                                        false,   //  false = DO not Validate Directories.
                                                        log
                                                       );
      if  (!configOurs)
      {
        log.Level (-1) << endl 
            << "TrainingProcess2::Read   ***ERROR**    Could not load TrainingConfiguration for Factory: " << factoryName << "  configName: " << configFileNameSpecified << endl
            << endl;
      }
      else
      {
        config = configOurs;

        if  (!fvFactoryProducer)
        {
          fvFactoryProducer = config->FvFactoryProducer ();
          fileDesc = fvFactoryProducer->FileDesc ();
        }
      }
    }
  
        
    else if  (lineName == "CLASSLIST")
    {
      log.Level (20) << "TrainingProcess2::Read    Classes[" << line << "]" << endl;

      if  (weOwnMLClasses)
        delete  mlClasses;
      mlClasses = new MLClassList ();
      weOwnMLClasses = true;
      while  (!line.Empty ())
      {
        KKStr  className = line.ExtractToken2 ("\t\n\r");
        MLClassPtr  c = MLClass::CreateNewMLClass (className);
        mlClasses->PushOnBack (c);
      }
    }

    else if  (lineName.SubStrPart (0, 13).EqualIgnoreCase ("<ClassProbList"))
    {
      delete  priorProbability;
      priorProbability = ClassProbList::CreateFromXMLStream (in);
    }

    else if  (lineName.EqualIgnoreCase ("<Model>"))
    {
      if  (!fvFactoryProducer)
      {
        fvFactoryProducer = GrayScaleImagesFVProducerFactory::Factory (&log);
        fileDesc = fvFactoryProducer->FileDesc ();
      }
    
      try
      {
        model = Model::CreateFromStream (in, fileDesc, cancelFlag, log);
        if  (model == NULL)
        {
          successful = false;
          log.Level (-1) << endl << endl
            << "TrainingProcess2::Read   ***ERROR***  Could not create model from stream." << endl
            << endl;
        }
      }
      catch  (const KKException&  e)
      {
        successful = false;
        log.Level (-1) << endl << endl
          << "TrainingProcess2::Read   ***ERROR***  Exception occurred reading from file." << endl
          << "     Exception[" << e.ToString () << "]." << endl
          << endl;
      }
      catch  (...)
      {
        successful = false;
        log.Level (-1) << endl << endl
          << "TrainingProcess2::Read   ***ERROR***  Exception occurred reading from file." << endl
          << endl;
      }
    }
  }

  if  (successful)
  {
    if  (mlClasses == NULL)
    {
      log.Level (-1) << endl
                     << endl
                     << "    **** ERROR ****" << endl
                     << endl
                     << "TrainingProcess2::Read  - ClassList were not defined." << endl
                     << endl;
      successful = false;
    }

    else
    {
      delete  configOurs;
      configOurs = new TrainingConfiguration2 (osGetRootName (configFileName), false, log);
      config = configOurs;

      if  (config->SubClassifiers () != NULL)
        LoadSubClassifiers (false,   // forceRebuild
                            true     // CheckForDuplicates
                           );
    }
  }

  log.Level (20) << "TrainingProcess2::Read    Done Reading in existing model." << endl;
}  /* Read */




void  TrainingProcess2::RemoveExcludeListFromTrainingData ()
{
  ImageFeaturesDataIndexed dataIndex (*excludeList);
  ImageFeaturesNameIndexed nameIndex (*excludeList);

  FeatureVectorList  trainingExamplesToDelete 
    (fileDesc, 
     false    /* owner = false */
    );

  FeatureVectorList::iterator idx;
  for  (idx = trainingExamples->begin ();  idx != trainingExamples->end ();  idx++)
  {
    FeatureVectorPtr  example = *idx;

    KKStr  rootImageFileName = osGetRootName (example->ExampleFileName ());
    if  (nameIndex.GetEqual (rootImageFileName) != NULL)
    {
      // Need to remove this example from training data.
      trainingExamplesToDelete.PushOnBack (example);
    }

    else if  (dataIndex.GetEqual (example))
    {
      // Need to remove this example from training data.
      trainingExamplesToDelete.PushOnBack (example);
    }
  }

  if  (trainingExamplesToDelete.QueueSize () == 0)
  {
    if  (report)
    {
      *report << endl << endl << endl
              << "    Images Being Classified Removed From Training Data"      << endl
              << "==========================================================" << endl
              << "No trainingExamples from source directory detected in training data." << endl
              << endl << endl;
    }
  }
  else
  {
    if  (report)
    {
      *report << endl << endl << endl
              << "Images Being Classified Removed From Training Data" << endl
              << "=================================================" << endl;

      trainingExamplesToDelete.PrintClassStatistics (*report);

      *report << endl;
    }

    for  (idx = trainingExamplesToDelete.begin ();  idx != trainingExamplesToDelete.end ();  idx++)
    {
      FeatureVectorPtr  example = *idx;

      if  (report)
        *report << example->ExampleFileName () << "\t" << example->MLClassName () << endl;

      trainingExamples->DeleteEntry (example);
    }
    if  (report)
      *report << endl << endl << endl;
  }

} /* RemoveExcludeListFromTrainingData */




void  TrainingProcess2::CheckForDuplicates (bool  allowDupsInSameClass)
{
  // Lets check for duplicate trainingExamples in training data.  Just to get a count, no other reason
  DuplicateImages  dupDetector (trainingExamples, log);
  duplicateCount = dupDetector.DuplicateCount ();
  duplicateDataCount = dupDetector.DuplicateDataCount ();
  if  ((allowDupsInSameClass  &&  (duplicateDataCount > 0))  ||
       (allowDupsInSameClass  &&  (duplicateCount > 0))
      )
  {
    log.Level (-1) << endl
                   << "ConstructOneVsOneModel        *** WARNING ***   Duplicates Detected in Training Data" << endl
                   << "                       DuplicateCount    [" << duplicateCount     << "]" << endl
                   << "                       DuplicateDataCount[" << duplicateDataCount << "]" << endl
                   << endl;

    if  (report)
      dupDetector.ReportDuplicates (*report);

    dupDetector.PurgeDuplicates (trainingExamples, allowDupsInSameClass, report);
  }
}  /* CheckForDuplicates */




void  TrainingProcess2::ExtractFeatures (const TrainingClassPtr  trainingClass,
                                         DateTime&               latestTimeStamp,
                                         bool&                   changesMade
                                        )
{
  for  (kkuint32 dirIdx = 0;  dirIdx < trainingClass->DirectoryCount ();  ++dirIdx)
  {
    KKStr  expDirName = trainingClass->ExpandedDirectory (config->RootDir (), dirIdx);

    log.Level (30) << "TrainingProcess2::ExtractFeatures - Extracting Features Directory["
                   << expDirName                          << "], file["
                   << trainingClass->FeatureFileName   () << "]."
                   << endl;

    KKStr  featureFileName = osMakeFullFileName (expDirName, trainingClass->FeatureFileName ());

    statusMessage = "Extracting features for class[" + trainingClass->MLClass ()->Name () + "].";

    FeatureFileIOPtr  driver = NULL;
    if  (fvFactoryProducer)
      driver = fvFactoryProducer->DefaultFeatureFileIO ();

    FeatureVectorListPtr  extractedImages = NULL; 

    try
    {
      extractedImages = driver->FeatureDataReSink 
                                  (fvFactoryProducer,
                                   expDirName,
                                   trainingClass->FeatureFileName (),
                                   trainingClass->MLClass (),
                                   true,   //  Make all entries in this directory 'trainingClass->MLClass ()'
                                   *mlClasses,
                                   cancelFlag,
                                   changesMade,
                                   latestTimeStamp,
                                   log
                                  );
    }
    catch (const std::exception&  e1)
    {
      log.Level (-1) << endl
        << "TrainingProcess2::ExtractFeatures  ***ERROR***   Exception occurred calling 'FeatureDataReSink'" << endl
        << e1.what () << endl
        << endl;
      extractedImages = NULL;
      Abort (true);
    }
    catch  (...)
    {
      log.Level (-1) << endl
        << "TrainingProcess2::ExtractFeatures  ***ERROR***   Exception occurred calling 'FeatureDataReSink'" << endl
        << endl;
      extractedImages = NULL;
      Abort (true);
    }

    if  ((extractedImages == NULL)  ||  (extractedImages->QueueSize () < 1))
    {
      log.Level (-1) << endl
                     << "ExtractFeatures    *** No Training Examples Found ***"   
                     << "                 Class    [" << trainingClass->Name () << "]"
                     << "                 Directory[" << expDirName             << "]"
                     << endl;
      Abort (true);
    }

    if  (extractedImages)
    {
      trainingExamples->AddQueue (*extractedImages);
      extractedImages->Owner (false);
      delete  extractedImages;
      extractedImages = NULL;
    }
  }

  log.Level (30) << "TrainingProcess2::ExtractFeatures  Exiting" << endl;
}  /* ExtractFeatures */




void  TrainingProcess2::ExtractTrainingClassFeatures (DateTime&  latestImageTimeStamp,
                                                      bool&      changesMadeToTrainingLibraries
                                                     )
{
  log.Level (20) << "TrainingProcess2::ExtractTraingClassFeatures - Starting." << endl;

  changesMadeToTrainingLibraries = false;

  //****************************************************************************
  // Make sure that there are no existing *.data files that we are going to use.
  // We need to do this in a separate pass because more than one entry may refer 
  // to the same Class and hence the same *.data file.


  //***********************************************************
  //  We will first extract the Raw features from each Class 

  const TrainingClassList&   trainingClasses = config->TrainingClasses ();
  TrainingClassList::const_iterator  tcIDX;

  DateTime  latestTimeStamp;
  bool      changesMadeToThisTrainingClass;

  for  (tcIDX = trainingClasses.begin ();  ((tcIDX != trainingClasses.end ())  &&  (!cancelFlag)  &&  (!Abort ()));  tcIDX++)
  {
    const TrainingClassPtr trainingClass = *tcIDX;

    log.Level (20) << "TrainingProcess2::ExtractTraingClassFeatures Starting on Class[" 
                   << trainingClass->Name () << "]."
                   << endl;

    ExtractFeatures (trainingClass, latestTimeStamp, changesMadeToThisTrainingClass);
    if  (latestTimeStamp > latestImageTimeStamp)
      latestImageTimeStamp = latestTimeStamp;

    if  (changesMadeToThisTrainingClass)
      changesMadeToTrainingLibraries = true;
  }

  // DONE Extracting Raw features from All classes.
  if  ((!Abort ())  &&  (!cancelFlag))
  {
    if  (config->NoiseTrainingClass ())
    {
      cout  << "*PHASE_MSG2* Training Class[" << config->NoiseTrainingClass ()->Name () << "]" << endl;
      ExtractFeatures (config->NoiseTrainingClass (), latestTimeStamp, changesMadeToThisTrainingClass);
      if  (latestTimeStamp > latestImageTimeStamp)
        latestImageTimeStamp = latestTimeStamp;

      if  (changesMadeToThisTrainingClass)
        changesMadeToTrainingLibraries = true;
    }
  }

  trainingExamples->RandomizeOrder ();

  log.Level (20) << "TrainingProcess2::ExtractTrainingClassFeatures   Exiting" << endl;
}  /*  ExtractTrainingClassFeatures */





void  TrainingProcess2::ReportTraningClassStatistics (ostream&  report)
{
  report << endl
         << "Training Class Statistics" << endl
         << endl;
  trainingExamples->PrintClassStatistics (report);
}  /* ReportTraningClassStatistics */




void  TrainingProcess2::AddImagesToTrainingLibray (FeatureVectorList&  trainingExamples,
                                                   FeatureVectorList&  examplesToAdd
                                                  )
{
  kkint32 idx        = 0;

  kkint32  numOfImages = examplesToAdd.QueueSize ();

  FeatureVectorPtr  example = NULL;

  for  (idx = 0;  idx < numOfImages;  idx++)
  {
    example = examplesToAdd.IdxToPtr (idx);

    if  (example->FeatureDataValid ())
    {
      trainingExamples.PushOnBack (example);
    }
    else
    {
      log.Level (-1) << endl << endl
          << "TrainingProcess2::AddImagesToTrainingLibray    ***ERROR***  " << endl
          << "        Example["  << example->ExampleFileName () << "], Class[" << example->ClassName () << "] Has Invalid Feature Data."
          << endl << endl;

      if  (report)
      {
        *report << "** ERROR **,  Image["  << example->ExampleFileName () << "], Class[" << example->ClassName () << "]"
                << "  Has Invalid Feature Data."
                << endl;
      }
    }
  }
}  /* AddImagesToTrainingLibray */



void  TrainingProcess2::CreateModelsFromTrainingData ()
{
  log.Level (20) << "TrainingProcess2::CreateModelsFromTrainingData    Starting" << endl;

  if  (config->ModelParameters () == NULL)
  {
    log.Level (-1) << endl 
                   << "TrainingProcess2::CreateModelsFromTrainingData    ***ERROR***"  << endl
                   << endl
                   << "                   Parameters not specified in configuration file." << endl
                   << endl;
    Abort (true);
    return;
  }

  delete  priorProbability;
  priorProbability = trainingExamples->GetClassDistribution ();

  // Will Create the Appropriate Model class given 'config->ModelingMethod ()'.
  model = Model::CreateAModel (config->ModelingMethod (),
                               osGetRootName (configFileName),
                               *(config->ModelParameters ()), 
                               fileDesc, 
                               cancelFlag, 
                               log
                              );

  try
  {
    model->TrainModel (trainingExamples, 
                       featuresAlreadyNormalized,
                       false     // false = 'model' We are not giving ownership of TrainingExdamples to model.
                      );
  }
  catch  (const KKException&  e)
  {
    log.Level (-1) << endl << endl
      << "TrainingProcess2::CreateModelsFromTrainingData   ***ERROR***   Exception occurred while running 'Model::TrainModel'." << endl
      << "     Exception[" << e.ToString () << endl
      << endl;
    Abort (true);
  }
  catch  (...)
  {
    log.Level (-1) << endl << endl
      << "TrainingProcess2::CreateModelsFromTrainingData   ***ERROR***   Exception occurred while running 'Model::TrainModel'." << endl
      << endl;
    Abort (true);
  }

  if  (!model->ValidModel ())
  {
    Abort (true);
  }
  else
  {
    // Need to LOad Sub Processors.
    LoadSubClassifiers (false, true);
  }

  //trainingExamples->Owner (false);
  buildDateTime = osGetLocalDateTime ();
  log.Level (20) << "TrainingProcess2::CreateModelsFromTrainingData    Ending" << endl;
} /* CreateModelsFromTrainingData */





//****************************************************************************
//*                                                                          *
//****************************************************************************
void  TrainingProcess2::SaveResults ()
{
  log.Level (20) << "TrainingProcess2::SaveResults" << endl;

  bool  successful = true;

  //KKStr  baseModelName = osRemoveExtension (configFileName) + ".Save";

  ofstream o (savedModelName.Str ());

  o << "BuildDateTime" << "\t" << buildDateTime << std::endl;
  
  o << "ClassList" << "\t" << mlClasses->ToTabDelimitedStr () << std::endl;


  if  (model)
  {
    model->RootFileName (osGetRootName (this->ConfigFileName ()));
    try
    {
      model->WriteXML (o);
    }
    catch  (KKException& e)
    {
      successful = false;
      log.Level (-1) << endl 
              << "TrainingProcess2::SaveResults   ***ERROR***   Exception was thrown calling 'WriteXML'." << endl
              << "       Exception[" << e.ToString () << "]" << endl
              << endl;
    }
    catch (...)
    {
      successful = false;
      log.Level (-1) << endl
              << "TrainingProcess2::SaveResults   ***ERROR***   Exception was thrown calling 'WriteXML'." << endl
              << endl;
    }
    if  (!successful)
    {
      log.Level (-1) << "TrainingProcess2::SaveResults - Saving Model 3 Failed." << endl;
      Abort (true);
      o.close ();
      return;
    }
  }

  o.close ();

  log.Level  (20) << "TrainingProcess2::SaveResults   Exiting" << endl;
}  /* SaveResults */



ModelParamPtr  TrainingProcess2::Parameters () const
{
  if  (!model)
    return NULL;
  else
    return model->Param ();
}  /* Parameters */




kkint32  TrainingProcess2::NumOfSupportVectors ()  const
{
  ModelOldSVMPtr  oldSvmModel = OldSVMModel ();
  if  (oldSvmModel)
    return oldSvmModel->NumOfSupportVectors ();
  else
    return 0;
}



void  TrainingProcess2::SupportVectorStatistics (kkint32&  numSVs,
                                                 kkint32&  totalNumSVs
                                                )
{
  numSVs = 0;
  totalNumSVs = 0;
  ModelOldSVMPtr  oldSvmModel = OldSVMModel ();
  if  (oldSvmModel)
    oldSvmModel->SupportVectorStatistics (numSVs, totalNumSVs);
}




TrainingProcess2Ptr   TrainingProcess2::TrainingProcessLeft ()
{
  if  ((!model)  ||  (model->ModelType () != Model::ModelTypes::mtDual))
    return NULL;
  return  dynamic_cast<ModelDualPtr>(model)->Trainer1 ();
}



TrainingProcess2Ptr   TrainingProcess2::TrainingProcessRight ()
{
  if  ((!model)  ||  (model->ModelType () != Model::ModelTypes::mtDual))
    return NULL;
  return  dynamic_cast<ModelDualPtr>(model)->Trainer2 ();
}
double   TrainingProcess2::TrainingTime ()  const
{
  if (model)
    return model->TrainingTime();
  else
    return 0.0;
}




MLClassListPtr    TrainingProcess2::ExtractFullHierachyOfClasses ()  const
{
  if  (config)
    return config->ExtractFullHierachyOfClasses ();
  
  else if  (mlClasses)
    return  new  MLClassList (*mlClasses);

  else
    return  NULL;
}




/**
 @brief If there is a config file; will return a list of its FormatErrors ().  
 */
VectorKKStr  TrainingProcess2::ConfigFileFormatErrors ()  const
{
  if  (config)
    return config->FormatErrorsWithLineNumbers ();
  else
    return VectorKKStr ();
}  /* ConfigFileFormatErrors */




void  TrainingProcess2::LoadSubClassifiers (bool  forceRebuild,
                                            bool  checkForDuplicates
                                           )
{
  if  ((config == NULL)  ||  (config->SubClassifiers () == NULL))
    return;

  delete  subTrainingProcesses;
  subTrainingProcesses = new TrainingProcess2List (true);


  TrainingConfiguration2ListPtr  subClassifiers = config->SubClassifiers ();
  TrainingConfiguration2List::const_iterator idx;
  for  (idx = subClassifiers->begin ();  idx != subClassifiers->end ();  ++idx)
  {
    TrainingConfiguration2Ptr  subClassifier = *idx;
    TrainingProcess2Ptr  tp = new TrainingProcess2 (subClassifier, 
                                                    NULL,     // ExcludeList
                                                    log,
                                                    NULL,     // reportFile
                                                    forceRebuild,
                                                    checkForDuplicates,
                                                    cancelFlag,
                                                    statusMessage
                                                   );
    subTrainingProcesses->PushOnBack (tp);
    if  (tp->Abort ())
    {
      log.Level (-1) << endl 
        << "TrainingProcess2::LoadSubClassifiers   ***ERROR***   Loading SubClassifier[" << subClassifier->ConfigRootName () << "]." << endl
        << endl;
      Abort (true);
      break;
    }
  }
}  /* LoadSubClassifiers */




TrainingProcess2List::TrainingProcess2List (bool  _owner):
   KKQueue<TrainingProcess2> (_owner)
{
}



TrainingProcess2List::~TrainingProcess2List ()
{
}



kkint32 TrainingProcess2List::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = 0;

  TrainingProcess2List::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  ++idx)
  {
    TrainingProcess2Ptr  tp = *idx;
    if  (tp)
      memoryConsumedEstimated += tp->MemoryConsumedEstimated ();
  }
  return memoryConsumedEstimated;
}



