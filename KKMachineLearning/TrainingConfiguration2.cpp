#include "FirstIncludes.h"
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "MemoryDebug.h"
using namespace  std;


#include "GlobalGoalKeeper.h"
#include "KKBaseTypes.h"
#include "KKException.h"
#include "OSservices.h"
#include "RunLog.h"
using namespace  KKB;


#include "TrainingConfiguration2.h"
#include "BinaryClassParms.h"
#include "FactoryFVProducer.h"
#include "FeatureFileIO.h"
#include "FeatureNumList.h"
#include "FeatureVector.h"
#include "FileDesc.h"
#include "GrayScaleImagesFVProducer.h"
#include "MLClass.h"
#include "KKMLVariables.h"
#include "Model.h"
#include "ModelParamDual.h"
#include "ModelParamKnn.h"
#include "ModelParamOldSVM.h"
#include "ModelParamSvmBase.h"
#include "ModelParamUsfCasCor.h"
#include "NormalizationParms.h"
using namespace  KKMLL;




void  TrainingConfiguration2::CreateModelParameters (const KKStr&           _parameterStr,
                                                     const FeatureNumList&  _selFeatures,
                                                     kkint32                _sectionLineNum,
                                                     kkint32                _parametersLineNum, 
                                                     kkint32                _featuresIncludedLineNum,
                                                     RunLog&                _log
                                                    )
{
  delete  modelParameters;
  modelParameters = NULL;

  if  (_parametersLineNum < 0)
  {
    KKStr errMsg = "'Parameters' setting was not specified.";
    _log.Level (-1) << endl << endl << "CreateModelParameters   ***ERROR***   " << errMsg << endl << endl;
    FormatErrorsAdd (_sectionLineNum, errMsg);
    FormatGood (false);
  }

  switch  (modelingMethod)
  {
  case  Model::ModelTypes::mtDual:      modelParameters = new ModelParamDual      ();  break;
  case  Model::ModelTypes::mtKNN:       modelParameters = new ModelParamKnn       ();  break;
  case  Model::ModelTypes::mtOldSVM:    modelParameters = new ModelParamOldSVM    ();  break;
  case  Model::ModelTypes::mtSvmBase:   modelParameters = new ModelParamSvmBase   ();  break;
  case  Model::ModelTypes::mtUsfCasCor: modelParameters = new ModelParamUsfCasCor ();  break;

  default:
    _log.Level (-1) << endl << endl
      << "TrainingConfiguration2::CreateModelParameters  ***ERROR***   Invalid modeling method[" 
      << Model::ModelTypeToStr (modelingMethod) << "] selected at this time." << endl
      << endl;
    break;
  }

  if  (modelParameters)
  {  
    bool  validParameterFormat = false;
    // We apply '_selFeatures' first;  this way if features were specified in '_parameterStr'  they
    // will override what '_selFeatures'.  This is important because quite often the caller does 
    // not know the features so they assume all.
    modelParameters->SelectedFeatures (_selFeatures);
    modelParameters->ParseCmdLine (_parameterStr, validParameterFormat, _log);

    if  (!modelParameters->ValidParam ())
    {
      KKStr errMsg = "Parameter setting is invalid";
      _log.Level (-1) << endl 
                     << "TrainingConfiguration2::CreateModelParameters  ***ERROR***   " << errMsg << endl 
                     << endl;
      FormatErrorsAdd (_parametersLineNum, errMsg);
      FormatGood (false);
    }

    else if  (_featuresIncludedLineNum > 0)
    {
      modelParameters->SelectedFeatures (_selFeatures);
    }
  }
}  /* CreateModelParameters */



TrainingConfiguration2::TrainingConfiguration2 ():
 Configuration (),

  configFileNameSpecified    (),
  configRootName             (),
  fileDesc                   (NULL),
  fvFactoryProducer          (NULL),
  fvFactoryProducerSpecified (false),
  mlClasses                  (NULL),
  mlClassesWeOwnIt           (false),
  modelingMethod             (Model::ModelTypes::mtNULL),
  examplesPerClass           (0),
  modelParameters            (NULL),
  noiseMLClass               (NULL),
  noiseTrainingClass         (NULL),
  otherClass                 (NULL),
  otherClassLineNum          (-1),
  rootDir                    (),
  rootDirExpanded            (),
  subClassifiers             (NULL),
  subClassifierNameList      (NULL),
  trainingClasses            ("", true),
  validateDirectories        (true)
{
}




void  TrainingConfiguration2::Load (const KKStr&  _configFileName, 
                                    bool          _validateDirectories,
                                    RunLog&       log
                                   )
{
  KKStr  effectiveConfigName = GetEffectiveConfigFileName (_configFileName);

  Configuration::Load (effectiveConfigName, log);

  configFileNameSpecified  = _configFileName;
  configRootName           = KKB::osGetRootName (_configFileName);
  validateDirectories = _validateDirectories;
  if  (!FormatGood ())
  {
    log.Level (-1) << endl 
                   << "TrainingConfiguration2   ***ERROR***  Format of Configuration File is Invalid." << endl 
                   << endl;
    return;
  }

  mlClasses = new MLClassList ();
  mlClassesWeOwnIt = true;

 ValidateConfiguration (log);
 if  (examplesPerClass < 1)
  {
    log.Level (10) << "TrainingConfiguration2 - examplesPerClass not specified.  Defaulting to 1300." << endl;
    examplesPerClass = 1300;
  }

  if  (!FormatGood ())
    log.Level (-1) << endl
                   << "TrainingConfiguration2   ***ERROR***  Format of Configuration File is Invalid." << endl
                   << endl;

  else if  (rootDir.Empty ())
    DetermineWhatTheRootDirectoryIs ();

}  /* Load */






TrainingConfiguration2::TrainingConfiguration2 (MLClassListPtr        _mlClasses,
                                                FactoryFVProducerPtr  _fvFactoryProducer,
                                                const KKStr&          _parameterStr,
                                                RunLog&               _log             
                                               ):
  Configuration              (),
  configFileNameSpecified    (""),
  configRootName             (),
  fileDesc                   (NULL),
  fvFactoryProducer          (_fvFactoryProducer),
  fvFactoryProducerSpecified (true),
  modelingMethod             (Model::ModelTypes::mtNULL),
  mlClasses                  (NULL),
  mlClassesWeOwnIt           (false),
  examplesPerClass           (0),
  modelParameters            (NULL),
  noiseMLClass               (NULL),
  noiseTrainingClass         (NULL),
  otherClass                 (NULL),
  otherClassLineNum          (-1),
  rootDir                    (),
  rootDirExpanded            (),
  subClassifiers             (NULL),
  subClassifierNameList      (NULL),
  trainingClasses            ("", true),
  validateDirectories        (false)
{
  if  (!fvFactoryProducer)
    fvFactoryProducer =  DefaultFeatureVectorProducer (_log);
  fileDesc = fvFactoryProducer->FileDesc ();

  if  (_mlClasses)
    mlClasses = new MLClassList (*_mlClasses);
  else
    mlClasses = new MLClassList ();

  mlClassesWeOwnIt = true;

  {
    MLClassList::iterator idx;
    for  (idx = mlClasses->begin ();  idx != mlClasses->end ();  idx++)
    {
      MLClassPtr mlClass = *idx;
      VectorKKStr  directories;
      TrainingClassPtr  tc = new TrainingClass (directories, mlClass->Name (), 1.0f, 1.0f, NULL, *mlClasses);
      trainingClasses.PushOnBack (tc);
    }
  }

  if  (modelingMethod == Model::ModelTypes::mtNULL)
    modelingMethod = Model::ModelTypes::mtOldSVM;

  {
    examplesPerClass = int32_max;
    FeatureNumList  selectedFeatures (fileDesc);
    selectedFeatures.SetAllFeatures (fileDesc);
    CreateModelParameters (_parameterStr, selectedFeatures, 1, 1, 1, _log);
    if  (!modelParameters  ||  (!modelParameters->ValidParam ()))
    {
      _log.Level (-1) << endl
                     << "TrainingConfiguration2   ***ERROR***   Invalid Parameters." << endl
                     << "    Parameters[" << _parameterStr << "]" << endl
                     << endl;
      FormatGood (false);
    }
  }
}




TrainingConfiguration2::TrainingConfiguration2 (MLClassListPtr  _mlClasses,
                                                FileDescPtr     _fileDesc,
                                                const KKStr&    _parameterStr,
                                                RunLog&         _log
                                               ):
  Configuration              (),
  configFileNameSpecified    (""),
  configRootName             (),
  fileDesc                   (_fileDesc),
  fvFactoryProducer          (NULL),
  fvFactoryProducerSpecified (false),
  mlClasses                  (NULL),
  mlClassesWeOwnIt           (false),
  modelingMethod             (Model::ModelTypes::mtNULL),
  examplesPerClass           (0),
  modelParameters            (NULL),
  noiseMLClass               (NULL),
  noiseTrainingClass         (NULL),
  otherClass                 (NULL),
  otherClassLineNum          (-1),
  rootDir                    (),
  rootDirExpanded            (),
  subClassifiers             (NULL),
  subClassifierNameList      (NULL),
  trainingClasses            ("", true),
  validateDirectories        (false)
{
  if  (_mlClasses)
    mlClasses = new MLClassList (*_mlClasses);
  else
    mlClasses = new MLClassList ();

  mlClassesWeOwnIt = true;

  {
    MLClassList::iterator idx;
    for  (idx = mlClasses->begin ();  idx != mlClasses->end ();  idx++)
    {
      MLClassPtr mlClass = *idx;
      VectorKKStr  directories;
      TrainingClassPtr  tc = new TrainingClass (directories, mlClass->Name (), 1.0f, 1.0f, NULL, *mlClasses);
      trainingClasses.PushOnBack (tc);
    }
  }

  if  (modelingMethod == Model::ModelTypes::mtNULL)
    modelingMethod = Model::ModelTypes::mtOldSVM;

  {
    examplesPerClass = int32_max;
    FeatureNumList  selectedFeatures (fileDesc);
    selectedFeatures.SetAllFeatures (fileDesc);
    CreateModelParameters (_parameterStr, selectedFeatures, 1, 1, 1, _log);
    if  (!modelParameters  ||  (!modelParameters->ValidParam ()))
    {
      _log.Level (-1) << endl
                      << "TrainingConfiguration2   ***ERROR***   Invalid Parameters." << endl
                      << "    Parameters[" << _parameterStr << "]" << endl
                      << endl;
      FormatGood (false);
    }
  }
}



  
TrainingConfiguration2::TrainingConfiguration2 (MLClassListPtr  _mlClasses,
                                                FileDescPtr     _fileDesc,
                                                ModelParamPtr   _modelParameters,
                                                RunLog&         _log
                                               ):
  Configuration              (),
  configFileNameSpecified    (""),
  configRootName             (""),
  fileDesc                   (_fileDesc),
  fvFactoryProducer          (NULL),
  fvFactoryProducerSpecified (false),
  mlClasses                  (NULL),
  mlClassesWeOwnIt           (false),
  modelingMethod             (Model::ModelTypes::mtNULL),
  examplesPerClass           (0),
  modelParameters            (_modelParameters),
  noiseMLClass               (NULL),
  noiseTrainingClass         (NULL),
  otherClass                 (NULL),
  otherClassLineNum          (-1),
  rootDir                    (),
  rootDirExpanded            (),
  subClassifiers             (NULL),
  subClassifierNameList      (NULL),
  trainingClasses            ("", true),
  validateDirectories        (false)
{
  if  (_mlClasses)
    mlClasses = new MLClassList (*_mlClasses);
  else
    mlClasses = new MLClassList ();
  mlClassesWeOwnIt = true;

  {
    MLClassList::iterator idx;
    for  (idx = mlClasses->begin ();  idx != mlClasses->end ();  idx++)
    {
      MLClassPtr      mlClass = *idx;
      VectorKKStr  directories;
      TrainingClassPtr  tc = new TrainingClass (directories, mlClass->Name (), 1.0f, 1.0f, NULL, *mlClasses);
      trainingClasses.PushOnBack (tc);
    }
  }

  switch  (_modelParameters->ModelParamType ())
  {
  case  ModelParam::ModelParamTypes::mptDual:      modelingMethod = Model::ModelTypes::mtDual;       break;
  case  ModelParam::ModelParamTypes::mptKNN:       modelingMethod = Model::ModelTypes::mtKNN;        break;
  case  ModelParam::ModelParamTypes::mptOldSVM:    modelingMethod = Model::ModelTypes::mtOldSVM;     break;
  case  ModelParam::ModelParamTypes::mptSvmBase:   modelingMethod = Model::ModelTypes::mtSvmBase;    break;
  case  ModelParam::ModelParamTypes::mptUsfCasCor: modelingMethod = Model::ModelTypes::mtUsfCasCor;  break;
  }
}
  
  
  
  
TrainingConfiguration2::TrainingConfiguration2 (const TrainingConfiguration2&  tc):
  Configuration (tc),
  configFileNameSpecified    (tc.configFileNameSpecified),
  configRootName             (tc.configRootName),
  fileDesc                   (tc.fileDesc),
  fvFactoryProducer          (tc.fvFactoryProducer),
  fvFactoryProducerSpecified (tc.fvFactoryProducerSpecified),
  mlClasses                  (NULL),
  mlClassesWeOwnIt           (false),
  modelingMethod             (tc.modelingMethod),
  examplesPerClass           (tc.examplesPerClass),
  modelParameters            (NULL),
  noiseMLClass               (tc.noiseMLClass),
  noiseTrainingClass         (tc.noiseTrainingClass),
  otherClass                 (tc.otherClass),
  otherClassLineNum          (tc.otherClassLineNum),
  rootDir                    (tc.rootDir),
  rootDirExpanded            (tc.rootDirExpanded),
  subClassifiers             (NULL),
  subClassifierNameList      (NULL),
  trainingClasses            (tc.rootDir, true),
  validateDirectories        (tc.validateDirectories)
{
  {
    kkint32  x;

    for  (x = 0;  x < tc.trainingClasses.QueueSize ();  x++)
    {
      TrainingClassPtr  trainingClass = tc.trainingClasses.IdxToPtr (x);
      trainingClasses.PushOnBack (new TrainingClass (*trainingClass));
    }
  }

  if  (tc.mlClasses)
  {
    mlClasses = new MLClassList (*tc.mlClasses);
    mlClassesWeOwnIt = true;
  }

  if  (tc.modelParameters)
    modelParameters  = tc.modelParameters->Duplicate ();

  if  (tc.subClassifiers)
  {
    subClassifiers = new TrainingConfiguration2List (true);
    TrainingConfiguration2List::const_iterator  idx;
    for  (idx = tc.subClassifiers->begin ();  idx != tc.subClassifiers->end ();  ++idx)
    {
      TrainingConfiguration2Ptr  subClassifier = *idx;
      subClassifiers->PushOnBack (new TrainingConfiguration2 (*subClassifier));
    }
  }
}





