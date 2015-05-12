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
#include "DuplicateImages.h"
#include "FactoryFVProducer.h"
#include "FeatureFileIO.h"
#include "FeatureNumList.h"
#include "FeatureVector.h"
#include "MLClass.h"
#include "Model.h"
#include "ModelOldSVM.h"
#include "ModelKnn.h"
#include "ModelSvmBase.h"
#include "TrainingClass.h"
#include "TrainingConfiguration2.h"
using namespace  KKMachineLearning;



TrainingProcess2::TrainingProcess2 (const KKStr&          _configFileName,
                                    FeatureVectorListPtr  _excludeList,
                                    FactoryFVProducerPtr  _fvFactoryProducer,
                                    RunLog&               _log,
                                    ostream*              _report,
                                    bool                  _forceRebuild,
                                    bool                  _checkForDuplicates,
                                    VolConstBool&         _cancelFlag,
                                    KKStr&                _statusMessage
                                   ):
  abort                     (false),
  buildDateTime             (DateTime (1900, 1, 1, 0, 0, 0)),
  cancelFlag                (_cancelFlag),
  config                    (NULL),
  configFileName            (_configFileName),
  configFileNameSpecified   (_configFileName),
  duplicateDataCount        (0),
  excludeList               (_excludeList),
  featuresAlreadyNormalized (false),
  fileDesc                  (NULL),
  fvFactoryProducer         (_fvFactoryProducer),
  mlClasses                 (NULL),
  log                       (_log),
  model                     (NULL),
  report                    (_report),
  savedModelName            (),
  statusMessage             (_statusMessage),
  trainingExamples          (NULL),
  weOwnImageClassesConfig   (true),
  weOwnTrainingExamples     (true)

