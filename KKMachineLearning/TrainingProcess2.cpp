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

#include "GlobalGoalKeeper.h"
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



TrainingProcess2Ptr  TrainingProcess2::LoadExistingTrainingProcess (const KKStr&   configRootName,
                                                                    VolConstBool&  cancelFlag,
                                                                    RunLog&        log
                                                                   )
{
  TrainingProcess2Ptr  trainer = NULL;

  KKStr  configFileFullName = TrainingConfiguration2::GetEffectiveConfigFileName (configRootName);

  KKStr  savedModelName = osRemoveExtension (configFileFullName) + ".Save";
  if  (!osFileExists (savedModelName))
  {
    log.Level (-1) << endl
      << "TrainingProcess2::LoadExistingTrainingProcess   ***ERROR***    SaveFile[" << savedModelName << "]  does not exist." << endl
      << endl;
    return NULL;
  }

  XmlStreamPtr  stream = new XmlStream (savedModelName, log);
  XmlTokenPtr  t = stream->GetNextToken (log);
  while  (t  &&  (typeid (*t)  !=  typeid (XmlElementTrainingProcess2)))
  {
    delete  t;
    t = stream->GetNextToken (log);
  }

  if  (t)
    trainer = dynamic_cast<XmlElementTrainingProcess2Ptr> (t)->TakeOwnership ();

  delete  stream;  stream = NULL;
  delete  t;       t      = NULL;

  return  trainer;
}  /* LoadExistingTrainingProcess */




TrainingProcess2Ptr  TrainingProcess2::CreateTrainingProcess 
                                   (TrainingConfiguration2Const*  config,
                                    bool                          checkForDuplicates,
                                    WhenToRebuild                 whenToRebuild,
                                    bool                          saveTrainedModel,
                                    VolConstBool&                 cancelFlag,
                                    RunLog&                       log
                                   )
{
  log.Level (10) << "TrainingProcess2::CreateTrainingProcess." << endl;

  bool  weShouldRebuild = false;
  TrainingProcess2Ptr trainer = NULL;

  if  (whenToRebuild == TrainingProcess2::WhenToRebuild::NeverRebuild)
  {
    trainer = LoadExistingTrainingProcess (config->ConfigRootName (), cancelFlag, log);
    if  (!trainer)
    {
      log.Level (-1) << endl
        << "TrainingProcess2::CreateTrainingProcess   ***ERROR***   Could not load existing save model." << endl
        << endl;
    }
    else if  (trainer->Abort ())
    {
      log.Level (-1) << endl
        << "TrainingProcess2::CreateTrainingProcess   ***ERROR***   Existing Save model is Invalid." << endl
        << endl;
      delete  trainer;
      trainer = NULL;
    }

    return trainer;
  }

  FeatureVectorListPtr  trainingExamples = NULL;
  KKStr  configRootName = config->ConfigRootName ();
  KKStr  configFileFullName = TrainingConfiguration2::GetEffectiveConfigFileName (configRootName);
  DateTime  configFileTimeStamp = KKB::osGetFileDateTime (configFileFullName);
  DateTime  latestTrainingImageTimeStamp;
  bool      changesMadeToTrainingLibrary = true;    

  if  (whenToRebuild == TrainingProcess2::WhenToRebuild::AlwaysRebuild)
  {
    trainingExamples = ExtractTrainingClassFeatures (config, latestTrainingImageTimeStamp, changesMadeToTrainingLibrary, cancelFlag, log);
    if  (!trainingExamples)
    {
      log.Level (-1) << endl
        << "TrainingProcess2::CreateTrainingProcess   ***ERROR***  Failed to load training data." << endl
        << endl;
      return NULL;
    }

    if  (cancelFlag)
    {
      delete trainingExamples;
      trainingExamples = NULL;
    }
    else
    {
      trainer = new TrainingProcess2 ();
      trainer->BuildTrainingProcess (config,
                                     whenToRebuild,
                                     trainingExamples,
                                     true,                 /**<  true = Take ownership of 'trainingExamples'. */
                                     checkForDuplicates,
                                     cancelFlag,
                                     log
                                    );
      if  (saveTrainedModel  &&   (!cancelFlag)  &&  (!trainer->Abort ()))
        trainer->SaveTrainingProcess (log);
    }

    trainingExamples = NULL;

    return trainer;
  }

  trainer = LoadExistingTrainingProcess (config->ConfigRootName (), cancelFlag, log);
  if  ((trainer)  &&  (trainer->Abort ()))
  {
    delete  trainer;
    trainer = NULL;
  }

  if  (trainer  &&  (whenToRebuild == TrainingProcess2::WhenToRebuild::NotValid))
  {
    // Model is valid and 'whenToRebuild' specifies to use existing model as long as valid.
    log.Level (20) << "CreateTrainingProcess   Existing model[" << configFileFullName << "] Valid!" << endl;
    return  trainer;
  }

  bool  rebuildTrainingProcess = false;
  if  (!trainer)
    rebuildTrainingProcess = true;

  else if  ((trainer->BuildDateTime () < configFileTimeStamp)  ||
            (trainer->BuildDateTime () < latestTrainingImageTimeStamp)
           )
  {
    log.Level (-1) << endl
      << "TrainingProcess2::CreateTrainingProcess   ***WARNING***  Saved model was not up to date;  we will rebuild." << endl
      << endl;
    rebuildTrainingProcess = true;
  }

  if  (rebuildTrainingProcess)
  {
    delete trainer;
    trainer = NULL;
    trainingExamples = ExtractTrainingClassFeatures (config, latestTrainingImageTimeStamp, changesMadeToTrainingLibrary, cancelFlag, log);
    if  (!trainingExamples)
    {
      log.Level (-1) << endl
        << "TrainingProcess2::CreateTrainingProcess   ***ERROR***  Failed to load training data." << endl
        << endl;
      return NULL;
    }

    if  (cancelFlag)
    {
      log.Level (-1) << "TrainingProcess2::CreateTrainingProcess   Canceled !!!" << endl;
      delete  trainingExamples;
      trainingExamples = NULL;
      return NULL;
    }

    trainer = new TrainingProcess2 ();
    trainer->BuildTrainingProcess (config,
                                   whenToRebuild,
                                   trainingExamples,
                                   true,                 /**<  true = Take ownership of 'trainingExamples'. */
                                   checkForDuplicates,
                                   cancelFlag,
                                   log
                                  );
    if  (saveTrainedModel  &&   (!cancelFlag)  &&  (!trainer->Abort ()))
      trainer->SaveTrainingProcess (log);

    // Since 'trainer' now has ownership to 'trainingExamples' we will set 'trainingExamples' to NULL
    trainingExamples = NULL;
    return trainer;
  }

  else
  {
    // The model that we loaded is valid and up-to-date; we can use as is and discard the training data we just loaded.
    delete  trainingExamples;
    trainingExamples = NULL;

    log.Level (10) << "TrainingProcess2::CreateTrainingProcess    Existing saved model["  << trainer->ConfigFileName () << "]." << endl;
    return  trainer;
  }
} /* CreateTrainingProcess */