TrainingConfiguration2::~TrainingConfiguration2 ()
{
  delete  noiseTrainingClass;     noiseTrainingClass    = NULL;
  delete  modelParameters;        modelParameters       = NULL;
  delete  subClassifierNameList;  subClassifierNameList = NULL;
  delete  subClassifiers;         subClassifiers        = NULL;

  if  (mlClassesWeOwnIt)
  {
    delete  mlClasses;
    mlClasses = NULL;
  }
}



TrainingConfiguration2Ptr  TrainingConfiguration2::Duplicate ()  const
{
  return new TrainingConfiguration2 (*this);
}



FactoryFVProducerPtr  TrainingConfiguration2::FvFactoryProducer (RunLog&  log) const  
{
  if  (!fvFactoryProducerSpecified)
    return  DefaultFeatureVectorProducer (log);
  else
    return  fvFactoryProducer;
}




FactoryFVProducerPtr  TrainingConfiguration2::DefaultFeatureVectorProducer (RunLog&  runLog)  const
{
  return  GrayScaleImagesFVProducerFactory::Factory (&runLog);
}





ModelParamOldSVMPtr   TrainingConfiguration2::OldSvmParameters ()  const
{
  if  (modelParameters  &&  (modelingMethod == Model::ModelTypes::mtOldSVM))
    return  dynamic_cast<ModelParamOldSVMPtr>(modelParameters);
  else
    return NULL;
}



KKStr  TrainingConfiguration2::RootDirExpanded () const 
{
  return rootDirExpanded;
}



KKStr  TrainingConfiguration2::DirectoryPathForClass (MLClassPtr  mlClass)  const
{
  TrainingClassList::const_iterator  idx;
  for  (idx = trainingClasses.begin ();  idx != trainingClasses.end ();  idx++)
  {
    const TrainingClassPtr  tc = *idx;
    if  (tc->MLClass () == mlClass)
      return  tc->ExpandedDirectory (rootDir, 0);
  }

  return  KKStr::EmptyStr ();
}  /* DirectoryPathForClass */




kkuint32  TrainingConfiguration2::NumHierarchialLevels ()  const
{
  TrainingClassList::const_iterator  idx;

  kkuint16 numHierarchialLevels = 0;

  for  (idx = trainingClasses.begin ();  idx !=  trainingClasses.end ();  idx++)
  {
    TrainingClassPtr  tc = *idx;
    numHierarchialLevels = Max (numHierarchialLevels, tc->MLClass ()->NumHierarchialLevels ());
  }

  return  numHierarchialLevels;
}  /* NumHierarchialLevels */




void  TrainingConfiguration2::SyncronizeMLClassListWithTrainingClassList ()
{
  if  (mlClassesWeOwnIt)
  {
    delete  mlClasses;
    mlClasses = NULL;
  }

  mlClasses = new MLClassList ();
  mlClassesWeOwnIt = true;
  TrainingClassList::const_iterator  idx;

  for  (idx = trainingClasses.begin ();  idx !=  trainingClasses.end ();  idx++)
  {
    MLClassPtr  ic = (*idx)->MLClass ();
    if  (mlClasses->PtrToIdx (ic) < 0)
      mlClasses->AddMLClass (ic);
  }
}  /* SyncronizeMLClassListWithTrainingClassList */





MLClassListPtr   TrainingConfiguration2::ExtractListOfClassesForAGivenHierarchialLevel (kkuint32 level)   const
{
  MLClassListPtr  classes = new MLClassList ();
  TrainingClassList::const_iterator  idx;

  for  (idx = trainingClasses.begin ();  idx !=  trainingClasses.end ();  idx++)
  {
    MLClassPtr  ic = (*idx)->MLClass ();
    MLClassPtr  claassForGivenLevel = ic->MLClassForGivenHierarchialLevel (level);
    if  (classes->PtrToIdx (claassForGivenLevel) < 0)
      classes->AddMLClass (claassForGivenLevel);
  }

  return  classes;
}  /* ExtractListOfClassesForAGivenHierarchialLevel */




TrainingConfiguration2Ptr  TrainingConfiguration2::GenerateAConfiguraionForAHierarchialLevel (kkuint32  level,
                                                                                              RunLog&   log
                                                                                             )  const
{
  log.Level (10) << "TrainingConfiguration2::GenerateAConfiguraionForAHierarchialLevel  level[" << level << "]" << endl;
  TrainingConfiguration2Ptr  hierarchialConfig = new TrainingConfiguration2 (*this);
  hierarchialConfig->EmptyTrainingClasses ();

  MLClassListPtr  hierarchialClassList = ExtractListOfClassesForAGivenHierarchialLevel (level);

  TrainingClassList::const_iterator  idx;
  
  for  (idx = trainingClasses.begin ();  idx != trainingClasses.end ();  idx++)
  {
    const TrainingClassPtr tc = *idx;
    MLClassPtr  ic = tc->MLClass ()->MLClassForGivenHierarchialLevel (level);
    hierarchialConfig->AddATrainingClass (new TrainingClass (tc->Directories (), ic->Name (), 1.0f, 1.0f, NULL, *hierarchialClassList));
  }

  hierarchialConfig->SyncronizeMLClassListWithTrainingClassList ();

  delete  hierarchialClassList;
  return  hierarchialConfig;
} /* GenerateAConfiguraionForAHierarchialLevel */




void  TrainingConfiguration2::Save (ostream&  o)  const
{
  {
    // Global Section
    o << "[Global]" << endl
      << "MODELING_METHOD = " << ModelTypeToStr (modelingMethod)  << endl
      << "ROOT_DIR = "        << rootDir                          << endl
      << "Parameters="        << modelParameters->ToCmdLineStr () << endl;

    if  (examplesPerClass > 0)
      o << "Examples_Per_Class=" << examplesPerClass << endl;

    if  (modelParameters->SelectedFeatures ())
      o << "Features_Included=" << modelParameters->SelectedFeatures ()->ToString () << endl;

    if  (otherClass != NULL)
      o << "OtherClass=" << otherClass->Name () << endl;
    o << endl;
  }


  if  (noiseTrainingClass)
  {
    o << "[NOISE_IMAGES]" << endl;
    o << "Class_Name=" << noiseTrainingClass->Name ()  << endl;
    for  (kkuint32 zed = 0;  zed < noiseTrainingClass->DirectoryCount ();  ++zed)
      o << "Dir=" << noiseTrainingClass->Directory (zed) << endl;

    o << endl;
  }

  TrainingClassList::const_iterator tcIDX;
  for  (tcIDX = trainingClasses.begin ();  tcIDX !=  trainingClasses.end ();  tcIDX++)
  {
    TrainingClassPtr  tc = *tcIDX;

    o << "[TRAINING_CLASS]" << endl;

    o << "Class_Name=" << tc->Name () << endl;
    if  (tc->Weight () != 1.0f)
      o << "Weight=" << tc->Weight () << endl;

    if  (tc->CountFactor () > 0.0f)
      o << "Count_Factor=" << tc->CountFactor () << endl;

    TrainingConfiguration2Ptr  subClassifier =  tc->SubClassifier ();
    if  (subClassifier)
      o << "Sub_Classifier=" << subClassifier->ConfigRootName () << endl;

    for  (kkuint32 zed = 0;  zed < tc->DirectoryCount ();  ++zed)
    {
      const KKStr&  trainClassDir = tc->Directory (zed);
      if  (!trainClassDir.StartsWith (rootDir))
      {
        KKStr  dirStr = tc->Directory (zed);
        dirStr.TrimLeft ();
        dirStr.TrimRight ();
        if  (!dirStr.Empty ())
          o << "Dir=" << dirStr  << endl;
      }
    }
    o << endl;
  }

  ModelParamOldSVMPtr  oldSVMParams = OldSvmParameters ();
  if  (oldSVMParams)
  {
    const BinaryClassParmsListPtr  binaryParms = oldSVMParams->BinaryParmsList ();
    if  (binaryParms)
    {
      BinaryClassParmsList::const_iterator  idx;

      for  (idx = binaryParms->begin ();  idx != binaryParms->end ();  idx++)
      {
        const BinaryClassParmsPtr  binaryClass = *idx;
        o << "[TWO_CLASS_PARAMETERS]" << endl;
        o << "CLASS1="            << binaryClass->Class1Name ()            << endl;
        o << "CLASS2="            << binaryClass->Class2Name ()            << endl;
        o << "Parameters="        << binaryClass->Param ().ToCmdLineStr () << endl;
        o << "Features_Included=" << binaryClass->SelectedFeatures ()      << endl;
        o << "Weight="            << binaryClass->Weight ()                << endl;
        o << endl;
      }
    }

    o << endl;
  }
}  /* Save */



