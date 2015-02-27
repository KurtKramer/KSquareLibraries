/* OSservices.cpp -- O/S related functions,  meant to be O/S neutral
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#include  "FirstIncludes.h"

#if  defined(OS_WINDOWS)
#include <direct.h>
#include <windows.h>
#include <Lmcons.h>
#include <conio.h>
#else
#include <dirent.h>
#include <sys/types.h>
#include <sys/times.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <iostream>
#include <fstream> 
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include "MemoryDebug.h"
using namespace std;


#include "OSservices.h"
#include "ImageIO.h"
using namespace KKU;



KKStr  KKU::osGetErrorNoDesc (int32  errorNo)
{
  KKStr  desc;

# ifdef WIN32
#   ifdef  USE_SECURE_FUNCS
      char buff[100];
      buff[0] = 0;
      strerror_s (buff, sizeof (buff), errorNo);
      desc = buff;
#   else
      const char* buff = _sys_errlist[errorNo];
      if  (buff)
        desc = buff;
#   endif
# else
   const char*  buff = strerror (errorNo);
   if  (buff)
     desc = buff;
# endif

  return  desc;
}  /* osGetErrorNoDesc */






FILE*  KKU::osFOPEN (const char* fileName, 
                     const char* mode
					          )
{
  FILE*  f = NULL;

# ifdef  USE_SECURE_FUNCS
    fopen_s (&f, fileName, mode);
# else
    f = fopen (fileName, mode);
# endif

  return  f;
}




//***************************** ParseSearchSpec ********************************
//*  Will parse the string searchSpec into seperate fields using '*' as the    *
//*  delimiter character.                                                      *
//*                                                                            *
//*  ex: input:   searchSpec = "Alpha*Right*.txt"                              *
//*      returns: ("Alpha", "*", "Right", "*", ".txt")                         *
//*                                                                            *
//*  The caller is responsible for deleting the StrigList that is returned      *
//*  when no longer needed.                                                    *
//*                                                                            *
//******************************************************************************
KKStrListPtr  osParseSearchSpec (const KKStr&  searchSpec)
{
  KKStrListPtr  searchFields = new KKStrList (true, 10);

  if  (searchSpec.Empty ())
  {
    searchFields->PushOnBack (new KKStr ("*"));
    return  searchFields;
  }

# ifdef  USE_SECURE_FUNCS
    char*  workStr = _strdup (searchSpec.Str ());
# else
    char*  workStr = strdup (searchSpec.Str ());
# endif
  
  char*  cp = workStr;

  char*  startOfField = cp;

  while  (*cp != 0)
  {
    if  (*cp == '*')
    {
      *cp = 0;
      if  (startOfField != cp)
         searchFields->PushOnBack (new KKStr (startOfField));

      searchFields->PushOnBack (new KKStr ("*"));
      startOfField = cp + 1;
    }
    cp++;
  }
  
  if  (cp != startOfField)
  {
    // There is one last field that we need to add,  meaning that there was no '*'s  or
    // that the last char in the searchField was not a '*'.
    searchFields->PushOnBack (new KKStr (startOfField));
  }

  delete  workStr;

  return  searchFields;
}  /*  osParseSearchSpec */




bool  osFileNameMatchesSearchFields (const KKStr&  fileName,
                                     KKStrListPtr  searchFields
                                    )
{
  if  (!searchFields)
    return  true;

  if  (searchFields->QueueSize () < 1)
    return true;

  KKStrConstPtr  field = searchFields->IdxToPtr (0);

  if  (searchFields->QueueSize () == 1)
  {
    if  (*field == "*")
      return  true;

    if  (*field == fileName)
      return true;
    else
      return false;
  }

 
  bool  lastFieldAStar = false;

  const char*  cp = fileName.Str ();
  int32        lenLeftToCheck = fileName.Len ();


  int32  fieldNum;

  for  (fieldNum = 0;  fieldNum < searchFields->QueueSize ();  fieldNum++)
  {
    const KKStr&  field = searchFields->IdxToPtr (fieldNum);
    
    if  (field == "*")
    {
      lastFieldAStar = true;
    }
    else
    {
      if  (lastFieldAStar)
      {
        // Since last was a '*'  then we can skip over characters until we find a sequence that matches field

        bool  matchFound = false;

        while  ((!matchFound)  &&  (lenLeftToCheck >= field.Len ()))
        {
          if  (strncmp (cp, field.Str (), field.Len ()) == 0)
          {
            matchFound = true;
            // lets move cp beyound wheer we found field in fileName
            cp = cp + field.Len ();
            lenLeftToCheck = lenLeftToCheck - field.Len ();
          }
          else
          {
            cp++;
            lenLeftToCheck--;
          }
        }

        if  (!matchFound)
        {
          // There was no match any were else in the rest of the fileName.
          // way that this fileName is a match.
          return  false;
        }
      }
      else
      {
        // Since last field was  NOT   a '*' the next field->Len () characters better be an exact match
        if  (lenLeftToCheck < field.Len ())
        {
          // Not enough chars left in fileName to check,  can not possibly be a match
          return  false;
        }

        if  (strncmp (cp, field.Str (), field.Len ()) != 0)
        {
          // The next field->Len ()  chars were not a match.  This means that fileName is
          // not a match.
          return  false;
        }
        else
        {
          cp = cp + field.Len ();
          lenLeftToCheck = lenLeftToCheck - field.Len ();
        }
      }

      lastFieldAStar = false;
    }
  }


  if  (lenLeftToCheck > 0)
  {
    // Since there are some char's left in fileName that we have not checked,  then 
    // the last field had better been a '*'
    if  (lastFieldAStar)
      return true;
    else
      return false;
  }
 
  return true;
}  /* osFileNameMatchesSearchFields */






/**
 *@brief  Given a full drive with path and name return the drive letter specified.  If
 * not on a Windows system returns x'00'= char(0).
 */
char  KKU::osGetDriveLetter (KKStr  fileName)
{
#ifndef  WIN32
  return 0;
#else
  if  (fileName.Len () < 3)
    return 0;

  if  (fileName[(int16)1] == ':')
    return  fileName[(int16)0];

  return 0;
#endif
}  /* osGetDriveLetter */







#ifdef WIN32
KKStr  KKU::osGetCurrentDirectory ()
{
  DWORD    buffLen = 0;
  char*    buff = NULL;
  DWORD    reqLen = 1000;

  while  (reqLen > buffLen)
  {
    buffLen = reqLen;

    if  (buff)
      delete buff;

    buff = new char[buffLen];
 
    reqLen = GetCurrentDirectory (buffLen, buff);
  }

  KKStr  curDir (buff);

  delete buff;   

  return  curDir;
}  /* GetCurrentDirectory */

#else
 




KKStr  KKU::osGetCurrentDirectory ()
{
  size_t   buffLen = 0;
  char*    buff    = NULL;
  char*    result  = NULL;


  while  (!result)
  {
    buffLen = buffLen + 1000;

    if  (buff)
      delete buff;

    buff = new char[buffLen];
 
    result = getcwd (buff, buffLen);
  }

  KKStr  curDir (buff);

  delete buff;   

  return  curDir;
}  /* ossGetCurrentDirectory */
#endif





#ifdef WIN32
bool  KKU::osValidDirectory (KKStrConstPtr  name)
{
  DWORD  returnCd = GetFileAttributes (name->Str ());

  if  (returnCd == 0xFFFFFFFF)
  {
    return false;
  }

  if  ((FILE_ATTRIBUTE_DIRECTORY & returnCd)  != FILE_ATTRIBUTE_DIRECTORY)
    return false;
  else
    return true;
}


