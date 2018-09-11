#include  "FirstIncludes.h"

#include  <stdio.h>
#include  <string>
#include  <iostream>
#include  <fstream>
#include  <vector>


#include  "MemoryDebug.h"
#include  "KKBaseTypes.h"

using namespace std;


#include "OSservices.h"
using namespace KKB;

#include "Variables.h"
using namespace  KKLSC;


Variables::Variables ()
{
}



Variables::~Variables ()
{
}


KKStr  Variables::homeDir = "";


void  Variables::SetHomeDir (const KKStr&  _homeDir)
{
  homeDir = _homeDir;
}



KKStr  Variables::HomeDir ()
{
  if  (!homeDir.Empty ())
    return  homeDir;

  KKStrPtr  homeDirEnvVar = osGetEnvVariable ("HomeDir");
  if  (!homeDirEnvVar)
  {
    #ifdef  WIN32
      return  "C:\\Scs";
    #else
      return  "~/Scs";
    #endif
  }
  else
  {
    KKStr  tempHomeDir = *homeDirEnvVar;
    delete  homeDirEnvVar;
    homeDirEnvVar = NULL;
    return  tempHomeDir;
  }
}  /* HomeDir */



KKStr  Variables::ConfigurationDir ()
{
  KKStr  configDir = osAddSlash (osAddSlash (HomeDir ()) + "Configurations");
  return  configDir;
}




KKStr  Variables::ScannerFilesDefaultDir ()
{
  KKStr  scsFilesDefaultDir = osAddSlash (HomeDir ()) + "ScannerFiles";
  return  scsFilesDefaultDir;
}