void  TrainingConfiguration2::Save (const KKStr& fileName)  const
{
  ofstream  o (fileName.Str ());
  Save (o);
  o.close ();
} /* Save */




void   TrainingConfiguration2::RootDir (const KKStr& _rootDir)
{
  rootDir = _rootDir;
  rootDirExpanded = KKB::osSubstituteInEnvironmentVariables (rootDir);
  trainingClasses.RootDir (rootDir);
}



void   TrainingConfiguration2::ModelParameters (ModelParamPtr  _modelParameters)   
{
  delete  modelParameters;
  modelParameters = _modelParameters->Duplicate ();
}



KKStr  TrainingConfiguration2::ModelParameterCmdLine ()  const
{
  ModelParamPtr  p = ModelParameters ();
  if  (!p)
    return "";
  else
    return p->ToCmdLineStr ();
}  /* ModelParameterCmdLine */



MLClassListPtr   TrainingConfiguration2::ExtractClassList ()  const
{
  TrainingClassList::const_iterator  idx;

  MLClassListPtr  classes = new MLClassList ();

  for  (idx = trainingClasses.begin ();  idx != trainingClasses.end ();  idx++)
  {
    TrainingClassPtr  tc = *idx;

    if  (classes->PtrToIdx (tc->MLClass ()) < 0)
      classes->PushOnBack (tc->MLClass ());
  }

  classes->SortByName ();

  return  classes;
}   /* ExtractClassList */





MLClassListPtr   TrainingConfiguration2::ExtractFullHierachyOfClasses ()  const
{
  TrainingClassList::const_iterator  idx;

  MLClassListPtr  classes = new MLClassList ();

  for  (idx = trainingClasses.begin ();  idx != trainingClasses.end ();  idx++)
  {
    TrainingClassPtr  tc = *idx;
    if  (tc->SubClassifier () == NULL)
    {
      if  (classes->PtrToIdx (tc->MLClass ()) < 0)
        classes->PushOnBack (tc->MLClass ());
    }
    else
    {
      MLClassListPtr  subClassifierClasses = tc->SubClassifier ()->ExtractFullHierachyOfClasses ();
      MLClassList::const_iterator  idx2;
      for  (idx2 = subClassifierClasses->begin();  idx2 != subClassifierClasses->end ();  ++idx2)
      {
        MLClassPtr       subClass = *idx2;
        if  (classes->PtrToIdx (subClass) < 0)
          classes->PushOnBack (subClass);
      }
      delete  subClassifierClasses;
      subClassifierClasses = NULL;
    }
  }

  classes->SortByName ();
  return  classes;
}  /* ExtractFullHierachyOfClasses */





TrainingConfiguration2Ptr  TrainingConfiguration2::CreateFromFeatureVectorList
                                            (FeatureVectorList&  _examples,
                                             FileDescPtr         _fileDesc,
                                             const KKStr&        _parameterStr, 
                                             RunLog&             _log
                                            )
{
  _log.Level (10) << "TrainingConfiguration2::CreateFromFeatureVectorList" << endl;
  FileDescPtr  fileDesc = _examples.FileDesc ();

  MLClassListPtr  mlClasses = _examples.ExtractListOfClasses ();
  mlClasses->SortByName ();
  KKStr  parameterStr = _parameterStr;
  if  (parameterStr.Empty ())
    parameterStr = "-m 200 -s 0 -n 0.11 -t 2 -g 0.024717  -c 10  -u 100  -up  -mt OneVsOne  -sm P";

  TrainingConfiguration2Ptr  config 
      = new TrainingConfiguration2 (mlClasses,
                                    _fileDesc,
                                    parameterStr,
                                   _log
                                  );

  config->SetFeatureNums (_examples.AllFeatures ());

  delete  mlClasses;  mlClasses = NULL;

  return  config;
}  /* CreateFromFeatureVectorList */







TrainingConfiguration2Ptr  TrainingConfiguration2::CreateFromDirectoryStructure 
                                                    (const KKStr&          _existingConfigFileName,
                                                     const KKStr&          _subDir,
                                                     FactoryFVProducerPtr  _fvFactoryProducer,
                                                     RunLog&               _log,
                                                     bool&                 _successful,
                                                     KKStr&                _errorMessage
                                                    )
{
  _log.Level (10) << "TrainingConfiguration2::CreateFromDirectoryStructure"  << endl
                  << "                       Feature Data from SubDir[" << _subDir << "]." << endl
                  << endl;

  _successful = true;

  KKStr  directoryConfigFileName = osGetRootNameOfDirectory (_subDir);
  if  (directoryConfigFileName.Empty ())
    directoryConfigFileName = osAddSlash (KKMLVariables::TrainingModelsDir ()) + "Root.cfg";
  else
    directoryConfigFileName = osAddSlash (KKMLVariables::TrainingModelsDir ()) + directoryConfigFileName + ".cfg";

  TrainingConfiguration2Ptr  config = NULL;

  if  (!_existingConfigFileName.Empty ())
  {
    if  (osFileExists (_existingConfigFileName))
    {
      config = _fvFactoryProducer->ManufacturTrainingConfiguration ();
      config->Load (_existingConfigFileName, 
                    false,  //  false = DO NOT Validate Directories.
                    _log
                   );
      config->RootDir (_subDir);
      if  (!(config->FormatGood ()))
      {
        delete  config;
        config = NULL;
      }
    }
  }

  if  (!config)
  {
    if  (osFileExists (directoryConfigFileName))
    {
      config = _fvFactoryProducer->ManufacturTrainingConfiguration ();
      config->Load (directoryConfigFileName,
                    false,  // false = Do Not Validate Directories.
                    _log 
                   );

      config->RootDir (_subDir);
      if  (!(config->FormatGood ()))
      {
        delete  config;
        config = NULL;
      }
    }
  }

  if  (!config)
  {
    if  (_fvFactoryProducer != NULL)
      _fvFactoryProducer = GrayScaleImagesFVProducerFactory::Factory (&_log);
    config = new TrainingConfiguration2 (NULL,      // Not supplying the MLClassList
                                         _fvFactoryProducer,
                                         "=-s 0 -n 0.11 -t 2 -g 0.01507  -c 12  -u 100  -up  -mt OneVsOne  -sm P",
                                         _log
                                        );
    config->RootDir (_subDir);
  }

  config->BuildTrainingClassListFromDirectoryStructure (_subDir,
                                                        _successful, 
                                                        _errorMessage,
                                                        _log
                                                       );

  if  (config->Gamma () == 0.0)
    config->Gamma (0.01507);

  config->Save (directoryConfigFileName);

  return  config;
} /* CreateFromDirectoryTree */




void  TrainingConfiguration2::BuildTrainingClassListFromDirectoryStructure (const KKStr&  _subDir,
                                                                            bool&         _successful,
                                                                            KKStr&        _errorMessage,
                                                                            RunLog&       _log
                                                                           )
{
  _log.Level (10) << "TrainingConfiguration2::BuildTrainingClassListFromDirectoryStructure"  << endl
                  << "                       Feature Data from SubDir[" << _subDir << "]." << endl
                  << endl;

  _successful = true;

  if  (ModelParameters () == NULL)
  {
    bool  _validFormat = true;
    KKStr  svmParameterStr = "-s 0 -n 0.11 -t 2 -g 0.01507  -c 12  -u 100  -up  -mt OneVsOne  -sm P";

    ModelParamOldSVMPtr  oldSVMparameters = new ModelParamOldSVM ();
    oldSVMparameters->ParseCmdLine (svmParameterStr, _validFormat, _log);
    ModelParameters (oldSVMparameters);
    ImagesPerClass (1000);
    delete  oldSVMparameters;
    oldSVMparameters = NULL;
  }
  
  TrainingClassListPtr  origTraniningClasses = TrainingClasses ().DuplicateListAndContents ();

  EmptyTrainingClasses ();
  
  if  (mlClasses)
  {
    while  (mlClasses->QueueSize () > 1)
      mlClasses->PopFromBack ();
  }

  KKStr  subDirUnderRoot = "";
  BuildTrainingClassListFromDirectoryEntry (_subDir,
                                            subDirUnderRoot,
                                            _successful, 
                                            _errorMessage,
                                            _log
                                           );

  {
    // We can now update the traningClasses found with that which is in origTraniningClasses.
    TrainingClassList::iterator  idx;

    for  (idx = trainingClasses.begin ();  idx != trainingClasses.end ();  idx++)
    {
      TrainingClassPtr  tc = *idx;

      // I am not sure about this line.  Might need to use rootDir from original configuration file.
      TrainingClassPtr  origTC = origTraniningClasses->LocateByDirectory (tc->ExpandedDirectory (_subDir, 0));
      if  (origTC)
      {
        tc->MLClass      (origTC->MLClass ());
        tc->FeatureFileName (origTC->FeatureFileName ());
      }
    }
  }

  delete  origTraniningClasses;
  origTraniningClasses = NULL;
}  /* BuildTrainingClassListFromDirectoryStructure */





void  TrainingConfiguration2::BuildTrainingClassListFromDirectoryEntry (const KKStr&  rootDir,
                                                                        const KKStr&  subDir,
                                                                        bool&         successful,
                                                                        KKStr&        errorMessage,
                                                                        RunLog&       log
                                                                       )
{
  log.Level (10) << "BuildTrainingClassListFromDirectoryEntry  subDir[" << subDir << "]" << endl;

  KKStr  currentDirectory = osAddSlash (rootDir) + subDir;

  KKStr  dirSearchPath = osAddSlash (currentDirectory) + "*.*";
  {
    // Are there any image files in this directory ?
    KKStrListPtr   imageFilesInDir = osGetListOfImageFiles (dirSearchPath);
    bool  thereAreImageFilesInDir = (imageFilesInDir->QueueSize () > 0);
    delete  imageFilesInDir;

    if  (thereAreImageFilesInDir)
    {
      KKStr  className = MLClass::GetClassNameFromDirName (currentDirectory);
      VectorKKStr subDirs;
      subDirs.push_back (subDir);
      AddATrainingClass (new TrainingClass (subDirs, className, 1.0f, 1.0f, NULL, *mlClasses));
    }
  }

  successful = true;
  KKStrListPtr  subDirectories = osGetListOfDirectories (dirSearchPath);
  KKStrList::iterator  sdIDX = subDirectories->begin ();
  for  (sdIDX = subDirectories->begin ();  ((sdIDX != subDirectories->end ())  &&  successful);  ++sdIDX)
  {
    KKStrPtr  subDirName = *sdIDX;
    KKStr  newDirPath = "";
    if  (subDir.Empty ())
       newDirPath = *subDirName;
    else
       newDirPath = osAddSlash (subDir) + (*subDirName);
    BuildTrainingClassListFromDirectoryEntry (rootDir, newDirPath, successful, errorMessage, log);
  }
  delete  subDirectories;
}  /* BuildTrainingClassListFromDirectoryEntry */



/**
 * @brief  Add a Training class to configuration
 *         Will take ownership of allocation.
 */
void  TrainingConfiguration2::AddATrainingClass (TrainingClassPtr  _trainClass)
{
  trainingClasses.PushOnBack (_trainClass);
}  /* AddATrainingClass */



/**
 *@brief Will assume that images for this class will be saved off the RootDirectory using its own
  name for the subdirectory name.
 */
void   TrainingConfiguration2::AddATrainingClass (MLClassPtr  _newClass)
{
  VectorKKStr directories;
  TrainingClassPtr  tc 
    = new TrainingClass (directories,
                         _newClass->Name (),
                         1.0f,                /**< Weight given to this Class during training  */
                         1.0f,                /**< CountFactor.                                */
                         NULL,                /**< Sub-Classifier.                             */
                         *mlClasses
                        );
  AddATrainingClass (tc);
}



