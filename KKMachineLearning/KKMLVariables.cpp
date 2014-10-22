#include "FirstIncludes.h"
#include <stdlib.h>
#include <memory>
#include "MemoryDebug.h"
using namespace  std;


#include "KKBaseTypes.h"
#include "OSservices.h"
using namespace  KKB;


#include "KKMLVariables.h"
using namespace  KKMachineLearning;



KKMLVariables::KKMLVariables ()
{
}



KKMLVariables::~KKMLVariables ()
{
}



KKStr  KKMLVariables::machineLearningHomeDir = "";



const KKStr&  KKMLVariables::MachineLearningHomeDir ()
{
  if  (machineLearningHomeDir.Empty ())
  {
    #if  defined(OS_WINDOWS)
      machineLearningHomeDir = "C:\\KKMachineLearning";
    #else
      machineLearningHomeDir = "/KKMachineLearning";
    #endif

    KKStrPtr  homeDir = osGetEnvVariable ("KKMLModeDir");
    if (homeDir == NULL)
      homeDir = osGetEnvVariable ("KKMachineLearning");
    if (homeDir == NULL)
      homeDir = osGetEnvVariable ("MachineLearningDir");

    if  (homeDir)
      machineLearningHomeDir = *homeDir;
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

