#include "FirstIncludes.h"
#include <stdlib.h>
#include <memory>
#include "MemoryDebug.h"
using namespace  std;


#include "KKBaseTypes.h"
#include "OSservices.h"
using namespace  KKB;


#include "KKMLVariables.h"
using namespace  KKMLL;



KKMLVariables::KKMLVariables ()
{
}



KKMLVariables::~KKMLVariables ()
{
}



KKStr  KKMLVariables::machineLearningHomeDir;



const KKStr&  KKMLVariables::MachineLearningHomeDir ()
{
  if  (machineLearningHomeDir.Empty ())
  {
    #if  defined(KKOS_WINDOWS)
      machineLearningHomeDir = "C:\\KKMLL";
    #else
      machineLearningHomeDir = "/KKMLL";
    #endif

    KKStrPtr  homeDir = osGetEnvVariable ("KKMLModelDir");
    if (homeDir == NULL)
      homeDir = osGetEnvVariable ("KKMLL");
    if (homeDir == NULL)
      homeDir = osGetEnvVariable ("MachineLearningDir");

    if  (homeDir)
      machineLearningHomeDir = *homeDir;

    delete  homeDir;
    homeDir = NULL;
  }
  return  machineLearningHomeDir;
}  /* MachineLearningHomeDir */



void   KKMLVariables::SetMachineLearninigHomeDir (const KKStr&  _machineLearningHomeDir)
{
  machineLearningHomeDir = _machineLearningHomeDir;
}



KKStr  KKMLVariables::TempDir ()
{
  return  osAddSlash (MachineLearningHomeDir ()) + "Temp";
}



KKStr  KKMLVariables::TrainingLibrariesDir ()
{
  return  osAddSlash (MachineLearningHomeDir ()) + "TrainingLibraries";
}



KKStr  KKMLVariables::TrainingModelsDir ()
{
  return  osAddSlash (MachineLearningHomeDir ()) + "TrainingModels";
}