TrainingProcess2Ptr  TrainingProcess2::CreateTrainingProcessForLevel (TrainingConfiguration2Const*  config,
                                                                      kkuint32                      level,
                                                                      VolConstBool&                 cancelFlag,
                                                                      RunLog&                       log
                                                                     )
{
  log.Level (20) << "TrainingProcess2::CreateTrainingProcessForLevel" << endl;
  if  (config == NULL)
  {
    KKStr errMsg = "CreateTrainingProcessForLevel  ***ERROR***   Configuration file was not provided.";
    log.Level (-1) << endl << errMsg << endl << endl;
    return  NULL;
  }

  DateTime  latestTrainingImageTimeStamp;
  bool  changesMadeToTrainingLibrary = false;
  FactoryFVProducerPtr  fvFactoryProducer = config->FvFactoryProducer (log);

  FeatureVectorListPtr  trainingExamples 
    = ExtractTrainingClassFeatures (config, latestTrainingImageTimeStamp, changesMadeToTrainingLibrary, cancelFlag, log);
  if  (!trainingExamples)
  {
    log.Level (-1) << endl
      << "TrainingProcess2::CreateTrainingProcessForLevel   ***ERROR***   Failed to load training data." << endl
      << endl;
    return NULL;
  }


  {
    DuplicateImagesPtr  dupDetector = new DuplicateImages (trainingExamples->FileDesc (), log);
    dupDetector->PurgeDuplicates (trainingExamples, true, NULL);
    delete  dupDetector;
    dupDetector = NULL;
  }
    
  MLClassListPtr  mlClasses = config->ExtractClassList ();

  {
    // We need to build data that is specific to the specified _level.
    FeatureVectorListPtr  newExamples = trainingExamples->ExtractExamplesForHierarchyLevel (level);
    delete  trainingExamples;
    trainingExamples = newExamples;
    newExamples = NULL;
  }

  TrainingConfiguration2Ptr  levelSpecificConfig = config->GenerateAConfiguraionForAHierarchialLevel (level, log);

  TrainingProcess2Ptr  trainer = new TrainingProcess2 ();
  trainer->BuildTrainingProcess (levelSpecificConfig,
                                 WhenToRebuild::AlwaysRebuild,
                                 trainingExamples, 
                                 true,        /**<  true = Take ownership of 'trainingExamples' */
                                 false,
                                 cancelFlag,
                                 log
                                );

  delete  levelSpecificConfig;
  levelSpecificConfig = NULL;
  trainingExamples = NULL;

  return  trainer;
}  /* CreateTrainingProcessForLevel */