const SVMparam&  TrainingConfiguration2::SVMparamREF (RunLog&  log)  const
{
  ModelParamOldSVMPtr  oldSVMparms = OldSvmParameters ();
  if  (oldSVMparms)
  {
    SVMparamPtr s =  oldSVMparms->SvmParameters ();
    return  *s;
  }
  else
  {
    KKStr  errMsg = "TrainingConfiguration2::SVMparamREF   ***ERROR***   (modelingMethod != mmOldSVM)";
    log.Level (-1) << endl << errMsg << endl << endl;
    throw new KKException (errMsg);
  }
}  /* SVMparam */



SVMparamPtr  TrainingConfiguration2::SVMparamToUse ()  const
{
  ModelParamOldSVMPtr  oldSVMparms = OldSvmParameters ();
  if  (oldSVMparms)
  {
    SVMparamPtr s =  oldSVMparms->SvmParameters  ();
    return  s;
  }
  else
  {
    return NULL;
  }
}  /* SVMparamToUse */





float   TrainingConfiguration2::A_Param () const
{
  if  (modelParameters)
    return  modelParameters->A_Param ();
  else
    return 0.0f;
}



void  TrainingConfiguration2::A_Param (float  _aParam)
{
  if  (modelParameters)
    modelParameters->A_Param (_aParam);
}  /* A_Param */



double  TrainingConfiguration2::Gamma () const
{
  if  (modelParameters)
    return  modelParameters->Gamma ();
  else
    return 0.0;
}  /* Gamma */



void  TrainingConfiguration2::Gamma (double  _gamma)
{
  if  (modelParameters)
    modelParameters->Gamma (_gamma);
}  /* Gamma */



double  TrainingConfiguration2::C_Param () const
{
  if  (modelParameters)
    return  modelParameters->C_Param ();
  else
    return 0.0;
}



void  TrainingConfiguration2::C_Param (double _CCC)
{
  if  (modelParameters)
    modelParameters->C_Param (_CCC);
}



kkint32 TrainingConfiguration2::Number_of_rounds ()  const
{
  if  (modelParameters  &&  (modelParameters->ModelParamType () == ModelParam::ModelParamTypes::mptUsfCasCor))
  {
    return  dynamic_cast<ModelParamUsfCasCor*>(modelParameters)->Number_of_rounds ();
  }
  else
  {
    return  -1;
  }
}



void   TrainingConfiguration2::Number_of_rounds (kkint32 _number_of_rounds)
{
  if  (modelParameters  &&  (modelParameters->ModelParamType () == ModelParam::ModelParamTypes::mptUsfCasCor))
  {
    dynamic_cast<ModelParamUsfCasCor*>(modelParameters)->Number_of_rounds (_number_of_rounds);
  }
}



void  TrainingConfiguration2::MachineType (SVM_MachineType _machineType)
{
  ModelParamOldSVMPtr  oldSVMparms = OldSvmParameters ();
  if  (oldSVMparms)
    oldSVMparms->MachineType (_machineType);

}  /* MachineType */




SVM_MachineType  TrainingConfiguration2::MachineType ()  const
{
  ModelParamOldSVMPtr  oldSVMparms = OldSvmParameters ();
  if  (oldSVMparms)
    return oldSVMparms->MachineType ();
  else
    return  SVM_MachineType::MachineType_NULL;
}  /* MachineType */




void  TrainingConfiguration2::SelectionMethod (SVM_SelectionMethod  _selectionMethod)
{
  ModelParamOldSVMPtr  oldSVMparms = OldSvmParameters ();
  if  (oldSVMparms)
    oldSVMparms->SelectionMethod (_selectionMethod);
}  /* SelectionMethod */



SVM_SelectionMethod  TrainingConfiguration2::SelectionMethod   ()  const
{
  ModelParamOldSVMPtr  oldSVMparms = OldSvmParameters ();
  if  (oldSVMparms)
    return (SVM_SelectionMethod)oldSVMparms->SelectionMethod ();
  else
    return SVM_SelectionMethod::SelectionMethod_NULL;
}  /* SelectionMethod */



void  TrainingConfiguration2::SetFeatureNums (const  FeatureNumList&  features)
{
  if  (modelParameters)
    modelParameters->SelectedFeatures (features);
}  /* SetFeatureNums */




void  TrainingConfiguration2::SetFeatureNums (MLClassPtr             class1,
                                              MLClassPtr             class2,
                                              const FeatureNumList&  _features,
                                              float                  _weight
                                             )
{
  ModelParamOldSVMPtr  oldSVMparms = OldSvmParameters ();
  if  (oldSVMparms)
  {
    oldSVMparms->SetFeatureNums (class1, class2, &_features, _weight);
  }
}  /* SetFeatureNums */





void  TrainingConfiguration2::EncodingMethod (SVM_EncodingMethod _encodingMethod)
{
  if  (modelParameters)
    modelParameters->EncodingMethod ((ModelParam::EncodingMethodType)_encodingMethod);
}



SVM_EncodingMethod   TrainingConfiguration2::EncodingMethod ()  const
{
  const SVMparamPtr  param = SVMparamToUse ();
  if  (param)
    return  param->EncodingMethod ();
  else
    return  SVM_EncodingMethod::NoEncoding;
}  /* EncodingMethod */





double  TrainingConfiguration2::C_Param (MLClassPtr  class1,
                                         MLClassPtr  class2
                                        )  const
{
  ModelParamOldSVMPtr  oldSVMparms = OldSvmParameters ();
  if  (oldSVMparms)
    return  oldSVMparms->C_Param (class1, class2);
  else
    return 0.0;
}




void  TrainingConfiguration2::C_Param (MLClassPtr  class1,
                                       MLClassPtr  class2,
                                       double      cParam
                                      )
{
  ModelParamOldSVMPtr  oldSVMparms = OldSvmParameters ();
  if  (oldSVMparms)
    oldSVMparms->C_Param (class1, class2, cParam);
}  /* C_Param */







void  TrainingConfiguration2::SetBinaryClassFields (MLClassPtr                     class1,
                                                    MLClassPtr                     class2,
                                                    const SVM233::svm_parameter&   _param,
                                                    const FeatureNumList&          _features,
                                                    float                          _weight
                                                   )
{
  ModelParamOldSVMPtr  oldSVMparms = OldSvmParameters ();
  if  (oldSVMparms)
    oldSVMparms->SetBinaryClassFields (class1, class2, _param, &_features, _weight);
}  /* SetBinaryClassFields */




void  TrainingConfiguration2::SetTrainingClasses (TrainingClassListPtr  _trainingClasses)
{
  EmptyTrainingClasses ();
  TrainingClassList::iterator  idx;

  for  (idx = _trainingClasses->begin ();  idx != _trainingClasses->end ();  idx++)
    AddATrainingClass (*idx);
}  /* SetTrainingClasses */




float  TrainingConfiguration2::AvgNumOfFeatures ()  const
{
  if  (modelParameters)
    return modelParameters->AvgMumOfFeatures ();
  return 0.0f;
}  /* AvgMumOfFeatures */




float  TrainingConfiguration2::AvgNumOfFeatures (FeatureVectorListPtr  trainExamples)  const
{
  const SVMparamPtr  param = SVMparamToUse ();
  if  (param)
    return param->AvgNumOfFeatures (trainExamples);
  else
    return  0.0f;
}




SVM_KernalType  TrainingConfiguration2::KernalType ()  const
{
  const SVMparamPtr  param = SVMparamToUse ();
  if  (param)
    return param->KernalType ();
  return  SVM_KernalType::KT_Linear;
}  /* KernalType */




void   TrainingConfiguration2::KernalType (SVM_KernalType _kernalType)
{
  const SVMparamPtr  param = SVMparamToUse ();
  if  (param)
    return param->KernalType (_kernalType);
}  /* KernalType */


FeatureNumList  TrainingConfiguration2::GetFeatureNums ()  const
{
  if  (modelParameters)
  {
    FeatureNumListConstPtr fnl = modelParameters->SelectedFeatures ();
    if  (fnl)
      return *fnl;
  }
  if  (fileDesc)
    return FeatureNumList (fileDesc);
  else
    return FeatureNumList (1);
}  /* GetFeatureNums */




kkint32  TrainingConfiguration2::NumOfFeaturesAfterEncoding (RunLog&  log)  const
{
  if  (modelParameters)
    return modelParameters->NumOfFeaturesAfterEncoding (fileDesc, log);
  else
    return 0;
}  /* NumOfFeaturesAfterEncoding */



void   TrainingConfiguration2::ExamplesPerClass (kkint32 _examplesPerClass)
{
  examplesPerClass = _examplesPerClass;
}



kkint32  TrainingConfiguration2::ExamplesPerClass ()  const
{
  return examplesPerClass;
}  /* ImagesPerClass */





FeatureNumList   TrainingConfiguration2::GetFeatureNums (MLClassPtr  class1,
                                                         MLClassPtr  class2
                                                        )
{
  ModelParamOldSVMPtr  oldSVMparms = OldSvmParameters ();
  if  (oldSVMparms)
  {
    FeatureNumListConstPtr  fnl = oldSVMparms->GetFeatureNums (fileDesc, class1, class2);
    if  (fnl)
      return *fnl;
  }

  return  FeatureNumList::AllFeatures (fileDesc);
}  /* GetFeatureNums */





void  TrainingConfiguration2::SetModelParameters (ModelParamPtr  _svmParanters,
                                                  kkint32        _examplesPerClass
                                                 )
{
  delete  modelParameters;
  modelParameters   = _svmParanters->Duplicate ();
  examplesPerClass  = _examplesPerClass;
}  /* SetModel3Parameters */




TrainingConfiguration2Ptr  TrainingConfiguration2::ValidateSubClassifier (const KKStr&  subClassifierName,
                                                                          bool&         errorsFound,
                                                                          RunLog&       log
                                                                         )
{
  errorsFound = false;
  KKStr  rootName = KKB::osGetRootName (subClassifierName);
  if  (!subClassifiers)
    subClassifiers = new TrainingConfiguration2List (true);

  TrainingConfiguration2Ptr config = subClassifiers->LookUp (subClassifierName);
  if  (!config)
  {
    config = fvFactoryProducer->ManufacturTrainingConfiguration ();
    config->Load (subClassifierName, true, log);
    subClassifiers->PushOnBack (config);
    if  (!config->FormatGood ())
    {
      errorsFound = true;
      const VectorKKStr&  errors   = config->FormatErrors ();
      const VectorInt&    lineNums = config->FormatErrorsLineNums ();
      kkuint32 zed = Max (errors.size (), lineNums.size ());
      for  (kkuint32 x = 0;  x < zed;  ++x)
        FormatErrorsAdd (lineNums[x], errors[x]);
    }
  }

  return  config;
}  /* ValidateSubClassifier */