{
  log.Level (10) << "TrainingProcess2::TrainingProcess2   9 Parameters." << endl;

  fileDesc = fvFactoryProducer->FileDesc ();

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

  config = new TrainingConfiguration2 (configFileName,
                                       fvFactoryProducer,
                                       true,   // true = Validate Directories
                                       log
                                      );
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

  mlClasses = config->ExtractClassList ();
  mlClasses->SortByName ();

  configTimeStamp = osGetFileDateTime (configFileName);
  if  (configTimeStamp > savedModelTimeStamp)
    useExistingSavedModel = false;

  cout  << "*PHASE_MSG* Extracting Training Class Data" << endl;

//  PostLarvaeFVResetDarkSpotCounts ();

  ExtractTrainingClassFeatures (latestTrainingImageTimeStamp, changesMadeToTrainingLibrary);
  if  (cancelFlag)
    Abort (true);
  //{
  //  ofstream o ("C:\\Temp\\DarkSpots.txt");
  //  PostLarvaeFVPrintReport (o);
  //  o.close ();
  //}


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

    buildDateTime = savedModelTimeStamp;  // Set to timestamp of save file for now.  Will be overwriiting by the
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
      bool  successful = false;
      Read (in, successful);
      if  (!successful)
      {
        log.Level (-1) << endl << endl << endl
                       << "TrainingProcess2      *** Invalid Format in SaveFile[" << savedModelName << "]  we will have to rebuild model." << endl
                       << endl;

        useExistingSavedModel = false;
        delete  model;
        model = NULL;
        // Since we have to build the model anyway we need to reload th etrainingExamples.
        delete  config;              config             = NULL;
        delete  trainingExamples;    trainingExamples   = NULL;
        delete  mlClasses;        mlClasses       = NULL;

        mlClasses = new MLClassList ();
        trainingExamples = fvFactoryProducer->ManufacturFeatureVectorList (true,  log);

        config = new  TrainingConfiguration2 (configFileName,
                                              fvFactoryProducer,
                                              true,   // true = Validate Directories.
                                              log
                                             );

        if  (!config->FormatGood ())
        {
          log.Level (-1) << "TrainingProcess2  Invalid Configuration File Specified." << endl;
          Abort (true);
          return;
        }

        mlClasses = config->ExtractClassList ();
        mlClasses->SortByName ();

        ExtractTrainingClassFeatures (latestTrainingImageTimeStamp, changesMadeToTrainingLibrary);
        if  (cancelFlag)
          Abort (true);
      }

      in.close ();
    }
  }

  if  (!useExistingSavedModel)
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
TrainingProcess2::TrainingProcess2 (const KKStr&         _configFileName,
                                    FeatureVectorListPtr _excludeList,
                                    FactoryFVProducerPtr _fvFactoryProducer,
                                    RunLog&              _log,
                                    kkuint32             _level,
                                    VolConstBool&        _cancelFlag,
                                    KKStr&               _statusMessage
                                   ):

  abort                     (false),
  buildDateTime             (DateTime (1900,1,1,0, 0, 0)),
  cancelFlag                (_cancelFlag),
  config                    (NULL),
  configFileName            (_configFileName),
  configFileNameSpecified   (_configFileName),
  duplicateDataCount        (0),
  excludeList               (_excludeList),
  featuresAlreadyNormalized (false),
  fvFactoryProducer         (_fvFactoryProducer),
  fileDesc                  (NULL),
  mlClasses                 (NULL),
  log                       (_log),
  model                     (NULL),
  report                    (NULL),
  savedModelName            (),
  statusMessage             (_statusMessage),
  trainingExamples          (NULL),
  weOwnImageClassesConfig   (true),
  weOwnTrainingExamples     (true)
{
  log.Level (20) << "TrainingProcess2::TrainingProcess2" << endl;

  fileDesc = fvFactoryProducer->FileDesc ();

  configFileName = TrainingConfiguration2::GetEffectiveConfigFileName (configFileName);

  config = new  TrainingConfiguration2 (configFileName,
                                        fvFactoryProducer,
                                        true,   // true = Validate Directories
                                        log
                                       );
  if  (!config->FormatGood ())
  {
    statusMessage = "Configuration file is invalid.";
    log.Level (-1) << "TrainingProcess2  Invalid Configuration File Specified." << endl;
    Abort (true);
    return;
  }

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
    // We need to build data that is specific to the spcified _level.
    FeatureVectorListPtr  newExamples = trainingExamples->ExtractExamplesForHierarchyLevel (_level);
    delete  trainingExamples;
    trainingExamples = newExamples;
    newExamples = NULL;

    TrainingConfiguration2Ptr  newConfig = config->GenerateAConfiguraionForAHierarchialLevel (_level);
    delete  config;
    config = newConfig;
    newConfig = NULL;

    delete  mlClasses;
    mlClasses = config->ExtractClassList ();
    mlClasses->SortByName ();
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
                                    FactoryFVProducerPtr _fvFactoryProducer,
                                    RunLog&              _log,
                                    bool                 _featuresAlreadyNormalized,
                                    VolConstBool&        _cancelFlag,
                                    KKStr&               _statusMessage
                                   ):

  abort                     (false),
  buildDateTime             (DateTime (1900,1,1,0, 0, 0)),
  cancelFlag                (_cancelFlag),
  config                    (NULL),
  configFileName            (_configFileName),
  configFileNameSpecified   (_configFileName),
  duplicateDataCount        (0),
  excludeList               (NULL),
  featuresAlreadyNormalized (_featuresAlreadyNormalized),
  fileDesc                  (NULL),
  fvFactoryProducer         (_fvFactoryProducer),
  mlClasses                 (new MLClassList ()),
  log                       (_log),
  model                     (NULL),
  report                    (NULL),
  savedModelName            (),
  statusMessage             (_statusMessage),
  trainingExamples          (NULL),
  weOwnImageClassesConfig   (true),
  weOwnTrainingExamples     (true)

{
  log.Level (20) << "TrainingProcess2::TrainingProcess2     Loading an existing trained model" << endl;

  fileDesc = fvFactoryProducer->FileDesc ();

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

  log.Level (20) << "TrainingProcess2::TrainingProcess2(6 parms)   Exiting." << endl;

  statusMessage = "All Done";
  return;
}






TrainingProcess2::TrainingProcess2 (TrainingConfiguration2Ptr _config, 
                                    FeatureVectorListPtr      _trainingExamples,
                                    MLClassListPtr            _mlClasses,
                                    ostream*                  _report,
                                    FactoryFVProducerPtr      _fvFactoryProducer,
                                    RunLog&                   _log,
                                    bool                      _featuresAlreadyNormalized,
                                    VolConstBool&             _cancelFlag,
                                    KKStr&                    _statusMessage
                                   )
:

    abort                     (false),
    buildDateTime             (DateTime (1900,1,1,0, 0, 0)),
    cancelFlag                (_cancelFlag),
    config                    (_config),
    configFileName            (),
    configFileNameSpecified   (),
    duplicateDataCount        (0),
    excludeList               (NULL),
    featuresAlreadyNormalized (_featuresAlreadyNormalized),
    fileDesc                  (NULL),
    fvFactoryProducer         (_fvFactoryProducer),
    mlClasses                 (_mlClasses),
    log                       (_log),
    model                     (NULL),
    report                    (_report),
    savedModelName            (),
    statusMessage             (_statusMessage),
    trainingExamples          (_trainingExamples),
    weOwnImageClassesConfig   (false),
    weOwnTrainingExamples     (false)