TrainingProcess2Ptr  TrainingProcess2::CreateTrainingProcessForLevel (const KKStr&   configFileName,
                                                                      kkuint32       level,
                                                                      VolConstBool&  cancelFlag,
                                                                      RunLog&        log
                                                                     )
{
  log.Level (20) << "TrainingProcess2::CreateTrainingProcessForLevel  configFileName: " << configFileName << endl;

  TrainingProcess2Ptr trainer = NULL;

  TrainingConfiguration2Ptr  config = new TrainingConfiguration2 ();
  config->Load (configFileName, true, log);
  if  (!config->FormatGood ())
  {
    log.Level (-1) << endl 
      << "TrainingProcess2::CreateTrainingProcessForLevel   ***ERROR***    Config File[" << configFileName << "]  Format is invalid." << endl
      << endl;
  }
  else
  {
    trainer = CreateTrainingProcessForLevel (config, level, cancelFlag, log);
  }
  delete  config;
  config = NULL;

  return  trainer;
}  /* CreateTrainingProcessForLevel */








TrainingProcess2Ptr  TrainingProcess2::CreateTrainingProcessFromTrainingExamples 
                                (TrainingConfiguration2Const* config, 
                                 FeatureVectorListPtr         trainingExamples,
                                 bool                         takeOwnershipOfTrainingExamples,
                                 bool                         featuresAlreadyNormalized,
                                 VolConstBool&                cancelFlag,
                                 RunLog&                      log
                                )
{
  log.Level (20) << "TrainingProcess2::TrainingProcess2" << endl;

  if  (!config)
  {
    log.Level (-1) << endl
      << "TrainingProcess2::CreateTrainingProcessFromTrainingExamples   ***ERROR***   No Configuration File Specified." << endl
      << endl;
    return NULL;
  }

  else  if  (!config->FormatGood ())
  {
    log.Level (-1) << endl
      <<  "TrainingProcess2::CreateTrainingProcessFromTrainingExamples   ***ERROR***   Format error in Configuration File[" << config->FileName () << "]."  << endl
      << endl;
    return NULL;
  }

  else
  {
    TrainingProcess2Ptr  trainer = new TrainingProcess2 ();
    trainer->FeaturesAlreadyNormalized (featuresAlreadyNormalized);
    trainer->BuildTrainingProcess (config,
                                   WhenToRebuild::AlwaysRebuild,
                                   trainingExamples, 
                                   takeOwnershipOfTrainingExamples, 
                                   true, 
                                   cancelFlag, 
                                   log
                                  );
    return  trainer;
  }
}  /* CreateTrainingProcessFromTrainingExamples */








//*****************************************************************************************
//*   Will build a new model from scratch for the specified class level. Will also remove *
//*   duplicates.                                                                         *
//*****************************************************************************************
TrainingProcess2::TrainingProcess2 ():

  abort                     (false),
  buildDateTime             (DateTime (1900,1,1,0, 0, 0)),
  config                    (NULL),
  configOurs                (NULL),
  configFileName            (""),
  configFileNameSpecified   (""),
  duplicateCount            (0),
  duplicateDataCount        (0),
  featuresAlreadyNormalized (false),
  fileDesc                  (NULL),
  fvFactoryProducer         (NULL),
  mlClasses                 (NULL),
  model                     (NULL),
  priorProbability          (NULL),
  report                    (NULL),
  savedModelName            (),
  subTrainingProcesses      (NULL),
  trainingExamples          (NULL),
  weOwnTrainingExamples     (true),
  weOwnMLClasses            (true)
{
}