bool  KKU::osValidDirectory (const KKStr&  name)
{
  #define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
  DWORD  returnCd = GetFileAttributes (name.Str ());

  if  (returnCd == INVALID_FILE_ATTRIBUTES)
    return false;


  if  ((FILE_ATTRIBUTE_DIRECTORY & returnCd)  != FILE_ATTRIBUTE_DIRECTORY)
    return false;
  else
    return true;
}

#else

bool  KKU::osValidDirectory (KKStrConstPtr  name)
{
  DIR*  openDir = opendir (name->Str ());

  if  (!openDir)
    return  false;

  closedir (openDir);
  return  true;
}



bool  KKU::osValidDirectory (const KKStr&  name)
{
  DIR*  openDir = opendir (name.Str ());

  if  (!openDir)
    return  false;

  closedir (openDir);
  return  true;
}

#endif



bool   KKU::osValidFileName (const KKStr&  _name)
{
  KKStrListPtr  errors = osValidFileNameErrors (_name);
  if  (errors == NULL)
    return true;
  else
  {
    delete errors;
    errors = NULL;
    return false;
  }
}



KKStrListPtr  KKU::osValidFileNameErrors (const KKStr&  _name)
{
  const char*  invalidChars = "\\\" :<>!?*";
  const char*  invalidNames[] = {"CON",  "PRN",  "AUX",  "NUL",  "COM1", "COM2", 
                                 "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", 
                                 "COM9", "LPT1", "LPT2", "LPT3", "LPT4", "LPT5",
                                 "LPT6", "LPT7", "LPT8", "LPT9", 
                                 NULL
                                }; 

  KKStrListPtr  errors = new KKStrList (true);
  if  (_name.Empty ())
  {
    errors->PushOnBack (new KKStr ("Blank names are invalid!"));
  }

  else
  {
    // Check for invalid names.
    kkint32  x = 0;
    while  (invalidNames[x] != NULL)
    {
      if  (_name.EqualIgnoreCase (invalidNames[x]))
        errors->PushOnBack (new KKStr ("Can not use \"" + _name + "\""));
    }

    // Check for invalid charcters
    for  (x = 0;  x < _name.Len ();  ++x)
    {
      char c = _name[x];
      if  (c == 0)
        errors->PushOnBack (new KKStr ("Null character at position: " + StrFormatInt (x, "##0")));

      else if  (c == ' ')
        errors->PushOnBack (new KKStr ("No spaces allowed."));

      else if  (c < 32)
      {
        errors->PushOnBack (new KKStr ("Character at position: " + StrFormatInt (x, "##0") + " has ordinal value of: " + StrFormatInt (c, "##0")));
      }

      else if  (strchr (invalidChars, c) != NULL)
      {
        KKStr invalidStr (2);
        invalidStr.Append (c);
        errors->PushOnBack (new KKStr ("Invalid character: " + invalidStr + "  at position: " + StrFormatInt (x, "##0")));
      }
    }
  }

  if  (errors->QueueSize () < 1)
  {
    delete  errors;
    errors = NULL;
  }

  return errors;
}  /* osValidFileNameErrors */




bool  KKU::osDeleteFile (KKStr  _fileName)
{
  #ifdef  WIN32
 
  const char*  fileName = _fileName.Str ();

  if  (DeleteFile (fileName))
  {
    return  true;
  }
  else
  {
    DWORD fileAttributes = GetFileAttributes (fileName);
    fileAttributes = FILE_ATTRIBUTE_NORMAL;
    if  (!SetFileAttributes (fileName, fileAttributes))
    {
      return  false;
    }

    else
    {
      // At this point we can either delete this file or not.
      return  (DeleteFile (fileName) != 0);
    }
  }


  #else
  int32  returnCd;

  // We are in Unix Environment.
  returnCd = unlink (_fileName.Str ());
  return  (returnCd == 0);
  #endif  
}  /* DeleteFile */








#ifdef WIN32

bool  KKU::osFileExists (const KKStr&  _fileName)
{
  const char* fileName = _fileName.Str ();

  #define INVALID_FILE_ATTRIBUTES ((DWORD)-1)

  DWORD fileAttributes = GetFileAttributes (fileName);
  if  (fileAttributes == INVALID_FILE_ATTRIBUTES)
  {
    // Error getting Attributes.  File may not exist.
      DWORD  lastErrorNum = GetLastError ();
      if  ((lastErrorNum == ERROR_FILE_NOT_FOUND)  ||  (lastErrorNum == 3))
        return false;
  }

  return  true;
}


#else


bool  KKU::osFileExists (const KKStr&  _fileName)
{
  struct  stat  buff;
  int32         result;

  result  = stat (_fileName.Str (), &buff);
  if  (result == 0)
     return  true;
  else
     return  false;
}

#endif






bool  KKU::osMoveFileBetweenDirectories (const KKStr&  _fileName,
                                         const KKStr&  _srcDir,
                                         const KKStr&  _destDir
                                        )
{
  #ifdef  WIN32

    KKStr  srcName (_srcDir);
    KKStr  destName (_destDir);

    if  (srcName.LastChar () != '\\')
      srcName  << "\\";
    
    if  (destName.LastChar () != '\\')
      destName << "\\";

    srcName   << _fileName;
    destName  << _fileName;

    return  (MoveFile (srcName.Str (), destName.Str ()) != 0);

  #else
    cerr << std::endl;
    cerr << "*** osMoveFileBetweenDirectories ***" << std::endl;
    cerr << std::endl;
    cerr << "*** Not yet implemented ***" << std::endl;
    osWaitForEnter ();
    exit (-1);
  #endif
}



#ifdef  WIN32
bool  KKU::osCopyFileBetweenDirectories (const KKStr&  _fileName,
                                         const KKStr&  _srcDir,
                                         const KKStr&  _destDir
                                        )
{
  KKStr  existingFile (_srcDir);
  KKStr  destinationName (_destDir);
  BOOL    fail = 1;
  
  osAddLastSlash (existingFile);
  existingFile    <<  _fileName;

  osAddLastSlash (destinationName);
  destinationName << _fileName;
  
  bool  result = (CopyFile (existingFile.Str (),
                            destinationName.Str (),
                            fail)  != 0);

  if  (result)
    return true;
  else 
    return false;
}

#else

bool  KKU::osCopyFileBetweenDirectories (const KKStr&  _fileName,
                                         const KKStr&  _srcDir,
                                         const KKStr&  _destDir
                                        )
{
  cerr << std::endl;
  cerr << "*** osCopyFileBetweenDirectories ***" << std::endl;
  cerr << std::endl;
  cerr << "*** Not yet implemented ***" << std::endl;
  osWaitForEnter ();
  exit (-1);
}

#endif




#ifdef  WIN32
bool  KKU::osCopyFile (const KKStr&  srcFileName,
                       const KKStr&  destFileName
                      )
{
  BOOL    fail = 1;
  
  bool  result = (CopyFile (srcFileName.Str (),
                            destFileName.Str (),
                            fail)  != 0);


  if  (result)
  {
    return true;
  }

  else 
  {
    DWORD lastError = GetLastError ();

    KKStr  errorMessage = StrFormatInt (lastError, "#,###,##0");

    cerr << std::endl
         << "osCopyFile    *** ERROR ***"                          << std::endl
         << "               srcFileName [" << srcFileName  << "]"  << std::endl
         << "               destFileName[" << destFileName << "]"  << std::endl
         << "               Error Code  [" << lastError    << "]"  << std::endl
         << "               Error Msg   [" << errorMessage << "]"  << std::endl;
    return false;
  }
}

#else



bool  KKU::osCopyFile (const KKStr&  srcFileName,
                       const KKStr&  destFileName
                      )
{
  cerr << std::endl;
  cerr << "*** osCopyFile ***" << std::endl;
  cerr << std::endl;
  cerr << "*** Not yet implemented ***" << std::endl;
  osWaitForEnter ();
  exit (-1);
}