TrainingClassPtr  TrainingConfiguration2::ValidateClassConfig (kkint32  sectionNum,
                                                               RunLog&  log
                                                              )
{
  kkint32  numOfSettings = NumOfSettings ((kkint32)sectionNum);

  kkint32  classNameLineNum   = 0;
  kkint32  dirLineNum         = 0;
  kkint32  weightLineNum      = 0;
  kkint32  countFactorLineNum = 0;
  kkint32  subClassifierLineNum = 0;

  kkint32  sectionLineNum = SectionLineNum (sectionNum);

  VectorKKStr  directories;
  KKStr        className;
  KKStr        subClassifierName;
  float        weight = 1.0f;
  float        countFactor = 0.0f;

  for  (kkint32 settingNum = 0;  settingNum < numOfSettings;  ++settingNum)
  {
    KKStrConstPtr  settingNamePtr = SettingName (sectionNum, settingNum);
    if  (!settingNamePtr)
      continue;

    kkint32 settingLineNum = 0;
    KKStrConstPtr  setingValuePtr = SettingValue (sectionNum, settingNum, settingLineNum);
    KKStr  settingValue = "";
    if  (setingValuePtr)
      settingValue = *setingValuePtr;
    
    if  (settingNamePtr->EqualIgnoreCase ("CLASS_NAME"))
    {
      className = settingValue;
      classNameLineNum = settingLineNum;
    }

    else if  (settingNamePtr->EqualIgnoreCase ("SUB_CLASSIFIER"))
    {
      subClassifierName = settingValue;
      subClassifierLineNum = settingLineNum;
    }

    else if  (settingNamePtr->EqualIgnoreCase ("WEIGHT"))
    {
      weight = settingValue.ToFloat ();
      weightLineNum = settingLineNum;
    }

    else if  (settingNamePtr->EqualIgnoreCase ("COUNT_FACTOR"))
    {
      countFactor = settingValue.ToFloat ();
      countFactorLineNum = settingLineNum;
    }

    else if  (settingNamePtr->EqualIgnoreCase ("DIR"))
    {
      if  (!settingValue.Empty ())
        directories.push_back (settingValue);

      else if  (!className.Empty ())
        directories.push_back (className);
    }
  }

  if  (directories.size () < 1)
  {
    directories.push_back (className);
  }

  if  (className.Empty ())
  {
    KKStr errMsg = "'TRAINING_CLASS' section, CLASS_NAME not defined.";
    log.Level (-1) << "ValidateClassConfig   " << errMsg  << endl;
    FormatErrorsAdd (sectionLineNum, errMsg);
    FormatGood (false);
    return  NULL;
  }

  TrainingConfiguration2Ptr subClassifer = NULL;
  if  (!subClassifierName.Empty ())
  {
    bool  errorsFound = false;
    subClassifer = ValidateSubClassifier (subClassifierName, errorsFound, log);
    if  (errorsFound)
    {
      KKStr  errMsg = "Sub-Classifier [" + (subClassifierName) + "] has a format error: " + subClassifer->FileName ();
      FormatErrorsAdd (subClassifierLineNum, errMsg);
      VectorKKStr subClassifierErrors = subClassifer->FormatErrorsWithLineNumbers ();
      VectorKKStr::const_iterator  idx;
      for  (idx = subClassifierErrors.begin ();  idx != subClassifierErrors.end ();  ++idx)
        FormatErrorsAdd (subClassifierLineNum, "  " + subClassifierName + ": " + *idx);
    }
  }

    if  (weight <= 0.0f)
    {
    KKStr  errMsg = "Class[" + className + "]    Invalid Weight Parameter.";
      log.Level (-1) << "ValidateClassConfig   " << errMsg << endl;
      FormatErrorsAdd (weightLineNum, errMsg);
      FormatGood (false);
      return  NULL;
    }

  TrainingClassPtr tc = new TrainingClass (directories, className, weight,  countFactor, subClassifer, *mlClasses);

  if  (validateDirectories)
  {
    KKStr  rootDirExpanded = osSubstituteInEnvironmentVariables (rootDir);
    for  (kkuint32  zed = 0;  zed < tc->DirectoryCount ();  ++zed)
    {
      KKStr  expandedDir = tc->ExpandedDirectory (rootDirExpanded, zed);
      if  (!osValidDirectory (expandedDir))
      {
        KKStr  errMsg = "Invalid Directory Specified[" + expandedDir + "].";
        log.Level (-1) << endl << "ValidateClassConfig   ***ERROR***  " << errMsg << endl << endl;
        FormatErrorsAdd (dirLineNum, errMsg);
        FormatGood (false);
        delete  tc;  tc = NULL;
        return NULL;
      }
    }
  }

  return  tc;
}  /* ValidateClassConfig */



void  TrainingConfiguration2::ValidateOtherClass (MLClassPtr  otherClass,
                                                  kkint32     otherClassLineNum,
                                                  RunLog&     log
                                                 )
{
  KKStr  classDirToUse = osAddSlash (rootDir) + otherClass->Name ();
  if  (!osValidDirectory (classDirToUse))
  {
    KKStr  errMsg = "OtherClass  Directory [" + classDirToUse + "]  does not exist.";
    log.Level (-1) << endl << "ValidateOtherClass   ***ERROR***  " << errMsg << endl << endl;
    FormatErrorsAdd (otherClassLineNum, errMsg);
    FormatGood (false);
  }
}  /* ValidateOtherClass */




void  TrainingConfiguration2::ValidateTrainingClassConfig (kkint32  sectionNum,
                                                           RunLog&  log
                                                          )
{
  TrainingClassPtr  trainingClass = ValidateClassConfig (sectionNum, log);
  if  (trainingClass)
  {
    trainingClasses.AddTrainingClass (trainingClass);
  }

}  /* ValidateTrainingClassConfig */



FeatureNumListPtr  TrainingConfiguration2::DeriveFeaturesSelected (kkint32  sectionNum)
{
  kkint32  sectionLineNum = SectionLineNum (sectionNum);

  kkint32  featuresIncludedLineNum = 0;
  kkint32  featuresExcludedLineNum = 0;

  KKStr  includedFeaturesStr (SettingValue (sectionNum, "FEATURES_INCLUDED", featuresIncludedLineNum));

  FeatureNumListPtr  selectedFeatures = NULL;

  if  (includedFeaturesStr.Empty ()  ||  includedFeaturesStr.EqualIgnoreCase ("ALL"))
  {
    // We will assume user want's all features.
    selectedFeatures = new FeatureNumList (FeatureNumList::AllFeatures (fileDesc));
  }

  else 
  {
    bool valid = false;
    selectedFeatures = new FeatureNumList (includedFeaturesStr, valid);
  }

  if  (!selectedFeatures)
  {
    FormatGood (false);
    delete  selectedFeatures;
    return  NULL;
  }

  return  selectedFeatures;
} /* DeriveFeaturesSelected */



void   TrainingConfiguration2::ValidateGlobalSection (kkint32  sectionNum,
                                                      RunLog&  log
                                                     )
{
  kkint32  methodLineNum  = 0;
  kkint32  rootDirLineNum = 0;
  kkint32  sectionLineNum = SectionLineNum (sectionNum);
  kkint32  fvFactoryLineNum = 0;

  KKStr  modelingMethodStr  (SettingValue (sectionNum, "MODELING_METHOD", methodLineNum));
  modelingMethodStr.Upper ();

  log.Level (30) << "ValidateGlobalSection - ModelingMethod[" << Model::ModelTypeToStr (modelingMethod) << "]   LineNum[" << methodLineNum << "]." << endl;

  modelingMethod = ModelTypeFromStr (modelingMethodStr);

  if  (methodLineNum < 0)
  {
    KKStr errMsg = "No Modeling_Method was specified.";
    log.Level (-1) << endl 
                   << "ValidateGlobalSection  ***ERROR***  " << errMsg << endl 
                   << endl;
    FormatErrorsAdd (sectionLineNum, errMsg);
    FormatGood (false);
  }

  if  (modelingMethod == Model::ModelTypes::mtNULL)
  {
    // We have a invalid Modeling Method Parameter.
    KKStr  errMsg = "Invalid Modeling Method[" + modelingMethodStr + "] was specified.";
    log.Level (-1) << endl 
                   << "ValidateGlobalSection   ***ERROR***    " << errMsg << endl
                   << endl;
    FormatErrorsAdd (methodLineNum, errMsg);
    FormatGood (false);
  }

  KKStr  fvFactoryProducerName (SettingValue (sectionNum, "FEATURE_COMPUTER", fvFactoryLineNum));
  if  (fvFactoryLineNum < 0)
  {
    fvFactoryProducerSpecified = false;
    if  (!fvFactoryProducer)
    {
      log.Level (-1) << "No Feature-Computer Specified;  defaulting to 'GrayScaleImages'." << endl;
      fvFactoryProducer = DefaultFeatureVectorProducer (log);
    }
  }
  else
  {
    KKStr  defaultName = "None";
    if  (!fvFactoryProducer)
    {
      fvFactoryProducer = DefaultFeatureVectorProducer (log);
      defaultName = fvFactoryProducer->Name ();
    }
    FactoryFVProducerPtr  factorySpecified = FactoryFVProducer::LookUpFactory (fvFactoryProducerName);
    if  (!factorySpecified)
    {
      log.Level (-1) << endl << "ValidateGlobalSection   ***ERROR***    Factory[" << fvFactoryProducerName << "] on lineNum[" << fvFactoryLineNum << "]  not defined" << endl
        << "  defaulting to: " << defaultName << endl
        << endl;
      fvFactoryProducerSpecified = false;
    }
    else
    {
      fvFactoryProducer = factorySpecified;
      fvFactoryProducerSpecified = true;
    }
  }

  KKStr  rootDirStr  (SettingValue (sectionNum, "ROOT_DIR", rootDirLineNum));
  if  (rootDirLineNum < 0)
  {
    // No Root Directory was specified.
    rootDir = osAddSlash (KKMLVariables::TrainingLibrariesDir ()) + osGetRootName (this->FileName ());
    log.Level (10) << endl
        << "ValidateGlobalSection   'Root_Dir'  Was not specified defaulting to[" << rootDir << "]" << endl
        << endl;
  }
  else
  {
    rootDir = rootDirStr;
    rootDirExpanded = osSubstituteInEnvironmentVariables (rootDir);
    if  (!rootDir.Empty ()  &&  (!osValidDirectory (rootDirExpanded))  &&  validateDirectories)
    {
      KKStr errMsg = "Invalid RootDir[" + rootDirStr + "]  Specified on line[" + StrFormatInt (rootDirLineNum, "ZZZ0") + "].  Expanded to[" + RootDirExpanded () + "]";
      log.Level (-1) << endl 
                     << "ValidateGlobalSection   ***ERROR*** " << errMsg << endl 
                     << endl;
      FormatErrorsAdd (rootDirLineNum, errMsg);
      FormatGood (false);
    }
  }

  kkint32  parametersLineNum        = -1;
  kkint32  featuresIncludedLineNum  = -1;
  kkint32  examplesPerClassLineNum  = -1;

  KKStr  modelParametersStr  = SettingValue (sectionNum, "Parameters",        parametersLineNum);
  KKStr  featuresIncludedStr = SettingValue (sectionNum, "Features_Included", featuresIncludedLineNum);

  if  ((parametersLineNum < 0)  &&  (modelingMethod == Model::ModelTypes::mtOldSVM))
    modelParametersStr = SettingValue ("Model3", "Parameters", parametersLineNum);

  if  ((featuresIncludedLineNum < 0)  &&  (modelingMethod == Model::ModelTypes::mtOldSVM))
    featuresIncludedStr = SettingValue ("Model3", "Features_Included", featuresIncludedLineNum);

  if  (parametersLineNum < 0)
  {
    KKStr errMsg = "'Parameters' setting was not specified.";
    log.Level (-1) << endl 
                   << "ValidateGlobalSection   ***ERROR***   " << errMsg << endl 
                   << endl;
    FormatErrorsAdd (sectionLineNum, errMsg);
    FormatGood (false);
  }

  if  (featuresIncludedStr.Empty ())
    featuresIncludedStr = "All";

  bool validFeaturesNums = false;
  FeatureNumList selFeatures (featuresIncludedStr, validFeaturesNums);
  if  (!validFeaturesNums)
  {
    KKStr  errMsg = "Invalid Features_Included[" + featuresIncludedStr + "].";
    log.Level (-1) << endl 
                   << "ValidateGlobalSection   ***ERROR***   "  << errMsg << endl 
                   << endl;
    FormatErrorsAdd (featuresIncludedLineNum, errMsg);
    FormatGood (false);
  }

  {
    KKStr  examplesPerClassStr = SettingValue (sectionNum, "Examples_Per_Class", examplesPerClassLineNum);
    if  (examplesPerClassStr.Empty ())
      examplesPerClassStr = SettingValue (sectionNum, "Images_Per_Class", examplesPerClassLineNum);

    if  ((examplesPerClassLineNum < 0)  &&  (modelingMethod == Model::ModelTypes::mtOldSVM))
    {
      examplesPerClassStr = SettingValue ("Model3", "Examples_Per_Class", examplesPerClassLineNum);
      if  (examplesPerClassStr.Empty ())
        examplesPerClassStr = SettingValue ("Model3", "Images_Per_Class", examplesPerClassLineNum);
    }

    if  (examplesPerClassStr.Empty ())
      examplesPerClass = int32_max;

    else
      examplesPerClass = examplesPerClassStr.ToInt ();

    if  (examplesPerClass < 1)
    {
      KKStr  errMsg = "Invalid Examples_Per_Class[" + examplesPerClassStr + "].";
      log.Level (-1) << endl 
                     << "ValidateGlobalSection   ***ERROR***   " << errMsg << endl
                     << endl;
      FormatErrorsAdd (examplesPerClassLineNum, errMsg);
      FormatGood (false);
    }
  }

  CreateModelParameters (modelParametersStr, selFeatures, sectionLineNum, parametersLineNum, featuresIncludedLineNum, log);
  if  (modelParameters)
  {
    if  (examplesPerClassLineNum >= 0)
      modelParameters->ExamplesPerClass (examplesPerClass);
    else
      examplesPerClass = modelParameters->ExamplesPerClass ();
  }

  KKStr  otherClassName (SettingValue (sectionNum, "OtherClass", otherClassLineNum));
  if  (otherClassLineNum >= 0)
  {
    if  (otherClassName.Empty ())
    {
      KKStr  errMsg = "OtherClass specified but no name provided.";
      log.Level (-1) << endl << "ValidateGlobalSection   ***ERROR***   " << errMsg << endl;
      FormatErrorsAdd (examplesPerClassLineNum, errMsg);
      FormatGood (false);
    }

    otherClass = MLClass::CreateNewMLClass (otherClassName, -1);
    ValidateOtherClass (otherClass, otherClassLineNum, log);
  }
} /* ValidateGlobalSection */