{
  log.Level (20) << "TrainingProcess2::TrainingProcess2" << endl;

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
}



TrainingProcess2::~TrainingProcess2 ()
{
  log.Level (20) << "TrainingProcess2::~TrainingProcess2  for Config[" << configFileName << "]" << endl;

  try
  {
    delete  model; 
  }
  catch (...)
  {
    log.Level(-1) << "TrainingProcess2::~TrainingProcess2   Exception deleting model." << endl;
  }
  model = NULL;

  if  (weOwnTrainingExamples)
  {
    delete  trainingExamples;
    trainingExamples  = NULL;
  }

  if  (weOwnImageClassesConfig)
  {
    delete config;        config       = NULL;
    delete mlClasses;  mlClasses = NULL;
  }
}



kkint32  TrainingProcess2::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = sizeof (TrainingProcess2)
    +  configFileName.MemoryConsumedEstimated ()
    +  configFileNameSpecified.MemoryConsumedEstimated ()
    +  savedModelName.MemoryConsumedEstimated ();

  if  (config)
    memoryConsumedEstimated += config->MemoryConsumedEstimated ();

  if  (weOwnTrainingExamples  &&  (trainingExamples != NULL))
    memoryConsumedEstimated += trainingExamples->MemoryConsumedEstimated ();

  if  (mlClasses)
    memoryConsumedEstimated += mlClasses->MemoryConsumedEstimated ();

  if  (model)
    memoryConsumedEstimated += model->MemoryConsumedEstimated ();

  return  memoryConsumedEstimated;
}  /* MemoryConsumedEstimated */



ModelOldSVMPtr  TrainingProcess2::OldSVMModel ()  const
{
  if  (model->ModelType () == Model::mtOldSVM)
    return dynamic_cast<ModelOldSVMPtr> (model);
  else
    return NULL;
}



SVMModelPtr  TrainingProcess2::Model3 ()
{
  ModelOldSVMPtr  oldSvmModel = OldSVMModel ();
  if  (oldSvmModel)
    return oldSvmModel->SvmModel ();
  else
    return NULL;
}



void  TrainingProcess2::Read (istream&  in,
                              bool&     successful
                             )

{
  log.Level (20) << "TrainingProcess2::Read" << endl;

  successful = true;
 
  delete  mlClasses;
  mlClasses = NULL;
  
  char  buff[50000];


  KKStr::MemSet (buff, 0, sizeof (buff));
  while  (in.getline (buff, sizeof (buff) - 1))
  {
    buff[sizeof (buff) - 1] = 0;
    KKStr  line (buff);
    line.TrimRight ();
    KKStr  lineName = line.ExtractToken2 ("\t");
    lineName.Upper ();
    lineName.TrimLeft ();
    lineName.TrimRight ();

    if  (lineName.EqualIgnoreCase ("<SVMModel>"))
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

    else if  (lineName == "CLASSLIST")
    {
      log.Level (20) << "TrainingProcess2::Read    Classes[" << line << "]" << endl;
      delete  mlClasses;

      mlClasses = new MLClassList ();
      while  (!line.Empty ())
      {
        KKStr  className = line.ExtractToken2 ("\t\n\r");
        MLClassPtr  c = MLClass::CreateNewMLClass (className);
        mlClasses->PushOnBack (c);
      }
    }

    else if  (lineName.EqualIgnoreCase ("<Model>"))
    {
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
          << "TrainingProcess2::Read   ***ERROR***  Exception occurecd reading from file." << endl
          << "     Exception[" << e.ToString () << "]." << endl
          << endl;
      }
      catch  (...)
      {
        successful = false;
        log.Level (-1) << endl << endl
          << "TrainingProcess2::Read   ***ERROR***  Exception occurecd reading from file." << endl
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
  }

  log.Level (20) << "TrainingProcess2::Read    Done Reading in existing model." << endl;
}  /* read */




void  TrainingProcess2::RemoveExcludeListFromTrainingData ()
{
  ImageFeaturesDataIndexed dataIndex (*excludeList);
  ImageFeaturesNameIndexed nameIndex (*excludeList);

  FeatureVectorList  trainingExamplesToDelete 
    (fileDesc, 
     false,    /* owner = false */
     log
    );

  FeatureVectorList::iterator idx;
  for  (idx = trainingExamples->begin ();  idx != trainingExamples->end ();  idx++)
  {
    FeatureVectorPtr  example = *idx;

    KKStr  rootImageFileName = osGetRootName (example->ImageFileName ());
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
              << "    Images Being Classifed Removed From Training Data"      << endl
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
              << "Images Being Classifed Removed From Training Data" << endl
              << "=================================================" << endl;

      trainingExamplesToDelete.PrintClassStatistics (*report);

      *report << endl;
    }

    for  (idx = trainingExamplesToDelete.begin ();  idx != trainingExamplesToDelete.end ();  idx++)
    {
      FeatureVectorPtr  example = *idx;

      if  (report)
        *report << example->ImageFileName () << "\t" << example->ImageClassName () << endl;

      trainingExamples->DeleteEntry (example);
    }
    if  (report)
      *report << endl << endl << endl;
  }

} /* RemoveExcludeListFromTrainingData */