TrainingProcess2::~TrainingProcess2 ()
{
  cout << "TrainingProcess2::~TrainingProcess2  for Config[" << configFileName << "]" << endl;

  try
  {
    delete  model; 
    model = NULL;
  }
  catch (...)
  {
    cerr  << "TrainingProcess2::~TrainingProcess2   ***ERROR***    Exception deleting model." << endl;
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








void  TrainingProcess2::BuildTrainingProcess (TrainingConfiguration2Const*  _config,
                                              WhenToRebuild                 _whenToRebuild,
                                              FeatureVectorListPtr          _trainingExamples,
                                              bool                          _takeOwnerShipOfTrainingExamples,
                                              bool                          _checkForDuplicates,
                                              VolConstBool&                 _cancelFlag,
                                              RunLog&                       _log
                                             )
{
  if  (_cancelFlag)
    return;

  _log.Level (20) << "TrainingProcess2   Building Support Vector Machine";

  if  (weOwnTrainingExamples)
  {
    delete  trainingExamples;
    trainingExamples = NULL;
  }

  configOurs = _config->Duplicate ();
  config = configOurs;

  configFileName = config->ConfigFileNameSpecified ();

  fvFactoryProducer = _config->FvFactoryProducer (_log);

  trainingExamples = _trainingExamples;
  weOwnTrainingExamples = _takeOwnerShipOfTrainingExamples;

  if  (_checkForDuplicates)
    CheckForDuplicates (true, _log);

  // trainer->ReportTraningClassStatistics (*report);
  CreateModelsFromTrainingData (_whenToRebuild,   
                                _cancelFlag, 
                                _log
                               );
  if  (Abort ())
  {
    _log.Level (-1) << endl
       << "TrainingProcess2::BuildTrainingProcess    ***ERROR***   Aborted During Model Creation ***" << endl
       << endl;
    return;
  }
}  /* BuildTrainingProcess */




void  TrainingProcess2::SaveTrainingProcess (RunLog&  log)
{
  configFileName = TrainingConfiguration2::GetEffectiveConfigFileName (config->ConfigFileNameSpecified ());
  savedModelName = osRemoveExtension (configFileName) + ".Save";
  log.Level (20) << "TrainingProcess2::SaveTrainingProcess  Saving trained model: " << savedModelName << endl;

  ofstream f (savedModelName.Str ());
  if  (!f.is_open ())
  {
    log.Level (-1) << endl
      << "TrainingProcess2::SaveTrainingProcess   ***ERROR***   Could not open SavedModelName[" << savedModelName << "]." << endl
      << endl;
    return;
  }

  this->WriteXML ("TrainingProcess2", f);

  f.close ();
}  /* SaveTrainingProcess */





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






void  TrainingProcess2::CheckForDuplicates (bool     allowDupsInSameClass,
                                            RunLog&  log
                                           )
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




FeatureVectorListPtr  TrainingProcess2::ExtractFeatures (TrainingConfiguration2ConstPtr  config,
                                                         MLClassList&                    mlClasses,
                                                         const TrainingClassPtr          trainingClass,
                                                         DateTime&                       latestTimeStamp,
                                                         bool&                           changesMade,
                                                         VolConstBool&                   cancelFlag,
                                                         RunLog&                         log
                                                        )
{
  log.Level (10) << "TrainingProcess2::ExtractFeatures   ClassName: " << trainingClass->Name () << endl;
  FactoryFVProducerPtr  fvFactoryProducer = config->FvFactoryProducer (log);

  FeatureFileIOPtr      driver           = fvFactoryProducer->DefaultFeatureFileIO ();
  FeatureVectorListPtr  trainingExamples = fvFactoryProducer->ManufacturFeatureVectorList (true, log);

  bool  abort = false;

  for  (kkuint32 dirIdx = 0;  dirIdx < trainingClass->DirectoryCount ();  ++dirIdx)
  {
    KKStr  expDirName = trainingClass->ExpandedDirectory (config->RootDir (), dirIdx);

    log.Level (30) << "TrainingProcess2::ExtractFeatures - Extracting Features Directory[" << expDirName << "], file[" << trainingClass->FeatureFileName () << "]."  << endl;

    KKStr  featureFileName = osMakeFullFileName (expDirName, trainingClass->FeatureFileName ());

    FeatureVectorListPtr  extractedExamples = NULL; 

    try
    {
      extractedExamples = driver->FeatureDataReSink 
                                  (fvFactoryProducer,
                                   expDirName,
                                   trainingClass->FeatureFileName (),
                                   trainingClass->MLClass (),
                                   true,   //  Make all entries in this directory 'trainingClass->MLClass ()'
                                   mlClasses,
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
      extractedExamples = NULL;
      abort = true;
    }
    catch  (...)
    {
      log.Level (-1) << endl
        << "TrainingProcess2::ExtractFeatures  ***ERROR***   Exception occurred calling 'FeatureDataReSink'" << endl
        << endl;
      extractedExamples = NULL;
      abort = true;
    }

    if  ((extractedExamples == NULL)  ||  (extractedExamples->QueueSize () < 1))
    {
      log.Level (-1) << endl
                     << "ExtractFeatures    *** No Training Examples Found ***"   
                     << "                 Class    [" << trainingClass->Name () << "]"
                     << "                 Directory[" << expDirName             << "]"
                     << endl;
      abort = true;
    }

    if  (extractedExamples)
    {
      trainingExamples->AddQueue (*extractedExamples);
      extractedExamples->Owner (false);
      delete  extractedExamples;
      extractedExamples = NULL;
    }
  }

  log.Level (30) << "TrainingProcess2::ExtractFeatures  Exiting" << endl;

  if  (abort)
  {
    delete  trainingExamples;
    trainingExamples = NULL;
  }

  return  trainingExamples;
}  /* ExtractFeatures */




FeatureVectorListPtr  TrainingProcess2::ExtractTrainingClassFeatures (TrainingConfiguration2ConstPtr  config,
                                                                      DateTime&                       latestImageTimeStamp,
                                                                      bool&                           changesMadeToTrainingLibraries,
                                                                      VolConstBool&                   cancelFlag,
                                                                      RunLog&                         log
                                                                     )
{
  log.Level (10) << "TrainingProcess2::ExtractTrainingClassFeatures - Starting." << endl;

  changesMadeToTrainingLibraries = false;
  bool   abort = false;

  FactoryFVProducerPtr  fvFactoryProdcer = config->FvFactoryProducer (log);

  FeatureVectorListPtr  trainingExamples = fvFactoryProdcer->ManufacturFeatureVectorList (true, log);

  //****************************************************************************
  // Make sure that there are no existing *.data files that we are going to use.
  // We need to do this in a separate pass because more than one entry may refer 
  // to the same Class and hence the same *.data file.


  //***********************************************************
  //  We will first extract the Raw features from each Class 

  const TrainingClassList&   trainingClasses = config->TrainingClasses ();

  bool  weOwnMlClasses = false; 
  MLClassListPtr mlClasses = config->MlClasses ();
  if  (!mlClasses)
  {
    mlClasses = new MLClassList ();
    weOwnMlClasses = true;
  }


  DateTime  latestTimeStamp;
  bool      changesMadeToThisTrainingClass = false;

  for  (auto  tcIDX: trainingClasses)
  {
    if  (cancelFlag ||  abort)
      break;

    const TrainingClassPtr trainingClass = tcIDX;

    log.Level (20) << "TrainingProcess2::ExtractTrainingClassFeatures Starting on Class[" << trainingClass->Name () << "]." << endl;

    FeatureVectorListPtr  examplesThisClass 
      = ExtractFeatures (config, *mlClasses, trainingClass, latestTimeStamp, changesMadeToThisTrainingClass, cancelFlag, log);

    if  (latestTimeStamp > latestImageTimeStamp)
      latestImageTimeStamp = latestTimeStamp;

    if  (changesMadeToThisTrainingClass)
      changesMadeToTrainingLibraries = true;

    if  (!examplesThisClass)
    {
      abort = true;
      log.Level (-1) << "TrainingProcess2::ExtractTrainingClassFeatures   ***ERROR***   No examples returned by 'ExtractTrainingClassFeatures': " << trainingClass->Name ()  << endl;
    }
    else
    {
      trainingExamples->AddQueue (*examplesThisClass);
      examplesThisClass->Owner (false);
      delete  examplesThisClass;
      examplesThisClass = NULL;
    }
  }

  // DONE Extracting Raw features from All classes.
  if  ((!abort)  &&  (!cancelFlag))
  {
    const TrainingClassPtr  noiseTC = config->NoiseTrainingClass ();
    if  (noiseTC)
    {
      FeatureVectorListPtr  examplesThisClass  = 
            ExtractFeatures (config, 
                             *mlClasses, 
                             noiseTC,
                             latestTimeStamp,
                             changesMadeToThisTrainingClass,
                             cancelFlag,
                             log
                            );
      if  (latestTimeStamp > latestImageTimeStamp)
        latestImageTimeStamp = latestTimeStamp;

      if  (changesMadeToThisTrainingClass)
        changesMadeToTrainingLibraries = true;
      if  (!examplesThisClass)
      {
        log.Level (-1) << endl
          << "TrainingProcess2::ExtractTrainingClassFeatures   ***ERROR***   Loading NoiseClass[" << noiseTC->Name () << "]." << endl
          << endl;
        abort = true;
      }
      else
      {
        trainingExamples->AddQueue (*examplesThisClass);
        examplesThisClass->Owner (false);
        delete  examplesThisClass;
        examplesThisClass = NULL;
      }
    }

    trainingExamples->RandomizeOrder ();

    if  (weOwnMlClasses)
    {
      delete  mlClasses;
      mlClasses = NULL;
    }
  }

  if  (abort)
  {
    delete  trainingExamples;
    trainingExamples = NULL;
    log.Level (10) << "TrainingProcess2::ExtractTrainingClassFeatures   Loading of training data failed." << endl;
  }
  else
  {
    log.Level (20) << "TrainingProcess2::ExtractTrainingClassFeatures   Exiting" << endl;
  }
  return  trainingExamples;
}  /*  ExtractTrainingClassFeatures */





void  TrainingProcess2::ReportTraningClassStatistics (ostream&  report)
{
  report << endl
         << "Training Class Statistics" << endl
         << endl;
  trainingExamples->PrintClassStatistics (report);
}  /* ReportTraningClassStatistics */




void  TrainingProcess2::AddImagesToTrainingLibray (FeatureVectorList&  trainingExamples,
                                                   FeatureVectorList&  examplesToAdd,
                                                   RunLog&             log
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
      log.Level (-1) << endl
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



void  TrainingProcess2::CreateModelsFromTrainingData (WhenToRebuild   whenToRebuild,
                                                      VolConstBool&   cancelFlag,
                                                      RunLog&         log
                                                     )
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

  if  (!fvFactoryProducer)
    fvFactoryProducer = config->FvFactoryProducer (log);

  if  (!fileDesc)
    fileDesc = fvFactoryProducer->FileDesc ();

  delete  priorProbability;
  priorProbability = trainingExamples->GetClassDistribution ();

  // Will Create the Appropriate Model class given 'config->ModelingMethod ()'.
  model = Model::CreateAModel (config->ModelingMethod (),
                               osGetRootName (configFileName),
                               *(config->ModelParameters ()), 
                               fvFactoryProducer, 
                               cancelFlag, 
                               log
                              );

  try
  {
    model->TrainModel (trainingExamples, 
                       featuresAlreadyNormalized,
                       false,     // false = 'model' We are not giving ownership of TrainingExdamples to model.
                       cancelFlag,
                       log
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

  else if  (cancelFlag)
  {
    Abort (true);
  }

  else
  {
    delete  mlClasses;
    mlClasses = model->MLClassesNewInstance ();

    // Need to LOad Sub Processors.
    LoadSubClassifiers (whenToRebuild,
                        false,          /**< false = DON'T check for duplicates. */
                        cancelFlag, 
                        log
                       );
  }

  //trainingExamples->Owner (false);
  buildDateTime = osGetLocalDateTime ();
  log.Level (20) << "TrainingProcess2::CreateModelsFromTrainingData    Ending" << endl;
} /* CreateModelsFromTrainingData */







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




MLClassListPtr  TrainingProcess2::ExtractFullHierachyOfClasses ()  const
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




void  TrainingProcess2::LoadSubClassifiers (WhenToRebuild  whenToRebuild,
                                            bool           checkForDuplicates,
                                            VolConstBool&  cancelFlag,
                                            RunLog&        log
                                           )
{
  if  (!config)
  {
    log.Level (-1) << endl << "TrainingProcess2::LoadSubClassifiers   ***ERROR***   'config' is not defines!!!!" << endl;
    Abort (true);
    return;
  }

  TrainingConfiguration2ListPtr  subClassifiers = config->SubClassifiers ();
  if  (!subClassifiers)
  {
    log.Level (10) << "TrainingProcess2::LoadSubClassifiers    NO Sub-classifiers requested in 'config'." << endl;
    return;
  }

  delete  subTrainingProcesses;
  subTrainingProcesses = new TrainingProcess2List (true);

  for  (auto  idx:  *subClassifiers)
  {
    TrainingConfiguration2Ptr  subClassifier = idx;

    KKStr  subClassifierName = subClassifier->ConfigRootName ();

    log.Level (20) << "TrainingProcess2::LoadSubClassifiers    Loading TrainingProcess2[" << subClassifierName << "]" << endl;

    TrainingProcess2Ptr  tp = 
      TrainingProcess2::CreateTrainingProcess (subClassifier, 
                                               checkForDuplicates, 
                                               whenToRebuild,
                                               true,                /**<  true = saveTrainedModel  when model has to be trained. */
                                               cancelFlag,
                                               log
                                             );
    subTrainingProcesses->PushOnBack (tp);
    if  (tp->Abort ())
    {
      log.Level (-1) << endl 
        << "TrainingProcess2::LoadSubClassifiers   ***ERROR***   Loading SubClassifier[" << subClassifierName << "]." << endl
        << endl;
      Abort (true);
      break;
    }

    else if  (cancelFlag)
    {
      log.Level (-1) << "TrainingProcess2::LoadSubClassifiers   Building of sub-classifiers canceled." << endl;
      Abort (true);
      break;
    }
  }
}  /* LoadSubClassifiers */




void  TrainingProcess2::WriteXML (const KKStr&  varName,
                                  ostream&      o
                                 )  const
{
  XmlTag  startTag ("TrainingProcess2",  XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);
  o << endl;

  XmlElementDateTime::WriteXML (buildDateTime, "BuildDateTime", o);

  configFileNameSpecified.WriteXML ("ConfigFileNameSpecified", o);
  configFileName.WriteXML ("ConfigFileName", o);

  if  (config)
    config->WriteXML ("Config", o);

  XmlElementBool::WriteXML (featuresAlreadyNormalized, "FeaturesAlreadyNormalized", o);
  if  (fvFactoryProducer)
    fvFactoryProducer->Name ().WriteXML ("FvFactoryProducer", o);

  if  (mlClasses)
    XmlElementMLClassNameList::WriteXML (*mlClasses, "MLClasses", o);

  if  (model)
    model->WriteXML ("Model", o);

  if  (priorProbability)
    priorProbability->WriteXML ("PriorProbability", o);

  if  (subTrainingProcesses)
  {
    VectorKKStr  subProcessorsNameList;
    for  (auto idx:  *subTrainingProcesses)
    {
      subProcessorsNameList.push_back (osGetRootName (idx->ConfigFileName ()));
    }

    XmlElementVectorKKStr::WriteXML (subProcessorsNameList, "SubTrainingProcesses", o);
  }


  XmlTag  endTag ("TrainingProcess2", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}  /* WriteXML */





void  TrainingProcess2::ReadXML (XmlStream&      s,
                                 XmlTagConstPtr  tag,
                                 RunLog&         log
                                )
{
  log.Level (20) << "TrainingProcess2::ReadXML" << endl;

  VectorKKStr*  subProcessorsNameList = NULL;

  delete  configOurs;
  configOurs = NULL;
  
  config = NULL;
  fileDesc = NULL;
  fvFactoryProducer = NULL;

  if  (weOwnMLClasses)
  {
    delete  mlClasses;
    mlClasses = NULL;
  }

  delete  model;             model            = NULL;
  delete  priorProbability;  priorProbability = NULL;

  delete  subTrainingProcesses;
  subTrainingProcesses = NULL;

  if  (weOwnTrainingExamples)
    delete  trainingExamples;
  trainingExamples = NULL;

  bool  errorsFound = false;

  XmlTokenPtr  t = s.GetNextToken (log);
  while  (t)
  {
    if  (t->TokenType () == XmlToken::TokenTypes::tokElement)
    {
      XmlElementPtr  e = dynamic_cast<XmlElementPtr> (t);
      const KKStr&  varName = e->VarName ();
      const KKStr&  sectionName = e->SectionName ();

      if  (varName.EqualIgnoreCase ("BuildDateTime")  &&  (typeid (*e) == typeid (XmlElementDateTime)))
        buildDateTime = dynamic_cast<XmlElementDateTimePtr> (e)->Value ();

      else if  (varName.EqualIgnoreCase ("ConfigFileNameSpecified")  &&  (typeid (*e) == typeid (XmlElementKKStr)))
        configFileNameSpecified = *(dynamic_cast<XmlElementKKStrPtr> (e)->Value ());

      else if  (varName.EqualIgnoreCase ("ConfigFileName")  &&  (typeid (*e) == typeid (XmlElementKKStr)))
        configFileName = *(dynamic_cast<XmlElementKKStrPtr> (e)->Value ());

      else if  (varName.EqualIgnoreCase ("Config") )
      {
        delete  configOurs;
        XmlElementTrainingConfiguration2Ptr  xmlConfigElement = dynamic_cast<XmlElementTrainingConfiguration2Ptr> (e);
        if  (xmlConfigElement)
        {
          configOurs = xmlConfigElement->TakeOwnership ();
          config = configOurs;
          if  ((config != NULL)  &&  (fvFactoryProducer == NULL))
          {
            fvFactoryProducer = config->FvFactoryProducer (log);
            fileDesc = fvFactoryProducer->FileDesc ();
          }
        }
      }

      else if  (varName.EqualIgnoreCase ("FvFactoryProducer"))
      {
        fvFactoryProducer = FactoryFVProducer::LookUpFactory (e->ToKKStr ());
        if  (fvFactoryProducer)
          fileDesc = fvFactoryProducer->FileDesc ();
      }
      
      else if  (varName.EqualIgnoreCase ("FeaturesAlreadyNormalized"))
        featuresAlreadyNormalized = e->ToBool ();

      else if  (varName.EqualIgnoreCase ("MlClasses")  &&  (typeid (*e) == typeid (XmlElementMLClassNameList)))
      {
        if  (weOwnMLClasses)
          delete  mlClasses;
        mlClasses = dynamic_cast<XmlElementMLClassNameListPtr> (e)->TakeOwnership ();
        weOwnMLClasses= true;
      }

      else if  (varName.EqualIgnoreCase ("Model"))
      {
        delete  model;
        XmlElementModelPtr  xmlElementModel = dynamic_cast<XmlElementModelPtr> (e);
        if  (xmlElementModel)
        {
          model = xmlElementModel->TakeOwnership ();
          if  (!model->ValidModel ())
          {
            errorsFound = true;
            log.Level (-1) << endl
              << "TrainingProcess2::ReadXML   ***ERROR***    Loaded Model is Invalid" << endl
              << endl;
          }
        }
      }

      else if  (varName.EqualIgnoreCase ("PriorProbability")  &&  (typeid (*e) == typeid (XmlElementClassProbList)))
      {
        delete  priorProbability;
        priorProbability = dynamic_cast<XmlElementClassProbListPtr> (e)->TakeOwnership ();
      }

      else if  (varName.EqualIgnoreCase ("SubTrainingProcesses")  &&  (typeid (*e) == typeid (XmlElementVectorKKStr)))
      {
        delete  subProcessorsNameList;
        subProcessorsNameList = dynamic_cast<XmlElementVectorKKStrPtr> (e)->TakeOwnership ();
      }
      else
      {
        log.Level (-1) << endl
          << "TrainingProcess2::ReadXML   ***ERROR***  Unrecognized Element Section: " << e->NameTagStr () << endl
          << endl;
      }
    }
    delete  t;
    t = s.GetNextToken (log);
  }

  if  (!model)
  {
    log.Level (-1) << endl
      << "TrainingProcess2::ReadXML   ***ERROR***   'model' was not defined." << endl
      << endl;
    errorsFound = true;
  }

  if  (mlClasses == NULL)
  {
    log.Level (-1) << endl
      << "TrainingProcess2::ReadXML   ***ERROR***   'mlClasses' was not defined." << endl
      << endl;
    errorsFound = true;
  }

  if  (model  &&  (model->MLClasses ())  &&  mlClasses)
  {
    if  (*mlClasses != *(model->MLClasses ()))
    {
      log.Level (-1) << endl
        << "TrainingProcess2::ReadXML   ***ERROR***   TrainingProcess2::mlClasses  does not agree with model->MlClasses" << endl
        << endl;
      errorsFound = true;
    }
  }

  if (!errorsFound)
  {
    if  (config)
    {
      if  (config->SubClassifiers () != NULL)
      {
        bool  cancelFlag = false;
        LoadSubClassifiers (WhenToRebuild::NotValid,
                            true,    // CheckForDuplicates
                            cancelFlag,
                            log
                           );
      }
    }
  }

  delete  subProcessorsNameList;
  subProcessorsNameList = NULL;

  if  (errorsFound)
    Abort (true);

  log.Level (20) << "TrainingProcess2::ReadXML    Exiting!" << endl;
}  /* ReadXML */


XmlFactoryMacro(TrainingProcess2)








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