void   TrainingConfiguration2::ValidateTwoClassParameters (kkint32  sectionNum,
                                                           RunLog&  log
                                                          )
{
  if  (!modelParameters)
  {
    log.Level (-1) << "ValidateTwoClassParameters - Global Parameters must be defined first."
                   << endl;
    FormatGood (false);
  }

  kkint32  class1NameLineNum = -1;
  kkint32  class2NameLineNum = -1;
  kkint32  parameterLinenum  = -1;
  kkint32  weightLineNum     = -1;

  kkint32  sectionLineNum = SectionLineNum (sectionNum);

  KKStr  class1Name   (SettingValue (sectionNum, "CLASS1",     class1NameLineNum));
  KKStr  class2Name   (SettingValue (sectionNum, "CLASS2",     class2NameLineNum));
  KKStr  parameterStr (SettingValue (sectionNum, "PARAMETERS", parameterLinenum));
  KKStr  weightStr    (SettingValue (sectionNum, "WEIGHT",     weightLineNum));

  if  (class1NameLineNum < 0)
  {
    KKStr errMsg = "'Class1' setting was not defined.";
    log.Level (-1) << endl << "ValidateTwoClassParameters   ***ERROR***   " << errMsg << endl << endl;
    FormatErrorsAdd (sectionLineNum, errMsg);
    FormatGood (false);
  }

  if  (class2NameLineNum < 0)
  {
    KKStr errMsg = "'Class2' setting was not defined.";
    log.Level (-1) << endl << "ValidateTwoClassParameters   ***ERROR***   " << errMsg << endl << endl;
    FormatErrorsAdd (sectionLineNum, errMsg);
    FormatGood (false);
  }

  MLClassPtr  class1 = mlClasses->LookUpByName (class1Name);
  MLClassPtr  class2 = mlClasses->LookUpByName (class2Name);

  if  (!class1)
  {
    log.Level (20) << "ValidateTwoClassParameters  LineNum[" << class1NameLineNum << "]  Class[" << class1Name << "] not part of classifier."  << endl;
    return;
  }

  if  (!class2)
  {
    log.Level (20) << "ValidateTwoClassParameters  LineNum[" << class2NameLineNum << "]  Class[" << class2Name << "] not part of classifier."  << endl;
    return;
  }

  FeatureNumListConstPtr  selectedFeatures = DeriveFeaturesSelected (sectionNum);
  if  (!selectedFeatures)
    selectedFeatures = modelParameters->SelectedFeatures ();

  if  (!selectedFeatures)
    selectedFeatures = new FeatureNumList (fileDesc);

  if  ((class1NameLineNum > 0)  &&  (!class1))
  {
    KKStr errMsg = "Class[" + class1Name + "] is not defined.";
    log.Level (-1) << endl << "ValidateTwoClassParameters   ***ERROR***   " << errMsg << endl << endl;
    FormatErrorsAdd (class1NameLineNum, errMsg);
    FormatGood (false);
  }

  if  ((class2NameLineNum > 0)  &&  (!class2))
  {
    KKStr errMsg = "Class[" + class2Name + "] is not defined.";
    log.Level (-1) << endl << "ValidateTwoClassParameters   ***ERROR***   " << errMsg << endl << endl;
    FormatErrorsAdd (class2NameLineNum, errMsg);
    FormatGood (false);
  }

  if  (!FormatGood ())
    return;

  if  (class1  &&  class2)
  {
    float  weight = 1.0f;
    if  (!weightStr.Empty ())
      weight = (float)atof (weightStr.Str ());

    ModelParamOldSVMPtr oldSvmParameters = OldSvmParameters ();
    if  (oldSvmParameters)
    {
      SVM233::svm_parameter binaryParam (oldSvmParameters->Param ());
      if  (!parameterStr.Empty ())
      {
        SVM233::svm_parameter  tempParam (parameterStr);
        binaryParam = tempParam;
      }

      oldSvmParameters->AddBinaryClassParms (class1, class2, binaryParam, selectedFeatures, weight);
    }
  }
  delete  selectedFeatures;
}  /* ValidateTwoClassParameters */







void   TrainingConfiguration2::ValidateConfiguration (RunLog&  log)
{
  // Validate TrainingClasses
  
  this->FormatErrorsClear ();

  kkint32 numOfSections = NumOfSections ();

  for  (kkint32  sectionNum = 0; sectionNum < numOfSections; sectionNum++)
  {
    KKStr  sectionName (SectionName (sectionNum));
    sectionName.Upper ();

    if  (sectionName == "TRAINING_CLASS")
    {
      ValidateTrainingClassConfig (sectionNum, log);
    }

    else if  (sectionName == "NOISE_IMAGES")
    {
       noiseTrainingClass = ValidateClassConfig (sectionNum, log);
       if  (noiseTrainingClass)
       {
         kkint32  noiseGuaranteedSizeLineNum = -1;
         noiseMLClass = noiseTrainingClass->MLClass ();
         noiseMLClass->UnDefined (true);
       }
       else
       {
         log.Level (-1) << "ValidateConfiguration *** Invalid Noise Class Definition ***" 
                        << endl;
         FormatGood (false);
       }
    }

    else if  (sectionName == "GLOBAL")
    {
      ValidateGlobalSection (sectionNum, log);
    }

    else if  ((sectionName == "TWO_CLASS_PARAMETERS")  ||
              (sectionName == "TWOCLASSPARAMETERS")    ||
              (sectionName == "TWO_CLASSPARAMETERS")
             )
    {
      ValidateTwoClassParameters (sectionNum, log);
    }
  }

  if  (trainingClasses.QueueSize () < 1)
  {
    log.Level (-1) << endl << "ValidateConfiguration ***ERROR***    No Training Classes defined." << endl;
    FormatGood (false);
  }


  if  (modelParameters == NULL)
  {
    log.Level (-1) << "ValidateConfiguration ***ERROR***       SVM Parameters not specified." << endl;
    FormatGood (false);
  }

  if  (otherClass != NULL)
  {
    if  (trainingClasses.LocateByMLClass (otherClass) != NULL)
    {
      KKStr  errMsg = "OtherClass[" + otherClass->Name () + "] was also defined as a TrainingClass.";
      log.Level (-1) << endl << "ValidateConfiguration   ***ERROR***    " << errMsg << endl << endl;
      FormatErrorsAdd (otherClassLineNum, errMsg);
      FormatGood (false);
    }
  }

}  /* ValidateConfiguration */






TrainingClassPtr   TrainingConfiguration2::LocateByMLClassName (const KKStr&  className)
{
  return  trainingClasses.LocateByMLClassName (className);
}




void   TrainingConfiguration2::EmptyTrainingClasses ()
{
  while  (trainingClasses.QueueSize () > 0)
  {
    TrainingClassPtr  trainingClass = trainingClasses.PopFromBack ();
    delete  trainingClass;
  }
}  /* DeleteTrainingClasses */





bool  TrainingConfiguration2::NormalizeNominalFeatures ()
{
  ModelParamOldSVMPtr oldSvmParameters = OldSvmParameters ();
  if  (oldSvmParameters)
    return oldSvmParameters->NormalizeNominalFeatures ();

  else if  (modelParameters)
    return modelParameters->NormalizeNominalFeatures ();

  else
    return false;
}  /* NormalizeNominalFeatures */




namespace KKMLL
{
  KKStr&  operator<< (KKStr&                              left,
                      TrainingConfiguration2::ModelTypes  modelingMethod
                     )
  {
    left.Append (TrainingConfiguration2::ModelTypeToStr (modelingMethod));
    return  left;
  }



  ostream&  KKMLL::operator<< (std::ostream&                       os,
                              TrainingConfiguration2::ModelTypes  modelingMethod
                              )
  {
    os << TrainingConfiguration2::ModelTypeToStr (modelingMethod);
    return os;
  }
}




/**
 *@brief  Load training data from the training library directories.
 *@details
 *@code
 **********************************************************************************************  
 * Description:   Loads all the FeatureData from the Training Library.  The images in the     *
 *                sub-directories specified by the 'TrainingClass' entries will be used as the*
 *                primary source.  Data from existing FeatureData files will be used unless   *
 *                they are out of date.  In that case the data will be recomputed from the    *
 *                original images.                                                            *
 *                                                                                            *
 * ExampleFileName: This field that is in each 'FeatureVector' instance will reflect the sub- *
 *                directory from where it was retrieved. Between that and the 'rootDir' field *
 *                you will be able to get the full path to then original image.               *
 **********************************************************************************************  
 *@endcode
 *@param[out] latestImageTimeStamp  Time-stamp of the most current image loaded.
 *@param[out] changesMadeToTrainingLibraries  True if any image needed to have its features recalculated
 *                                            or images were removed or added to a training directory.
 *@param[in]  cancelFlag  Will monitor this flag.  If it gets set to true will terminate the load and return.
 */