#endif



#ifdef  WIN32

bool  KKU::osRenameFile (const KKStr&  oldName,
                         const KKStr&  newName
                        )
{
  bool  returnCd = (MoveFile (oldName.Str (), newName.Str ()) != 0);

  if  (!returnCd)
  {
     DWORD lastError = GetLastError ();

     cerr << std::endl
          << "osRenameFile   *** ERROR ***,   Rename Failed"                          << std::endl
          <<                                                                             std::endl
          << "               oldName[" << oldName << "]   NewName[" << newName << "]" << std::endl
          << "               LastError[" << lastError << "]"                          << std::endl
          << std::endl;
     
  }
 
  return  returnCd;

}  /* osRenameFile */
#else


bool  KKU::osRenameFile (const KKStr&  oldName,
                         const KKStr&  newName
                        )
{
  
  int32  returnCd = rename (oldName.Str (), newName.Str ());

  if  (returnCd == 0)
    return true;

  int32  errorCode = errno;

  cerr << std::endl
       << "osRenameFile   *** ERROR ***,   Rename Failed"                          << std::endl
       << "               oldName[" << oldName << "]   NewName[" << newName << "]" << std::endl
       << "               errno[" << errorCode << "]"                              << std::endl
       << std::endl;

  return  false;
}  /* osRenameFile */
#endif




void  KKU::osChangeDir (const KKStr&  dirName,
                        bool&          successful
                       )
{
#ifdef  WIN32


  BOOL  changeOk = SetCurrentDirectory(dirName.Str ());
  if  (changeOk)
    successful = true;
  else
    successful = false;

#else
  int32 errorCd = chdir (dirName.Str ());
  successful = (errorCd == 0);

  if  (!successful)
  {
    cerr << std::endl << std::endl << std::endl
         << "osChangeDir  *** ERROR ***   DirPath[" << dirName << "]" << std::endl
         << std::endl;
  }

#endif


  return;
}  /* osChangeDir */



bool  KKU::osCreateDirectoryPath (KKStr  _pathName)
{
  KKStr  nextPartOfPath;

  if  (_pathName.FirstChar () == DSchar)
  {
    nextPartOfPath = DS;
    _pathName = _pathName.SubStrPart (1);
    nextPartOfPath << _pathName.ExtractToken (DS);
  }
  else
  {
    nextPartOfPath = _pathName.ExtractToken (DS);
  }

  KKStr  pathNameSoFar;

  while  (!nextPartOfPath.Empty ())
  {
    if  (!pathNameSoFar.Empty ())
    {
      if  (pathNameSoFar.LastChar () != DSchar)
        pathNameSoFar << DS;
    }

    pathNameSoFar << nextPartOfPath;

    if  (!KKU::osValidDirectory (pathNameSoFar))
    {
      bool  createdSucessfully = osCreateDirectory (pathNameSoFar);
      if  (!createdSucessfully)
      {
        cerr << std::endl 
             << "osCreateDirectoryPath   Error Creating Directory[" << pathNameSoFar << "]"
             << std::endl;
        return  false;
      }
    }

    nextPartOfPath = _pathName.ExtractToken (DS);
  }


  return  true;
}  /* osCreateDirectoryPath */





bool  KKU::osCreateDirectory (const KKStr&  _dirName)
{
  #ifdef  WIN32
    return  (CreateDirectory (_dirName.Str (), NULL) != 0);
  
  #else  
      
    int32  result;

    mode_t mode  = S_IRWXU + S_IRWXG;

    result = mkdir (_dirName.Str (), mode);
    return result == 0;

  #endif 
}




#ifdef  WIN32
 
KKStrPtr  KKU::osGetEnvVariable (KKStr  _varName)
{
  char    buff[1024];
  DWORD   varLen;

  varLen = GetEnvironmentVariable (_varName.Str (), buff, sizeof (buff));

  if  (varLen == 0)
  {
    return NULL;
  }
  else
  {
    return  new KKStr (buff);
  }
} /* GetEnvVariable */

#else


KKStrPtr  KKU::osGetEnvVariable (KKStr  _varName)
{
  char*  envStrValue = NULL;
  
  envStrValue = getenv (_varName.Str ());

  if  (envStrValue)
    return  new KKStr (envStrValue);
  else
    return NULL;

} /* GetEnvVariable */



#endif


int32  KKU::osLocateLastSlashChar (const KKStr&  fileSpec)
{
  int32  lastLeftSlash  = fileSpec.LocateLastOccurrence ('\\');
  int32  lastRightSlash = fileSpec.LocateLastOccurrence ('/');

  return  Max (lastLeftSlash, lastRightSlash);
}  /* LastSlashChar */



int32  KKU::osLocateFirstSlashChar (const KKStr&  fileSpec)
{
  int32  firstForewardSlash  = fileSpec.LocateCharacter ('/');
  int32  firstBackSlash = fileSpec.LocateCharacter ('\\');

  if  (firstForewardSlash < 0)
    return firstBackSlash;

  else if  (firstBackSlash < 0)
    return firstForewardSlash;

  return  Min (firstForewardSlash, firstBackSlash);
}  /* LastSlashChar */





void  KKU::osAddLastSlash (KKStr&  fileSpec)
{
  char  c = fileSpec.LastChar ();

  if  ((c != '\\')  &&  (c != '/'))
    fileSpec << DS;
}  /* osAddLastSlash */



KKStr  KKU::osAddSlash (const KKStr&  fileSpec)
{
  KKStr  result (fileSpec);
  if  ((result.LastChar () != '\\')  &&  (result.LastChar () != '/'))
     result << DS;
  return  result;
}  /* OsAddLastSlash */




KKStr  KKU::osMakeFullFileName (const KKStr&  _dirName, 
                                const KKStr&  _fileName
                               )
{
  KKStr  fullFileName (_dirName);

  osAddLastSlash (fullFileName);

  fullFileName << _fileName;

  return  fullFileName;
}



KKStr  KKU::osGetDirNameFromPath (KKStr  dirPath)
{
  if  (dirPath.LastChar () == DSchar)
    dirPath.ChopLastChar ();

  KKStr  path, root, ext;
  osParseFileName (dirPath, path, root, ext);
  
  if  (ext.Empty ())
    return  root;
  else
    return  root + "." + ext;
}  /* osGetDirNameFromPath */



void   KKU::osParseFileName (KKStr   _fileName, 
                             KKStr&  _dirPath,
                             KKStr&  _rootName, 
                             KKStr&  _extension
                            )
{
  int32  x;
  
  x = osLocateLastSlashChar (_fileName);

  if  (x < 0)
  {
    _dirPath = "";
  }

  else
  {
    _dirPath  = _fileName.SubStrPart (0, x - 1);
    _fileName = _fileName.SubStrPart (x + 1);
  }
  
      
  x = _fileName.LocateLastOccurrence ('.');
  if  (x < 0)
  {
    _rootName  = _fileName;
    _extension = "";
  }
  else
  {
    _rootName  = _fileName.SubStrPart (0, x - 1);
    _extension = _fileName.SubStrPart (x + 1);
  }

  return;
}  /* ParseFileName */



KKStr  KKU::osRemoveExtension (const KKStr&  _fullFileName)
{
  int32  lastSlashChar = osLocateLastSlashChar (_fullFileName);
    
  int32  lastPeriodChar = _fullFileName.LocateLastOccurrence ('.');

  if  (lastPeriodChar < lastSlashChar)
    return KKStr (_fullFileName);

  if  (lastPeriodChar <= 1)
    return KKStr (_fullFileName);

  return _fullFileName.SubStrPart (0, lastPeriodChar - 1);
}  /* osRemoveExtension */