void  TrainingProcess2::CheckForDuplicates (bool  allowDupsInSameClass)
{
  // Lets check for duplicate trainingExamples in training data.  Just to get a count, no other reason
  DuplicateImages  dupDetector (trainingExamples, fileDesc, log);
  duplicateDataCount = dupDetector.DuplicateDataCount ();
  if  (duplicateDataCount > 0)
  {
    log.Level (-1) << endl << endl
                   << "ConstructOneVsOneModel        *** WARNING ***   Dupliactes Detected in Training Data" << endl
                   << "                              DuplicateCount[" << duplicateDataCount << "]" << endl
                   << endl;

    if  (report)
      dupDetector.ReportDuplicates (*report);

    dupDetector.PurgeDuplicates (NULL, allowDupsInSameClass);
  }
}  /* CheckForDuplicates */




void  TrainingProcess2::ExtractFeatures (const TrainingClassPtr  trainingClass,
                                         DateTime&               latestTimeStamp,
                                         bool&                   changesMade
                                        )
{
  KKStr  expDirName = trainingClass->ExpandedDirectory (config->RootDirExpanded ());

  log.Level (30) << "TrainingProcess2::ExtractFeatures - Extracting Features from Directory["
                 << expDirName                          << "],  into file["
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
    //  We will use list from ReSink  to add to trainingExamples
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
  // We need to do this in a seperate pass because more than one entry may refer 
  // to the same Class and hense the same *.data file.


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
      cout  << "*PHASE_MSG2* Training Class[" << config->NoiseTrainingClass ()->Directory () << "]" << endl;
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




void  TrainingProcess2::AddImagesToTrainingLibray (FeatureVectorList&  trainingImages,
                                                   FeatureVectorList&  imagesToAdd
                                                  )
{
  kkint32 idx        = 0;

  kkint32  numOfImages = imagesToAdd.QueueSize ();

  FeatureVectorPtr  example = NULL;

  for  (idx = 0;  idx < numOfImages;  idx++)
  {
    example = imagesToAdd.IdxToPtr (idx);

    if  (example->FeatureDataValid ())
    {
      trainingImages.PushOnBack (example);
    }
    else
    {
      log.Level (-1) << endl << endl
          << "TrainingProcess2::AddImagesToTrainingLibray    ***ERROR***,  Image["  << example->ImageFileName () << "], Class[" << example->ClassName () << "]"
          << "  Has Invalid Feature Data."
          << endl << endl;

      if  (report)
      {
        *report << "** ERROR **,  Image["  << example->ImageFileName () << "], Class[" << example->ClassName () << "]"
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
      << "TrainingProcess2::CreateModelsFromTrainingData   ***ERROR***   Exception occurecd while running 'Model::TrainModel'." << endl
      << "     Exception[" << e.ToString () << endl
      << endl;
    Abort (true);
  }
  catch  (...)
  {
    log.Level (-1) << endl << endl
      << "TrainingProcess2::CreateModelsFromTrainingData   ***ERROR***   Exception occurecd while running 'Model::TrainModel'." << endl
      << endl;
    Abort (true);
  }

  if  (!model->ValidModel ())
  {
    Abort (true);
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


double   TrainingProcess2::TrainingTime ()  const
{
  if (model)
    return model->TrainingTime();
  else
    return 0.0;
}



kkint32  TrainingProcess2::DuplicateDataCount () const
{
  return  duplicateDataCount;
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