FeatureVectorListPtr  TrainingConfiguration2::LoadFeatureDataFromTrainingLibraries (DateTime&  latestImageTimeStamp,
                                                                                    bool&      changesMadeToTrainingLibraries,
                                                                                    bool&      cancelFlag,
                                                                                    RunLog&    log
                                                                                   )
{
  log.Level (10) << "TrainingConfiguration2::LoadFeatureDataFromTrainingLibraries - Starting." << endl;

  bool  errorOccured = false;

  FeatureVectorListPtr  featureData = fvFactoryProducer->ManufacturFeatureVectorList (true, log);

  changesMadeToTrainingLibraries = false;

  //****************************************************************************
  // Make sure that there are no existing *.data files that we are going to use.
  // We need to do this in a separate pass because more than one entry may refer 
  // to the same Class and hence the same *.data file.


  //***********************************************************
  //  We will first extract the Raw features from each Class 

  TrainingClassList::const_iterator  tcIDX;

  DateTime  latestTimeStamp;
  bool      changesMadeToThisTrainingClass = false;

  for  (tcIDX = trainingClasses.begin ();  (tcIDX != trainingClasses.end ()  &&  (!cancelFlag)  &&  (!errorOccured));  tcIDX++)
  {
    const TrainingClassPtr trainingClass = *tcIDX;

    log.Level (20) << "LoadFeatureDataFromTrainingLibraries   Starting on Class[" << trainingClass->Name () << "]." << endl;

    FeatureVectorListPtr  featureDataThisClass = ExtractFeatures (trainingClass, latestTimeStamp, changesMadeToThisTrainingClass, cancelFlag, log);

    if  (!featureDataThisClass)
    {
      log.Level (-1) << endl
                     << "Error Loading Feature data for class[" << trainingClass->Name () << "]." << endl
                     << endl;
      errorOccured = true;
    }
    else
    {
      if  (latestTimeStamp > latestImageTimeStamp)
        latestImageTimeStamp = latestTimeStamp;

      if  (changesMadeToThisTrainingClass)
        changesMadeToTrainingLibraries = true;

      featureData->AddQueue (*featureDataThisClass);
      featureDataThisClass->Owner (false);
    }
    delete  featureDataThisClass;  featureDataThisClass = NULL;
  }


  if  (NoiseTrainingClass ()  &&  (!cancelFlag)  &&  (!errorOccured))
  {
    log.Level (30) << "LoadFeatureDataFromTrainingLibraries  Loading Noise Class [" << NoiseTrainingClass ()->ExpandedDirectory (rootDir, 0) << "]" << endl;
    FeatureVectorListPtr  noiseFeatureData = ExtractFeatures (NoiseTrainingClass (), latestTimeStamp, changesMadeToThisTrainingClass, cancelFlag, log);
    if  (!noiseFeatureData)
    {
      log.Level (-1) << endl
                     << "TrainingConfiguration2::LoadFeatureDataFromTrainingLibraries   ***ERROR***  Error Loading feature data for Noise Images, Directory[" 
                     << NoiseTrainingClass ()->ExpandedDirectory (rootDir, 0) 
                     << "]." 
                     << endl
                     << endl;
      delete  featureData;  featureData = NULL;
      errorOccured = true;
    }
    else
    {
      if  (latestTimeStamp > latestImageTimeStamp)
        latestImageTimeStamp = latestTimeStamp;

      if  (changesMadeToThisTrainingClass)
        changesMadeToTrainingLibraries = true;

      featureData->AddQueue (*noiseFeatureData);
      noiseFeatureData->Owner (false);
      delete  noiseFeatureData;  noiseFeatureData = NULL;
    }
  }

  if  (cancelFlag  ||  errorOccured)
  {
    log.Level (-1) << "LoadFeatureDataFromTrainingLibraries     ***ERROR***    CancelFlag or ErrorOcured  set to 'true'." << endl;
    delete  featureData;  
    featureData = NULL;
  }

  log.Level (20) << "LoadFeatureDataFromTrainingLibraries   Exiting" << endl;
  return  featureData;
}  /*  LoadFeatureDataFromTrainingLibraries */





FeatureVectorListPtr  TrainingConfiguration2::ExtractFeatures (const TrainingClassPtr  trainingClass,
                                                               DateTime&               latestTimeStamp,
                                                               bool&                   changesMade,
                                                               bool&                   cancelFlag,
                                                               RunLog&                 log
                                                              )
{
  log.Level (30) << "TrainingConfiguration2::ExtractFeatures - Extracting Features For Class["
                 << trainingClass->Name () << "],  into file["
                 << trainingClass->FeatureFileName   ()        << "]."
                 << endl;

  FeatureVectorListPtr  extractedExamples = fvFactoryProducer->ManufacturFeatureVectorList (true, log);

  FeatureFileIOPtr  driver = NULL;
  if  (fvFactoryProducer)
    driver = fvFactoryProducer->DefaultFeatureFileIO ();

  for  (kkuint32 zed = 0;  zed < trainingClass->DirectoryCount ();  ++zed)
  {
    KKStr  expandedDir = trainingClass->ExpandedDirectory (rootDir, zed);
    log.Level (30) << "ExtractFeatures  Directory: " << expandedDir << endl;
    FeatureVectorListPtr  extractedExamplesThisDir = driver->FeatureDataReSink 
                 (fvFactoryProducer,
                  expandedDir,
                  trainingClass->FeatureFileName (),
                  trainingClass->MLClass (),
                  true,   //  Make all entries in this directory 'trainingClass->MLClass ()'
                  *mlClasses,
                  cancelFlag,
                  changesMade,
                  latestTimeStamp,
                  log
                 );
  {
    // We now want to reset the ImageFileNames stored in each 'FeatureVector' instance so that it 
      // reflects the sub-directory from where it was retrieved. Between that and the 'rootDir' 
    // field you will be able to get the full path to then original image.

      KKStr subDirName = trainingClass->Directory (zed);
    if  (!subDirName.Empty ())
      subDirName = osAddSlash (subDirName);

    FeatureVectorList::iterator  idx;
      for  (idx = extractedExamplesThisDir->begin ();  idx != extractedExamplesThisDir->end ();  ++idx)
    {
      FeatureVectorPtr  fv = *idx;
      KKStr  newFileName =  subDirName + osGetRootNameWithExtension (fv->ExampleFileName ());
      fv->ExampleFileName (newFileName);
    }
  }

    extractedExamples->AddQueue (*extractedExamplesThisDir);
    extractedExamplesThisDir->Owner (false);
    delete  extractedExamplesThisDir;
    extractedExamplesThisDir = NULL;
  }

  log.Level (30) << "ExtractFeatures    Exiting" << endl;
  return  extractedExamples;
}  /* ExtractFeatures */





/**
 *@brief  Determine the correct configuration file name.
 *@details
 *@code
 *  If  no extension then add ".cfg" to end of file name.
 *
 *  if  Path Provided
 *  {
 *     Return name as submitted with added extension.
 *  }
 *  else
 *  {
 *     a. If file exists in default directory;  then return name with default directory path.
 *     b. If file exists in Training Model Directory  $(HomeDir) + "\\DataFiles\\TrainingModels"
 *        then return with Training Model Directory.
 *     c. Since not found then we will return name passed in with extension.
 *  }
 *@endcode
 */
KKStr   TrainingConfiguration2::GetEffectiveConfigFileName (const  KKStr&  configFileName)
{
  if  (osFileExists (configFileName))
    return  configFileName;

  KKStr  rootNameEithExtension = osGetRootNameWithExtension (configFileName);
  KKStr  extension = osGetFileExtension (configFileName);
  KKStr  path = osGetPathPartOfFile (configFileName);

  KKStr  configFileNameWithExtension = configFileName;
  if  (extension.Empty ())
  {
    configFileNameWithExtension = osRemoveExtension (configFileName) + ".cfg";
  }

  if  (!path.Empty ())
  {
    // Caller is telling us which directory to look in.  In this case there is nothing to do
    return configFileNameWithExtension;
  }

  // See if file exists
  KKStr  configFileNameInDefaultDirectory = osAddSlash (osGetCurrentDirectory ()) + configFileNameWithExtension;
  if  (osFileExists (configFileNameInDefaultDirectory))
    return  configFileNameInDefaultDirectory;

  KKStr  configNameInTrainingModelDir = osAddSlash (KKMLVariables::TrainingModelsDir ()) + configFileNameWithExtension;
  if  (osFileExists (configNameInTrainingModelDir))
  {
    return  configNameInTrainingModelDir;
  }

  return  configFileNameInDefaultDirectory;
}  /* GetEffectiveConfigFileName */



bool  TrainingConfiguration2::ConfigFileExists (const KKStr& _configFileName)
{
  KKStr  effectiveName = GetEffectiveConfigFileName (_configFileName);
  return osFileExists (effectiveName);
}  /* ConfigFileExists */





void   TrainingConfiguration2::DetermineWhatTheRootDirectoryIs ()
{
  TrainingClassList::iterator  idx;

  rootDir = "";

  if  (trainingClasses.QueueSize () < 0)
    return;

  kkuint32 zed = 0;

  VectorKKStr  rootDirParts = osSplitDirectoryPathIntoParts (trainingClasses[0].ExpandedDirectory (rootDir, 0));
  for  (idx = trainingClasses.begin ();  idx != trainingClasses.end ();  idx++)
  {
    TrainingClassPtr  tc = *idx;

    for  (kkuint32  dirIdx = 0;  dirIdx < tc->DirectoryCount ();  ++dirIdx)
    {
      VectorKKStr  parts = osSplitDirectoryPathIntoParts (tc->ExpandedDirectory (rootDir, dirIdx));

      while  (rootDirParts.size () > parts.size ())
        rootDirParts.pop_back ();
    
      zed = 0;
      while  (zed < rootDirParts.size ())
      {
        if  (!(rootDirParts[zed].EqualIgnoreCase (parts[zed])))
          break;
        zed++;
    }

    while  (zed < rootDirParts.size ())
      rootDirParts.pop_back ();
  }
  }


  //***********************************************************************************
  // We need to update the 'trainingClasses' structure before the 'rootDir' variable.
  // This is because the following code that updates the 'TrainingClass' objects in
  // 'trainingClasses' will use the 'rootDir' variable in the 'ExpandedDirectory' 
  // method.
  for  (idx = trainingClasses.begin ();  idx != trainingClasses.end ();  idx++)
  {
    TrainingClassPtr  tc = *idx;

    for  (kkuint32  dirIdx = 0;  dirIdx < tc->DirectoryCount ();  ++dirIdx)
    {
      VectorKKStr  parts = osSplitDirectoryPathIntoParts (tc->ExpandedDirectory (rootDir, dirIdx));

    KKStr newTrainClassSubDir = "";
    for (zed = (kkuint32)rootDirParts.size ();  zed < (kkuint32)parts.size ();  zed++)
      newTrainClassSubDir << parts[zed];

      tc->Directory (dirIdx, newTrainClassSubDir);
    }
  }

  rootDir = "";
  for  (zed = 0;  zed < rootDirParts.size ();  zed++)
    rootDir << rootDirParts[zed];

}  /* DetermineWhatTheRootDirectoryIs */



const
BinaryClassParmsPtr  TrainingConfiguration2::GetParamtersToUseFor2ClassCombo (MLClassPtr  class1,
                                                                              MLClassPtr  class2
                                                                             )  const
{
  const SVMparamPtr  param = SVMparamToUse ();
  if  (!param)
    return NULL;

  return param->GetParamtersToUseFor2ClassCombo (class1, class2);
}  /* BinaryClassParms */



BinaryClassParmsPtr   TrainingConfiguration2::GetBinaryClassParms (MLClassPtr       class1,
                                                                   MLClassPtr       class2
                                                                  )
{
  const SVMparamPtr  param = SVMparamToUse ();
  if  (!param)
    return NULL;

  return param->GetBinaryClassParms (class1, class2);
}



TrainingConfiguration2Ptr  TrainingConfiguration2List::LookUp (const KKStr&  configFileName)  const
{
  KKStr  rootName = KKB::osGetRootName (configFileName);
  TrainingConfiguration2List::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  ++idx)
  {
    TrainingConfiguration2Ptr c = *idx;
    if  (c->ConfigRootName ().EqualIgnoreCase (rootName))
      return c;
  }
  return NULL;
}




map<KKStr,TrainingConfiguration2::Factory*>*  TrainingConfiguration2::registeredFactories = NULL;


TrainingConfiguration2::Factory*   TrainingConfiguration2::factoryInstace = NULL;


TrainingConfiguration2::Factory*   TrainingConfiguration2::FactoryInstace ()
{
  if  (factoryInstace)
    return  factoryInstace;

  GlobalGoalKeeper::StartBlock ();
  factoryInstace = new Factory ();
  RegisterFatory ("TrainingConfiguration2", factoryInstace );
  GlobalGoalKeeper::EndBlock ();
  return  factoryInstace;
}