KKStr  KKU::osGetRootName (const KKStr&  fullFileName)
{
  int32  lastSlashChar = osLocateLastSlashChar (fullFileName);
  int32  lastColon     = fullFileName.LocateLastOccurrence (':');

  int32  lastSlashOrColon = Max (lastSlashChar, lastColon);
 
  KKStr  lastPart;
  if  (lastSlashOrColon < 0)
  {
    lastPart = fullFileName;
  }
  else
  {
    lastPart = fullFileName.SubStrPart (lastSlashOrColon + 1);
  }

  int32  period = lastPart.LocateLastOccurrence ('.');

  if  (period < 0)
    return  lastPart;

  return  lastPart.SubStrPart (0, period - 1);
}  /*  osGetRootName */





KKStr  KKU::osGetRootNameOfDirectory (KKStr  fullDirName)
{
  if  (fullDirName.LastChar () == DSchar)
    fullDirName.ChopLastChar ();

  int32  lastSlashChar = osLocateLastSlashChar (fullDirName);
  int32  lastColon     = fullDirName.LocateLastOccurrence (':');

  int32  lastSlashOrColon = Max (lastSlashChar, lastColon);
 
  KKStr  lastPart;
  if  (lastSlashOrColon < 0)
    lastPart = fullDirName;
  else
    lastPart = fullDirName.SubStrPart (lastSlashOrColon + 1);

  return  lastPart;
}  /*  osGetRootNameOfDirectory */



KKStr  KKU::osGetParentDirectoryOfDirPath (KKStr  path)
{
  if  (path.LastChar () == DSchar) 
    path.ChopLastChar ();

  int32  x1 = path.LocateLastOccurrence (DSchar);
  int32  x2 = path.LocateLastOccurrence (':');

  int32  x = Max (x1, x2);
  if  (x < 1)
    return KKStr::EmptyStr ();

  return  path.SubStrPart (0, x);
}  /* osGetParentDirectoryOfDirPath */



KKStr  KKU::osGetRootNameWithExtension (const KKStr&  fullFileName)
{
  int32  lastSlashChar = osLocateLastSlashChar (fullFileName);
  int32  lastColon     = fullFileName.LocateLastOccurrence (':');

  int32  lastSlashOrColon = Max (lastSlashChar, lastColon);
 
  KKStr  lastPart;
  if  (lastSlashOrColon < 0)
  {
    lastPart = fullFileName;
  }
  else
  {
    lastPart = fullFileName.SubStrPart (lastSlashOrColon + 1);
  }

  return  lastPart;
}  /* osGetRootNameWithExtension */





void  KKU::osParseFileSpec (KKStr   fullFileName,
                            KKStr&  driveLetter,
                            KKStr&  path,
                            KKStr&  root,
                            KKStr&  extension
                           )
{
  path = "";
  root = "";
  extension = "";
  driveLetter = "";

  // Look for Drive Letter
  int32  driveLetterPos = fullFileName.LocateCharacter (':');
  if  (driveLetterPos >= 0)
  {
    driveLetter  = fullFileName.SubStrPart (0, driveLetterPos - 1);
    fullFileName = fullFileName.SubStrPart (driveLetterPos + 1);
  }

  KKStr  fileName;

  if  (fullFileName.LastChar () == DSchar)
  {
    // FileSpec must look like  'c:\xxx\xx\'
    path      = fullFileName;
    root      = "";
    extension = "";
    return;
  }

  int32  lastSlash =  osLocateLastSlashChar (fullFileName);
  if  (lastSlash < 0)
  {
    path = "";
    fileName = fullFileName;
  }
  else 
  {
    path     = fullFileName.SubStrPart (0, lastSlash - 1);
    fileName = fullFileName.SubStrPart (lastSlash + 1);
  }

  int32  period = fileName.LocateLastOccurrence ('.');
  if  (period <= 0)
  {
    root = fileName;
    extension = "";
  }
  else 
  {
    root      = fileName.SubStrPart (0, period - 1);
    extension = fileName.SubStrPart (period + 1);
  }

  return;
}  /* osParseFileSpec */





KKStr  KKU::osGetPathPartOfFile (KKStr  fullFileName)
{
  int32  lastSlash =  osLocateLastSlashChar (fullFileName);

  if  (lastSlash >= 0)
  {
    return  fullFileName.SubStrPart (0, lastSlash - 1);
  }

  int32  lastColon = fullFileName.LocateLastOccurrence (':');
  if  (lastColon >= 0)
    return  fullFileName.SubStrPart (0, lastColon);
  else
    return  KKStr ("");
}  /* GetPathPartOfFile */



KKStr    KKU::osGetFileExtension (KKStr  fullFileName)
{
  KKStr   fileName, dirPath, rootName, extension;
  osParseFileName (fullFileName, dirPath, rootName, extension);
  return  extension;
}  /* osGetFileExtension */




KKStr  KKU::osGetParentDirPath (KKStr  dirPath)
{
  if  ((dirPath.LastChar () == '\\')  ||  (dirPath.LastChar () == '/'))
    dirPath.ChopLastChar ();
  
  int x = Max (dirPath.LocateLastOccurrence ('\\'), dirPath.LocateLastOccurrence ('/'));
  
  if  (x < 0)
  {
    x = dirPath.LocateLastOccurrence (':');
    if  (x >= 0)
      return  dirPath.SubStrPart (0, x - 1);
    else
      return  KKStr::EmptyStr ();
  }

  return  dirPath.SubStrPart (0, x - 1);
}



KKStr   KKU::osGetHostName ()
{
#if  defined(OS_WINDOWS)
  char  buff[1024];
  DWORD buffSize = sizeof (buff) - 1;
  memset (buff, 0, sizeof(buff));
 
  KKStr  compName = "";

  //BOOL returnCd = GetComputerNameEx (ComputerNameDnsFullyQualified, buff, &buffSize);
  BOOL returnCd = GetComputerNameA (buff, &buffSize);
  if  (returnCd != 0)
  {
    compName = buff;
  } 
  else 
  {  
    KKStrPtr  compNameStr = osGetEnvVariable ("COMPUTERNAME");
    if  (compNameStr)
    {
      compName = *compNameStr;
      delete compNameStr;
      compNameStr = NULL;
    }
    else
    {
      compName = "";
    }
  }  

  return  compName;  


#else

  char  buff[1024];
  memset (buff, 0, sizeof (buff));
  int32  returnCd = gethostname (buff, sizeof (buff) - 2);
  if  (returnCd != 0)
    return "";
  else
    return buff;

#endif
}  /* osGetHostName */



KKStr  KKU::osGetProgName ()
{
#if  defined(OS_WINDOWS)
  KKStr  progName;

  char filename[ MAX_PATH ];
  DWORD size = GetModuleFileNameA (NULL, filename, MAX_PATH);
  if  (size)
    progName = filename;  
  else
    progName = "";

  return progName;

#else
  return  "NotImplemented";  
#endif
}






KKStr  KKU::osGetUserName ()
{
#if  defined(OS_WINDOWS)
  TCHAR name [ UNLEN + 1 ];
  DWORD size = UNLEN + 1;

  KKStr  userName = "";
  if  (GetUserName ((TCHAR*)name, &size))
    userName = name;
  else
    userName = "***ERROR***";

  return  userName;
#else

  return "NoImplemented";
#endif
}  /* osGetUserName */








int32  KKU::osGetNumberOfProcessors ()
{
#if  defined(OS_WINDOWS)
  KKStrPtr numProcessorsStr = osGetEnvVariable ("NUMBER_OF_PROCESSORS");
  uint32  numOfProcessors = -1;
  if  (numProcessorsStr)
  {
    numOfProcessors = numProcessorsStr->ToInt32 ();
    delete  numProcessorsStr;
    numProcessorsStr = NULL;
  }

  return  numOfProcessors;
#else
  /** @todo  Need to implement 'osGetNumberOfProcessors' for linux. */
  return  1;
#endif
}  /* osGetNumberOfProcessors */




