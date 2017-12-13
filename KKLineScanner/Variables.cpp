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
    
  return  homeDir;
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



kkint64  Variables::LocateEnvStrStart (const KKStr&  str)
{
  kkStrUint  x = 0;
  kkStrUint  y = 1;
  kkStrUint  len = str.Len ();
  const char*  s = str.Str ();

  while  (y < len)
  {
    if  (s[y] == 0)
      return -1;

    if  (s[x] == '$')
    {
      if  ((s[y] == '(')  ||  (s[y] == '{')  ||  (s[y] == '['))
        return  x;
    }

    x++;
    y++;
  }

  return  -1;
}  /* LocateEnvStrStart */




KKStr  Variables::SubstituteInEnvironmentVariables (const KKStr&  src)
{
  kkint64  x = LocateEnvStrStart (src);
  if  (x < 0)
    return  src;

  char  startChar = src[x + 1];
  char  endChar = ')';
  if  (startChar == '(')
    endChar = ')';

  else if  (startChar == '{')
    endChar = '}';

  else if  (startChar == '[')
    endChar = ']';

  KKStr  str (src);

  while  (x >= 0)
  {
    KKStr  beforeEnvStr = str.SubStrPart (0, x - 1);
    str = str.SubStrPart (x + 2);
    x = str.LocateCharacter (endChar);
    if  (x < 0)
      return  src;

    KKStr  envStrName   = str.SubStrPart (0, x - 1);
    KKStr  afterStrName = str.SubStrPart (x + 1);

    KKStrPtr envStrValue = osGetEnvVariable (envStrName);
    if  (envStrValue == NULL)
    {
      if  (envStrName.EqualIgnoreCase ("HomeDir"))
        envStrValue = new KKStr (Variables::HomeDir ());
    }

    if  (envStrValue == NULL)
      return  src;

    str = beforeEnvStr + (*envStrValue)  + afterStrName;
    delete  envStrValue;
    x = LocateEnvStrStart (str);
  }

  return  str;
}  /* SubstituteInEnvironmentVariables */