TrainingConfiguration2Ptr  TrainingConfiguration2::Manufacture 
                                                       (const KKStr&  className,
                                                        const KKStr&  configFileName,
                                                        bool          validateDirectories,
                                                        RunLog&       log
                                                       )
{
  if  (!registeredFactories)
  {
    log.Level (-1)  << endl << "TrainingConfiguration2::Manufacture   ***ERROR***   registeredFactories == NULL Will construct default 'TrainingConfiguration2' instance." << endl << endl;
    TrainingConfiguration2Ptr  config = new TrainingConfiguration2 ();
    config->Load (configFileName, validateDirectories, log);
    return  config;
  }

  GlobalGoalKeeper::StartBlock ();

  TrainingConfiguration2Ptr  newInstance = NULL;

  map<KKStr,Factory*>::const_iterator  idx;
  idx = registeredFactories->find (className);
  if  (idx == registeredFactories->end ())
  {
    log.Level (-1) << endl << "TrainingConfiguration2::Manufacture   Factory: " << className << "  Is not defined;  will return instance of 'TrainingConfiguration2'." << endl << endl;
    newInstance = new TrainingConfiguration2 ();
    newInstance->Load (configFileName, validateDirectories, log);
  }
  else
  {
    newInstance = idx->second->Manufacture (configFileName, validateDirectories, log);
  }

  GlobalGoalKeeper::EndBlock ();
  return  newInstance;
}  /* Manufacture */



void  TrainingConfiguration2::RegisterFatory (const KKStr&  className,
                                              Factory*      factory
                                             )
{
  GlobalGoalKeeper::StartBlock ();
  if  (!registeredFactories)
  {
    registeredFactories = new map<KKStr,Factory*> ();
    atexit (FinalCleanUp);
  }

  map<KKStr,Factory*>::const_iterator  idx;
  idx = registeredFactories->find (className);
  if  (idx == registeredFactories->end ())
    registeredFactories->insert (pair<KKStr,Factory*> (className, factory));
  else
  {
    cerr << endl << "TrainingConfiguration2::RegisterFatory   ***ERROR***   'TrainingConfiguration2::Factory with same name[" << className << "]" << endl << endl;
  }

  GlobalGoalKeeper::EndBlock ();
}




TrainingConfiguration2Ptr  TrainingConfiguration2::Factory::Manufacture 
             (const KKStr&  configFileName,
              bool          validateDirectories,
              RunLog&       log
             )
{
  TrainingConfiguration2Ptr  config = new TrainingConfiguration2 ();
  config->Load (configFileName, validateDirectories, log);
  return  config;
}




void  TrainingConfiguration2::FinalCleanUp ()
{
  delete  registeredFactories;
  registeredFactories = NULL;
}




void  TrainingConfiguration2::WriteXMLFields (ostream& o)  const
{
  if  (!configFileNameSpecified.Empty ())
    XmlElementKKStr::WriteXML (configFileNameSpecified, "ConfigFileNameSpecified", o);

  if  (!configRootName.Empty ())
    XmlElementKKStr::WriteXML (configFileNameSpecified, "ConfigRootName", o);

  XmlElementInt32::WriteXML (examplesPerClass, "ExamplesPerClass", o);

  if  (fvFactoryProducer)
    XmlElementKKStr::WriteXML (fvFactoryProducer->Name (), "FvFactoryProducer", o);

  XmlElementBool::WriteXML (fvFactoryProducerSpecified, "FvFactoryProducerSpecified", o);

  if  (mlClasses)
    XmlElementMLClassNameList::WriteXML (*mlClasses, "MLClasses", o);

  XmlElementKKStr::WriteXML (Model::ModelTypeToStr (modelingMethod), "ModelingMethod", o);

  if  (modelParameters)
    modelParameters->WriteXML ("ModelParameters", o);

  if  (noiseMLClass)
    XmlElementKKStr::WriteXML (noiseMLClass->Name (), "NoiseMLClass", o);

  if  (noiseTrainingClass)
      XmlElementTrainingClass::WriteXML (*noiseTrainingClass, "NoiseTrainingClass", o);

   if  (otherClass)
    XmlElementKKStr::WriteXML (otherClass->Name (), "OtherClass", o);

  if  (!rootDir.Empty ())
    XmlElementKKStr::WriteXML (rootDir, "RootDir", o);

  if  (!rootDirExpanded.Empty ())
    XmlElementKKStr::WriteXML (rootDirExpanded, "RootDirExpanded", o);
  
  XmlElementBool::WriteXML (validateDirectories, "validateDirectories", o);

  trainingClasses.WriteXML ("TrainingClasses", o);

  if  (subClassifiers)
  {
    VectorKKStr  subClassifierList;
    TrainingConfiguration2List::const_iterator  idx;
    for  (idx = subClassifiers->begin (); idx != subClassifiers->end (); ++idx)
    {
      TrainingConfiguration2Ptr  sc = *idx;
      subClassifierList.push_back (sc->ConfigFileNameSpecified ());
    }
    XmlElementVectorKKStr::WriteXML (subClassifierList, "subClassifiers", o);
  }
}  /* WriteXMLFields */





void  TrainingConfiguration2::WriteXML (const KKStr&  varName,
                                        ostream&      o
                                       )  const
{
  XmlTag  startTag ("TrainingConfiguration2", XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);
  o << endl;

  WriteXMLFields (o);

  XmlTag  endTag ("TrainingConfiguration2", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}  /* WriteXML */



XmlTokenPtr  TrainingConfiguration2::ReadXMLBaseToken (XmlTokenPtr  t,
                                                       RunLog&      log
                                                      )
{
  const KKStr&  varName = t->VarName ();
  if  (t->TokenType () == XmlToken::TokenTypes::tokElement)
  {
    bool  tokenFound = true;

    XmlElementPtr  e = dynamic_cast<XmlElementPtr> (t);

    const KKStr&  varName = e->VarName ();
   
    if  (typeid (*e) == typeid (XmlElementKKStr))
    {
      XmlElementKKStrPtr  s = dynamic_cast<XmlElementKKStrPtr> (e);
      if  (s  &&  s->Value ())
      {
        KKStrConstPtr   str = s->Value ();
        if  (varName.EqualIgnoreCase ("ConfigFileNameSpecified"))
          configFileNameSpecified = *str;

        else if  (varName.EqualIgnoreCase ("configRootName"))
          configRootName = *str;

        else if  (varName.EqualIgnoreCase ("FvFactoryProducer"))
          fvFactoryProducer = FactoryFVProducer::LookUpFactory (*str);

        else if  (varName.EqualIgnoreCase ("ModelingMethod"))
          modelingMethod = Model::ModelTypeFromStr (*str);

        else if  (varName.EqualIgnoreCase ("NoiseMLClass"))
          noiseMLClass = MLClass::CreateNewMLClass (*str);

        else if  (varName.EqualIgnoreCase ("otherClass"))
          otherClass = MLClass::CreateNewMLClass (*str);

        else if  (varName.EqualIgnoreCase ("RootDir"))
          rootDir = *str;

        else if  (varName.EqualIgnoreCase ("RootDirExpanded"))
          rootDirExpanded = *str;

        else
          tokenFound = false;
      }
      else
      {
        tokenFound = false;
      }
    }

    else if  (varName.EqualIgnoreCase  ("ExamplesPerClass"))
    {
      XmlElementInt32Ptr  i = dynamic_cast<XmlElementInt32Ptr> (e);
      if  (i)
        examplesPerClass = i->Value ();
    }

    else if  (varName.EqualIgnoreCase  ("MLClasses"))
    {
      XmlElementMLClassNameListPtr  listOfClasses = dynamic_cast<XmlElementMLClassNameListPtr> (e);
      if  (listOfClasses)
      {
        if  (mlClassesWeOwnIt)
          delete  mlClasses;
        mlClasses = listOfClasses->TakeOwnership ();
        mlClassesWeOwnIt = true;
      }
    }

    else if  (varName.EqualIgnoreCase  ("ModelParameters"))
    {
      /** KKKK
       modelParameters
      ***/
    }

    else if  (varName.EqualIgnoreCase  ("NoiseTrainingClass"))
    {
      XmlElementTrainingClassPtr  ntc = dynamic_cast<XmlElementTrainingClassPtr> (e);
      if  (ntc)
      {
        delete  noiseTrainingClass;
        noiseTrainingClass = NULL;
        noiseTrainingClass = ntc->TakeOwnership ();
      }
    }

    else if  (varName.EqualIgnoreCase  ("FvFactoryProducerSpecified"))
    {
      XmlElementBoolPtr  vBool = dynamic_cast<XmlElementBoolPtr> (e);
      if  (vBool)
        fvFactoryProducerSpecified = vBool->Value ();
    }

    else if  (varName.EqualIgnoreCase  ("ValidateDirectories"))
    {
      XmlElementBoolPtr  vd = dynamic_cast<XmlElementBoolPtr> (e);
      if  (vd)
        validateDirectories = vd->Value ();
    }

    else if  (varName.EqualIgnoreCase  ("TrainingClasses"))
    {
      XmlElementTrainingClassListPtr  tcl = dynamic_cast<XmlElementTrainingClassListPtr> (e);
      if  (tcl)
      {
        TrainingClassListPtr  xmlTrainClasses = tcl->TakeOwnership ();
        if  (xmlTrainClasses)
        {
          trainingClasses.DeleteContents ();
          while  (xmlTrainClasses->QueueSize () > 0)
          {
            TrainingClassPtr  tc = xmlTrainClasses->PopFromBack ();
            trainingClasses.PushOnFront (tc);
          }
          delete  xmlTrainClasses;
          xmlTrainClasses = NULL;
        }
      }
    }

    else if  (varName.EqualIgnoreCase  ("SubClassifiers"))
    {
      XmlElementVectorKKStrPtr  sc = dynamic_cast<XmlElementVectorKKStrPtr> (e);
      if  (sc)
      {
        delete subClassifierNameList;
        subClassifierNameList = sc->TakeOwnership ();
      }
    }

    else
    {
      tokenFound = false;
    }

    if  (tokenFound)
    {
      delete  t;
      t = NULL;
    }
  }

  return  t;
}  /* ReadXMLBaseToken */



void  TrainingConfiguration2::ReadXMLPost (RunLog&  log)
{
  if  (!fvFactoryProducer)
    fvFactoryProducer = DefaultFeatureVectorProducer (log);

  fileDesc = fvFactoryProducer->FileDesc ();
  
  TrainingClassList::iterator  idx;
  for  (idx = trainingClasses.begin ();  idx != trainingClasses.end ();  ++idx)
  {
    TrainingClassPtr  tc = *idx;
    if  (!tc->SubClassifierName ().Empty ())
    {
      bool  errorsFound = false;

      TrainingConfiguration2Ptr subClassifeir = 
           ValidateSubClassifier (tc->SubClassifierName (), errorsFound, log);
      tc->SubClassifier (subClassifeir);
    }
  }
}  /* ReadXMLPost */




void   TrainingConfiguration2::ReadXML (XmlStream&      s,
                                        XmlTagConstPtr  tag,
                                        RunLog&         log
                                       )

{
  fileDesc          = NULL;
  fvFactoryProducer = NULL;
  if  (mlClassesWeOwnIt)
    delete  mlClasses;
  mlClasses = NULL;
  otherClass = NULL;
  delete  noiseTrainingClass;  noiseTrainingClass = NULL;
  delete  subClassifiers;      subClassifiers     = NULL;
  trainingClasses.DeleteContents ();

  VectorKKStr*  subClassifierNameList = NULL;

  XmlTokenPtr t = s.GetNextToken (log);
  while  (t)
  {
    t = ReadXMLBaseToken (t, log);
    delete  t;
    t = s.GetNextToken (log);
  }
  ReadXMLPost (log);
}  /* ReadXML */




XmlFactoryMacro(TrainingConfiguration2)

  