KKStr  KKU::osGetFileNamePartOfFile (KKStr  fullFileName)
{
  if  (fullFileName.LastChar () == DSchar)
    return  KKStr ("");

  int32  lastSlash =  osLocateLastSlashChar (fullFileName);
  if  (lastSlash < 0)
  {
   int32  colon = fullFileName.LocateCharacter (':');
   if  (colon < 0)
     return fullFileName;
   else
     return fullFileName.SubStrPart (colon + 1);
  }
  else
  {
    return  fullFileName.SubStrPart (lastSlash + 1);
  }
}  /* FileNamePartOfFile */





bool  backGroundProcess = false;



void  KKU::osRunAsABackGroundProcess ()
{
  backGroundProcess = true;
}



bool  KKU::osIsBackGroundProcess ()
{
  return  backGroundProcess;
}



void  KKU::osWaitForEnter ()
{
  if  (backGroundProcess)
    return;

  cout << std::endl
       << std::endl
       << "Press Enter To Continue"
       << std::endl;

  while  (getchar () != '\n');

} /* osWaitForEnter */




#ifdef  WIN32
KKStrListPtr  KKU::osGetListOfFiles (const KKStr&  fileSpec)
{
  WIN32_FIND_DATA     wfd;

  HANDLE  handle = FindFirstFile  (fileSpec.Str (),  &wfd);

  if  (handle == INVALID_HANDLE_VALUE)
  {
    return  NULL;
  }

  KKStrListPtr  nameList = new KKStrList (true, 50);

  BOOL  moreFiles = true;
  while  (moreFiles)
  {
    if  ((wfd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == 0)  
    {
      if  ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
      {
        nameList->PushOnBack (new KKStr (wfd.cFileName));
      }
    }

    moreFiles = FindNextFile (handle, &wfd);
  }

  FindClose (handle);

  return  nameList;
}  /* osGetListOfFiles */

#else




KKStrListPtr  osDirectoryList (KKStr  dirName)   /*  Unix Version of Function  */
{
  if  (dirName.Empty ())
  {
    dirName = osGetCurrentDirectory ();
  }
  else
  {
    osAddLastSlash (dirName);
  }

  KKStrListPtr  nameList = new KKStrList (true, 50);

  DIR*  openDir = opendir (dirName.Str ());
  if  (openDir == NULL)
    return NULL;

  struct dirent  *de;
  de = readdir (openDir);

  while  (de)
  {
    KKStr  rootName (de->d_name);
    if  ((rootName != ".")  &&  (rootName != ".."))
    {
      KKStr  fullName  (dirName);
      fullName << rootName;
      struct stat  fs;


      if  ((fs.st_mode & S_IFDIR) == 0)
      {
        nameList->PushOnBack (new KKStr (rootName));
      }
    }

    de = readdir (openDir);
  }

  closedir (openDir);

  return  nameList;
}  /* osDirectoryList */





KKStrListPtr  KKU::osGetListOfFiles (const KKStr&  fileSpec)
{
  KKStr  afterLastSlash;
  KKStr  afterStar;
  KKStr  beforeStar;
  KKStr  dirPath;

  int32 lastSlash = osLocateLastSlashChar (fileSpec);
    
  if  (lastSlash < 0)
  {
    dirPath = osGetCurrentDirectory ();
    afterLastSlash = fileSpec;
  }
  else
  {
    dirPath = fileSpec.SubStrPart (0, lastSlash);
    afterLastSlash = fileSpec.SubStrPart (lastSlash + 1);
  }

  osAddLastSlash (dirPath);

  KKStrListPtr  allFilesInDirecory = osDirectoryList (dirPath);

  if  (!allFilesInDirecory)
    return NULL;

  KKStrListPtr  searchSpecParms = osParseSearchSpec (afterLastSlash);

  KKStrListPtr  resultList = new KKStrList (true, allFilesInDirecory->QueueSize ());

  //StringListIterator  nameIDX (*allFilesInDirecory);
  KKStrList::iterator nameIDX;

  KKStrPtr  name;

  for  (nameIDX =allFilesInDirecory->begin ();  nameIDX != allFilesInDirecory->end ();  ++nameIDX)
  {
    name = *nameIDX;
    if  (osFileNameMatchesSearchFields (*name, searchSpecParms))
       resultList->PushOnBack (new KKStr (*name));
  }  


  delete  allFilesInDirecory;
  delete  searchSpecParms;
  
  return  resultList;
}  /* osGetListOfFiles */

#endif




void  KKU::osGetListOfFilesInDirectoryTree (const KKStr&  rootDir,
                                            KKStr         fileSpec,
                                            VectorKKStr&  fileNames   // The file names include full path.
                                           )
{ 
  if  (fileSpec.Empty ())
    fileSpec = "*.*";

  {
    KKStrListPtr  filesInThisDirectory = osGetListOfFiles (osAddSlash (rootDir) + fileSpec);
    if  (filesInThisDirectory)
    {
      KKStrList::iterator  idx;
      for  (idx = filesInThisDirectory->begin ();  idx != filesInThisDirectory->end ();  idx++)
      {
        KKStrPtr  fn = *idx;
        KKStr  fullName = osAddSlash (rootDir) + (*fn);
        fileNames.push_back (fullName);
      }
      delete  filesInThisDirectory;  filesInThisDirectory = NULL;
    }
  }

  // Lets now process all sub directories below 'rootDir'
  KKStrListPtr  subDirectories = osGetListOfDirectories (osAddSlash (rootDir) + "*.*");
  if  (subDirectories)
  {
    KKStrList::iterator  idx;
    for  (idx = subDirectories->begin ();  idx != subDirectories->end ();  idx++)
    {
      KKStr subDirName = **idx;
      if  ((subDirName == ".")  ||  (subDirName == ".."))
        continue;


      KKStr  dirToSearch = osAddSlash (rootDir) + subDirName;
      osGetListOfFilesInDirectoryTree (dirToSearch, fileSpec, fileNames);
    }

    delete  subDirectories;  subDirectories = NULL;
  }

  return;
}  /* osGetListOfFilesInDirectoryTree */





KKStrListPtr  KKU::osGetListOfImageFiles (KKStr  fileSpec)
{
  KKStrListPtr  imageFileNames = new KKStrList (true);

  KKStrListPtr filesInDir = osGetListOfFiles (fileSpec);
  if  (filesInDir)
  {
    KKStrList::iterator  fnIDX;
    for  (fnIDX = filesInDir->begin ();  fnIDX != filesInDir->end (); ++fnIDX)
    {
      KKStr  fileName (*fnIDX);
      if  (SupportedImageFileFormat (fileName))
        imageFileNames->PushOnBack (new KKStr (fileName));
    }

    delete  filesInDir;
    filesInDir = NULL;
  }

  return  imageFileNames;
}  /* osGetListOfImageFiles */




#ifdef  WIN32
KKStrListPtr  KKU::osGetListOfDirectories (KKStr  fileSpec)
{
  WIN32_FIND_DATA     wfd;

  
  if  (fileSpec.LastChar () == DSchar)
  {
    fileSpec << "*.*";
  }

  else if  (fileSpec.LocateCharacter ('*') < 0)
  {
    if  (osValidDirectory (&fileSpec))
    {
      fileSpec << "\\*.*";
    }
  }

  KKStrListPtr  nameList = new KKStrList (true, 50);

  HANDLE  handle = FindFirstFile  (fileSpec.Str (),  &wfd);

  if  (handle == INVALID_HANDLE_VALUE)
  {
    delete  nameList;
    return  NULL;
  }


  BOOL  moreFiles = true;
  while  (moreFiles)
  {
    if  ((wfd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == 0)  
    {
      if  ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
      {
        KKStrPtr dirName = new KKStr (wfd.cFileName);

        if  ((*dirName != ".")  &&  (*dirName != ".."))
          nameList->PushOnBack (dirName);
        else
          delete  dirName;
      }
    }

    moreFiles = FindNextFile (handle, &wfd);
  }

  FindClose (handle);

  return  nameList;
}  /* osGetListOfDirectories */




#else
KKStrListPtr  KKU::osGetListOfDirectories (KKStr  fileSpec)
{

  KKStr  rootDirName;
  int32  x = fileSpec.LocateCharacter ('*');
  if  (x > 0)
    rootDirName = fileSpec.SubStrPart (0, x - 1);
  else
    rootDirName = fileSpec;


  osAddLastSlash (rootDirName);

  KKStrListPtr  nameList = new KKStrList (true, 50);

  DIR*  openDir = opendir (rootDirName.Str ());
  if  (openDir == NULL)
    return NULL;

  struct dirent  *de;
  de = readdir (openDir);

  while  (de)
  {
    KKStr  rootName (de->d_name);
    if  ((rootName != ".")  &&  (rootName != ".."))
    {      
      KKStr  fullName  (rootDirName);
      fullName << rootName;
      struct stat  fs;


      if  ((fs.st_mode & S_IFDIR) != 0)
      {
        nameList->PushOnBack (new KKStr (rootName));
      }
    }

    de = readdir (openDir);
  }

  closedir (openDir);

  return  nameList;
}  /* osGetListOfDirectories */

#endif







#ifdef  WIN32
double  KKU::osGetSystemTimeUsed ()
{
  HANDLE h = GetCurrentProcess ();

  FILETIME   creationTime, exitTime, kernelTime, userTime;

  BOOL  ok = GetProcessTimes (h, &creationTime, &exitTime, &kernelTime, &userTime);

  if  (!ok)
    return 0;


  SYSTEMTIME  st;

  FileTimeToSystemTime(&kernelTime, &st);
  double  kt =  st.wHour * 3600 + st.wMinute * 60 + st.wSecond;
  kt += ((double)st.wMilliseconds / 1000.0);

  FileTimeToSystemTime(&userTime, &st);
  double  ut =  st.wHour * 3600 + st.wMinute * 60 + st.wSecond;
  ut += ((double)st.wMilliseconds / 1000.0);


  double  numOfSecs = kt + ut;
  

  // (kernelTime.dwLowDateTime + userTime.dwLowDateTime) / 10000000 + 0.5;

  return  numOfSecs;
}  /* osGetSystemTimeUsed */

#else
double  KKU::osGetSystemTimeUsed ()
{
  struct  tms  buff;
  times (&buff);
  double  totalTime = (double)(buff.tms_utime + buff.tms_stime);
  return  (totalTime / (double)(sysconf (_SC_CLK_TCK)));
}
#endif





#ifdef  WIN32
double  KKU::osGetUserTimeUsed ()
{
  HANDLE h = GetCurrentProcess ();

  FILETIME   creationTime, exitTime, kernelTime, userTime;

  BOOL  ok = GetProcessTimes (h, &creationTime, &exitTime, &kernelTime, &userTime);

  if  (!ok)
    return 0;


  SYSTEMTIME  st;

  FileTimeToSystemTime(&userTime, &st);
  double   ut =  st.wHour * 3600 + 
                 st.wMinute * 60 + 
                 st.wSecond      +
                 st.wMilliseconds / 1000.0;

  return  ut;
}  /* osGetSystemTimeUsed */

#else




double  KKU::osGetUserTimeUsed ()
{
  struct  tms  buff;
  times (&buff);
  double  totalTime = (double)(buff.tms_utime);
  return  (totalTime / (double)(sysconf (_SC_CLK_TCK)));
}
#endif




#ifdef  WIN32
double  KKU::osGetKernalTimeUsed ()
{
  HANDLE h = GetCurrentProcess ();

  FILETIME   creationTime, exitTime, kernelTime, userTime;

  BOOL  ok = GetProcessTimes (h, &creationTime, &exitTime, &kernelTime, &userTime);

  if  (!ok)
    return 0.0;

  SYSTEMTIME  st;

  FileTimeToSystemTime(&kernelTime, &st);
  double  kt =  st.wHour * 3600 + 
                st.wMinute * 60 + 
                st.wSecond      +
                st.wMilliseconds / 1000.0;


  return  kt;
}  /* osGetSystemTimeUsed */

#else
double  KKU::osGetKernalTimeUsed ()
{
  struct  tms  buff;
  times (&buff);
  double  totalTime = (double)(buff.tms_stime);
  return  (totalTime / (double)(sysconf (_SC_CLK_TCK)));
}  /* osGetSystemTimeUsed */
#endif





#ifdef  WIN32
DateTime  KKU::osGetLocalDateTime ()
{
  SYSTEMTIME  sysTime;

  GetLocalTime(&sysTime);

  DateTime  dateTime ((short)sysTime.wYear,
                      (uchar)sysTime.wMonth,
                      (uchar)sysTime.wDay,
                      (uchar)sysTime.wHour,
                      (uchar)sysTime.wMinute,
                      (uchar)sysTime.wSecond
                     );

  return  dateTime;
}  /* osGetCurrentDateTime */




#else
DateTime  KKU::osGetLocalDateTime ()
{
  struct tm  *curTime;
  time_t     long_time;

  time (&long_time);                /* Get time as long integer. */
  curTime = localtime (&long_time); /* Convert to local time. */

  DateTime  dateTime (curTime->tm_year + 1900,
                      curTime->tm_mon + 1,
                      curTime->tm_mday,
                      curTime->tm_hour,
                      curTime->tm_min,
                      curTime->tm_sec
                     );

  return  dateTime;
}  /* osGetCurrentDateTime */
#endif



#ifdef  WIN32
DateTime  KKU::osGetFileDateTime (const KKStr& fileName)
{
  WIN32_FIND_DATA   wfd;

  HANDLE  handle = FindFirstFile  (fileName.Str (), &wfd);
  if  (handle == INVALID_HANDLE_VALUE)
  {
    return  DateTime (0, 0, 0, 0 ,0 ,0);
  }


  SYSTEMTIME  fileTime;
  SYSTEMTIME  stLocal;


  FileTimeToSystemTime (&(wfd.ftLastWriteTime), &fileTime);
  SystemTimeToTzSpecificLocalTime(NULL, &fileTime, &stLocal);

  return  DateTime ((short)stLocal.wYear, 
                    (uchar)stLocal.wMonth, 
                    (uchar)stLocal.wDay,
                    (uchar)stLocal.wHour,
                    (uchar)stLocal.wMinute,
                    (uchar)stLocal.wSecond
                   );

}  /* osGetFileDateTime */



#else

DateTime  KKU::osGetFileDateTime (const KKStr& fileName)
{
  struct  stat  buf;

  int32  returnCd = stat (fileName.Str (), &buf);

  if  (returnCd != 0)
  {
    return  DateTime (0, 0, 0, 0, 0, 0);
  }


  struct tm* dt =  localtime (&(buf.st_mtime));

  return  DateTime (1900 + dt->tm_year, 
                    dt->tm_mon + 1, 
                    dt->tm_mday,
                    dt->tm_hour,
                    dt->tm_min,
                    dt->tm_sec
	           );
}
#endif



#ifdef  WIN32
int64   KKU::osGetFileSize (const KKStr&  fileName)
{
  WIN32_FIND_DATA   wfd;

  HANDLE  handle = FindFirstFile  (fileName.Str (), &wfd);
  if  (handle == INVALID_HANDLE_VALUE)
  {
    return  -1;
  }

  return  (wfd.nFileSizeHigh * MAXDWORD) + wfd.nFileSizeLow;
}


#else

int64   KKU::osGetFileSize (const KKStr&  fileName)
{
  struct  stat  buf;

  int32  returnCd = stat (fileName.Str (), &buf);

  if  (returnCd != 0)
  {
    return  -1;
  }

  return  buf.st_size;
}
#endif




#if  defined(WIN32)
void  KKU::osDisplayWarning (KKStr  _message)
{
  if  (backGroundProcess)
    return;

  MessageBox (NULL,
              _message.Str (),
              "Warning",
              (MB_OK + MB_SETFOREGROUND)
             );
/*
  cerr << endl
       << "    *** WARNING ***" << endl
       << endl 
       << _message << endl
       << endl;
  osWaitForEnter ();
*/
}


#else
void  KKU::osDisplayWarning (KKStr  _message)
{
  cerr << std::endl
       << "    *** WARNING ***" << std::endl
       << std::endl 
       << _message << std::endl
       << std::endl;

  if  (!backGroundProcess)
    osWaitForEnter ();
}
#endif




//*******************************************************************
//*   fileName  - Name of file we are looking for.                  *
//*   srcDir    - Sub Directory tree we want to search.             *
//*                                                                 *
//*   Returns   - Full directory path to where first occurrence of   *
//*               fileName is located.  If not found will return    *
//*               back an empty string.                             *
//*******************************************************************
KKStr   KKU::osLookForFile (const KKStr&  fileName,
                            const KKStr&  srcDir
                           )
{
  KKStr  fileNameUpper (fileName);
  fileNameUpper.Upper ();

  KKStr  dirFoundIn = "";

  KKStr  fileSpec = osAddSlash (srcDir) + fileName;

  // We will first look at contents of 'srcDir'  and if not there then look at sub directories
  bool  found = false;
  KKStrListPtr files = osGetListOfFiles (fileSpec);
  if  (files)
  {
    for  (KKStrList::iterator nIDX = files->begin ();  nIDX != files->end ();  nIDX++)
    {
      KKStrPtr  fnPtr = *nIDX;
      if  (KKStr::StrEqualNoCase (fileName.Str (), fnPtr->Str ()))
      {
        found = true;
        dirFoundIn = srcDir;
      }
    }
    delete  files;
    files = NULL;
  }

  if  (!found)
  {
    KKStrListPtr  subDirs = osGetListOfDirectories (srcDir);
    if  (subDirs)
    {
      for  (KKStrList::iterator sdIDX = subDirs->begin ();  (sdIDX != subDirs->end ())  &&  (!found);  ++sdIDX)
      {
        KKStr  subDirName = osAddSlash (srcDir) + **sdIDX;
        KKStr  resultDir = osLookForFile (fileName, subDirName);
        if  (!resultDir.Empty ())
        {
          found = true;
          dirFoundIn = resultDir;
        }
      }
      delete  subDirs;  subDirs = NULL;
    }
  }

  return  dirFoundIn;
}  /* osLookForFile */




KKStr  KKU::osCreateUniqueFileName (KKStr  fileName)
{
  if  (fileName.Empty ())
    fileName = "Temp.txt";

  KKStr   dirPath, rootName, extension;

  osParseFileName (fileName, dirPath, rootName, extension);

  int32  seqNum = 0;
  bool  fileNameExists = osFileExists (fileName);
  while  (fileNameExists)
  {
    if  (dirPath.Empty ())
    {
      fileName = rootName + "_" + StrFormatInt (seqNum, "ZZ00") + "." + extension;
    }
    else
    {
      fileName = osAddSlash (dirPath) + 
                 rootName + "_" + StrFormatInt (seqNum, "ZZ00") + 
                 "." + extension;
    }

    fileNameExists = osFileExists (fileName);
    seqNum++;
  }

  return  fileName;
}  /* osCreateUniqueFileName */








char*  KKU::osReadNextLine (FILE*  in)
{
  /** @todo  This function has never be properly tested.  Next time we use it we should debug through it to make sure that all is copacetic. */
  char  buff[1024];
  buff[0] = 0;
  int32  buffLen = sizeof (buff);

  if  (fgets (buff, buffLen, in) == NULL)
    return NULL;

  char*  line = NULL;
#ifdef WIN32
  line = _strdup (buff);
#else
  line = strdup (buff);
#endif

  int32  lineLen = (int32)strlen (line);
  while  (line[lineLen - 1] != '\n')
  {
    if  (fgets (buff, buffLen, in) == NULL)
      return  line;

    int32  additionalCharsRead = (int32)strlen (buff);
    int32  newLineLen = lineLen + additionalCharsRead;
    line = osGrowAllocation (line, lineLen, newLineLen);
    memcpy (line + lineLen, buff, additionalCharsRead);
    lineLen = newLineLen;
  }
  
  return  line;  
}  /* osReadNextLine */






KKStr  KKU::osReadNextToken (istream&    in, 
                             const char* delimiters,
                             bool&       eof,
                             bool&       eol
                            )
{
  eof = false;
  eol = false;

  char  token[1024];
  int32  maxTokenLen = sizeof (token) - 1;

  //int32  ch = fgetc (in);  eof = (feof (in) != 0);
  int32  ch = in.get ();  
  eof = in.eof ();
  if  (eof)
  {
    eol = true;
    return "";
  }

  // lets skip leading white space
  while  ((!eof)  &&  ((ch == ' ') || (ch == '\r'))  &&  (ch != '\n'))
  {
    ch = in.get (); 
    eof = in.eof ();
  }

  if  (ch == '\n')
  {
    eol = true;
    return "";
  }

  int32 tokenLen = 0;

  // Read till first delimiter or eof
  while  ((!eof)  &&  (!strchr (delimiters, ch)))
  {
    if  (ch == '\n')
    {
      in.putback (ch);
      break;
    }
    else
    {
      token[tokenLen] = ch;
      tokenLen++;
      
      if  (tokenLen >= maxTokenLen)
        break;

      ch = in.get (); 
      eof = in.eof ();
    }
  }

  token[tokenLen] = 0;  // Terminating NULL character.

  // Remove Trailing whitespace
  while  (tokenLen > 0)
  {
    if  (strchr (" \r", token[tokenLen - 1]) == 0)
      break;
    tokenLen--;
    token[tokenLen] = 0;
  }

  return  token;
}  /* osReadNextToken */



KKStr  KKU::osReadNextToken (FILE*       in, 
                             const char* delimiters,
                             bool&       eof,
                             bool&       eol
                            )
{
  eof = false;
  eol = false;

  char  token[1024];
  int32  maxTokenLen = sizeof (token) - 1;

  int32  ch = fgetc (in);  eof = (feof (in) != 0);

  if  (eof)
  {
    eol = true;
    return "";
  }

  // lets skip leading white space
  while  ((!eof)  &&  ((ch == ' ') || (ch == '\r'))  &&  (ch != '\n'))
    {ch = fgetc (in); eof = (feof (in)!= 0);}

  if  (ch == '\n')
  {
    eol = true;
    return "";
  }

  int32 tokenLen = 0;

  // Read till first delimiter or eof
  while  ((!eof)  &&  (!strchr (delimiters, ch)))
  {
    if  (ch == '\n')
    {
      ungetc (ch, in);
      break;
    }
    else
    {
      token[tokenLen] = ch;
      tokenLen++;
      
      if  (tokenLen >= maxTokenLen)
        break;

      ch = fgetc (in); eof = (feof (in)!= 0);
    }
  }

  token[tokenLen] = 0;  // Terminating NULL character.


  // Remove Trailing whitespace
  while  (tokenLen > 0)
  {
    if  (strchr (" \r", token[tokenLen - 1]) == 0)
      break;
    tokenLen--;
    token[tokenLen] = 0;
  }

  return  token;
}  /* ReadNextToken */





KKStr   KKU::osReadRestOfLine (std::istream&    in,
                               bool&            eof
                              )
{
  eof = false;

  char  line[20480];
  int32  maxLineLen = sizeof (line) - 1;

  int32  ch = in.get ();  
  eof = in.eof ();

  if  (eof)
    return "";

  int32 lineLen = 0;

  // Read till first delimiter or eof
  while  (!eof)
  {
    if  (ch == '\n')
    {
      break;
    }
    else
    {
      line[lineLen] = ch;
      lineLen++;
      
      if  (lineLen >= maxLineLen)
        break;

      ch = in.get ();  eof = in.eof ();
    }
  }

  line[lineLen] = 0;  // Terminating NULL character.


  // Remove Trailing whitespace
  while  (lineLen > 0)
  {
    if  (strchr (" \r\n", line[lineLen - 1]) == 0)
      break;
    lineLen--;
    line[lineLen] = 0;
  }

  return  line;
}  /* osReadRestOfLine */




KKStr  KKU::osReadRestOfLine (FILE*  in,
                              bool&  eof
                             )
{
  eof = false;

  char  line[20480];
  int32  maxLineLen = sizeof (line) - 1;

  int32  ch = fgetc (in);  
  eof = (feof (in) != 0);
  if  (eof)  return "";

  int32 lineLen = 0;

  // Read till first delimiter or eof
  while  (!eof)
  {
    if  (ch == '\n')
    {
      break;
    }
    else
    {
      line[lineLen] = ch;
      lineLen++;
      if  (lineLen >= maxLineLen)
        break;
      ch = fgetc (in);  eof = (feof (in) != 0);
    }
  }
  line[lineLen] = 0;  // Terminating NULL character.

  // Remove Trailing whitespace
  while  (lineLen > 0)
  {
    if  (strchr (" \r\n", line[lineLen - 1]) == 0)
      break;
    lineLen--;
    line[lineLen] = 0;
  }

  return  line;
}  /* osReadRestOfLine */







KKStr  KKU::osReadNextQuotedStr (FILE*        in,
                                 const char*  whiteSpaceCharacters,
                                 bool&        eof
                                )
{
  if  (feof (in))
  {
    eof = true;
    return KKStr::EmptyStr ();
  }

  // Skip leading white space and find first character in Token
  int32  ch = fgetc (in);

  while  ((!feof (in))  &&  (strchr (whiteSpaceCharacters, ch) != NULL))
  {
    ch = fgetc (in); 
  }

  if  (feof (in))
  {
    eof = true;
    return  KKStr::EmptyStr ();
  }


  KKStr  result (10);

  bool  lookForTerminatingQuote = false;

  if  (ch == '"')
  {
    // We are going to read in a quoted string.  In this case we include all characters until 
    // we find the terminating quote
    lookForTerminatingQuote = true;
    ch = fgetc (in);
  }

  // Search for matching terminating Quote

  while  (!feof (in))
  {
    if  (lookForTerminatingQuote)
    {
      if  (ch == '"')
      {
        break;
      }
    }

    else 
    {
      if  (strchr (whiteSpaceCharacters, ch))
      {
        // We found the next terminating white space character.
        break;
      }
    }

    if  ((ch == '\\')  &&  (lookForTerminatingQuote))
    {
      if  (!feof (in))
      {
        ch = fgetc (in);
        switch  (ch)
        {
         case  '"': result.Append ('"');      break;
         case  't': result.Append ('\t');     break;
         case  'n': result.Append ('\n');     break;
         case  'r': result.Append ('\r');     break;
         case '\\': result.Append ('\\');     break;
         case  '0': result.Append (char (0)); break;
         case    0:                           break;
         default:   result.Append (ch); break;
        }
      }
    }
    else
    {
      result.Append (ch);
    }

    ch = fgetc (in);
  }

  // Eliminate all trailing white space
  if  (!feof (in))
  {
    ch = fgetc (in);
    while  ((!feof (in))  &&  (strchr (whiteSpaceCharacters, ch) != NULL))
    {
      ch = fgetc (in);
    }

    if  (!feof (in))
    {
      ungetc (ch, in);
    }
  }


  return  result;
}  /* osReadNextQuotedStr */




void  KKU::osSkipRestOfLine (FILE*  in,
                             bool&  eof
                            )
{
  eof = false;
  int32  ch = fgetc (in);  eof = (feof (in) != 0);
  while  ((ch != '\n')  &&  (!eof))
  {
    ch = fgetc (in);  eof = (feof (in) != 0);
  }
}  /* osSkipRestOfLine */




void  KKU::osSkipRestOfLine (istream&  in,
                             bool&     eof
                            )
{
  int32  ch = in.get ();  
  eof = in.eof ();

  while  ((ch != '\n')  &&  (!eof))
  {
    ch = in.get ();  
    eof = in.eof ();
  }
}  /* osSkipRestOfLine */



int32  KKU::osGetProcessId ()
{
#ifdef  WIN32
  DWORD  processId = GetCurrentProcessId ();
  return  processId;

#else
  pid_t processID = getpid ();
  return  processID;

#endif
}



int32  KKU::osGetThreadId ()
{
#ifdef  WIN32
  DWORD threadId = GetCurrentThreadId ();
  return  threadId;
#else
  return 0;
#endif
}


void  KKU::osSleep (float secsToSleep)
{
  #ifdef  WIN32
  int32  miliSecsToSleep = (int32)(1000.0f * secsToSleep + 0.5f);
  Sleep (miliSecsToSleep);
  #else
  int32  secsToSleepInt = (int32)(0.5f + secsToSleep);

  if  (secsToSleepInt < 1)
    secsToSleepInt = 1;
 
  else if  (secsToSleepInt > 3600)
    cout  << "osSleep  secsToSleep[" << secsToSleepInt << "]" << std::endl;

  sleep (secsToSleepInt);
  #endif
}




int32  LocateEnvStrStart (const KKStr&  str)
{
  int32  x = 0;
  int32  y = 1;
  int32  len = str.Len ();
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



KKStr  KKU::osSubstituteInEvironmentVariables (const KKStr&  src)
{
  int32  x = LocateEnvStrStart (src);
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
      return  src;

    str = beforeEnvStr + (*envStrValue)  + afterStrName;
    delete  envStrValue;
    envStrValue = NULL;
    x = LocateEnvStrStart (str);
  }

  return  str;
}  /* osSubstituteInEvironmentVariables */



VectorKKStr  KKU::osSplitDirectoryPathIntoParts (const KKStr&  path)
{
  VectorKKStr  parts;

  if  (path.Len () == 0)
    return parts;

  int32  zed = 0;

  if  (path[(uint16)1] == ':')
  {
    parts.push_back (path.SubStrPart (0, 1));
    zed += 2;
  }

  while  (zed < path.Len ())
  {
    if  (path[zed] == '\\')
    {
      parts.push_back ("\\");
      zed++;
    }
    else if  (path[zed] == '/')
    {
      parts.push_back ("/");
      zed++;
    }
    else
    {
      // Scan until we come up to another separator or end of string
      int32  startPos = zed;
      while  (zed < path.Len ())
      {
        if  ((path[zed] == '\\')  ||  (path[zed] == '/'))
          break;
        zed++;
      }

      parts.push_back (path.SubStrPart (startPos, zed - 1));
    }
  }

  return  parts;
}  /* osSplitDirectoryPathIntoParts */



KKStr  KKU::osGetFullPathOfApplication ()
{
#if  defined(WIN32)
  char  szAppPath[MAX_PATH] = "";
  DWORD  result = ::GetModuleFileName (0, szAppPath, MAX_PATH);
  return  szAppPath;
#else
  return  KKStr::EmptyStr ();
#endif
}
