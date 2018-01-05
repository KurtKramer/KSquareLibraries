/* OSservices.cpp -- O/S related functions,  meant to be O/S neutral
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */

// For non windows systems; will allow fseeko() and ftello() to work with 64 bit int's.
#define _FILE_OFFSET_BITS 64

#include "FirstIncludes.h"
#include <cstdio>


#if  defined(KKOS_WINDOWS)
#include <windows.h>
#include <Lmcons.h>
#include <conio.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/time.h>
#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <iostream>
#include <fstream> 
//#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <string>
#include <vector>
#include "MemoryDebug.h"
using namespace std;

#include "KKException.h"
#include "OSservices.h"
#include "ImageIO.h"
#include "KKStr.h"
using namespace KKB;



KKStr  KKB::osGetErrorNoDesc (kkint32  errorNo)
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



std::FILE*  KKB::osFOPEN (const char* fileName,
                          const char* mode
					               )
{
  std::FILE*  f = NULL;

# ifdef  USE_SECURE_FUNCS
  fopen_s (&f, fileName, mode);
# else
    f = std::fopen (fileName, mode);
# endif

  return  f;
}


kkint64  KKB::osFTELL (std::FILE* f)
{
#if  defined(KKOS_WINDOWS)
  return  _ftelli64 (f);
#else
  return  ftello( f);
#endif
}



int  KKB::osFSEEK (std::FILE*  f,
                   kkint64     offset,
                   int         origin
                  )
{
#if  defined(KKOS_WINDOWS)
  return  _fseeki64(f, offset, origin);
#else
  return  fseeko(f, offset, origin);
#endif
}





//***************************** ParseSearchSpec ********************************
//*  Will parse the string searchSpec into separate fields using '*' as the    *
//*  delimiter character.                                                      *
//*                                                                            *
//*  ex: input:   searchSpec = "Alpha*Right*.txt"                              *
//*      returns: ("Alpha", "*", "Right", "*", ".txt")                         *
//*                                                                            *
//*  The caller is responsible for deleting the StrigList that is returned     *
//*  when no longer needed.                                                    *
//*                                                                            *
//******************************************************************************
KKStrListPtr  osParseSearchSpec (const KKStr&  searchSpec)
{
  KKStrListPtr  searchFields = new KKStrList (true);

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
  workStr = NULL;

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

  KKStrConstPtr  fieldPtr = searchFields->IdxToPtr (0);

  if  (searchFields->QueueSize () == 1)
  {
    if  (*fieldPtr == "*")
      return  true;

    if  (*fieldPtr == fileName)
      return true;
    else
      return false;
  }
 
  bool  lastFieldAStar = false;

  const char*  cp = fileName.Str ();
  kkuint32     lenLeftToCheck = fileName.Len ();

  kkint32  fieldNum;

  for  (fieldNum = 0;  fieldNum < searchFields->QueueSize ();  fieldNum++)
  {
    const KKStr&  field = *(searchFields->IdxToPtr (fieldNum));
    
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
            // lets move cp beyond where we found field in fileName
            cp = cp + field.Len ();
            if  (field.Len() > lenLeftToCheck)
              lenLeftToCheck = 0;
            else
              lenLeftToCheck = lenLeftToCheck - field.Len ();
          }
          else
          {
            ++cp;
            --lenLeftToCheck;
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





char  KKB::osGetDriveLetter (const KKStr&  pathName)
{
#ifndef  WIN32
  cerr << "KKB::osGetDriveLetter  no such think as a drive letter in this environment;  "
       << "pathName[" << pathName << "]."
       << endl;
  return 0;
#else
  if  (pathName.Len () < 3)
    return 0;

  if  (pathName[(kkint16)1] == ':')
    return  pathName[(kkint16)0];

  return 0;
#endif
}  /* osGetDriveLetter */







#ifdef WIN32
KKStr  KKB::osGetCurrentDirectory ()
{
  DWORD    buffLen = 0;
  char*    buff = NULL;
  DWORD    reqLen = 1000;

  while  (reqLen > buffLen)
  {
    buffLen = reqLen;

    if  (buff)
      delete[] buff;

    buff = new char[buffLen];
 
    reqLen = GetCurrentDirectory (buffLen, buff);
  }

  KKStr  curDir (buff);

  delete[] buff;   

  return  curDir;
}  /* GetCurrentDirectory */

#else
 




KKStr  KKB::osGetCurrentDirectory ()
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
bool  KKB::osValidDirectory (KKStrConstPtr  name)
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


bool  KKB::osValidDirectory (const KKStr&  name)
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

bool  KKB::osValidDirectory (KKStrConstPtr  name)
{
  auto*  openDir = opendir (name->Str ());

  if  (!openDir)
    return  false;

  closedir (openDir);
  return  true;
}



bool  KKB::osValidDirectory (const KKStr&  name)
{
  DIR*  openDir = opendir (name.Str ());

  if  (!openDir)
    return  false;

  closedir (openDir);
  return  true;
}

#endif



bool   KKB::osValidFileName (const KKStr&  _name)
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



KKStrListPtr  KKB::osValidFileNameErrors (const KKStr&  _name)
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
    kkuint32  x = 0;
    while  (invalidNames[x] != NULL)
    {
      if  (_name.EqualIgnoreCase (invalidNames[x]))
        errors->PushOnBack (new KKStr ("Can not use \"" + _name + "\""));
      ++x;
    }

    // Check for invalid characters
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






bool  KKB::osDeleteFile (KKStr  _fileName)
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
  kkint32  returnCd;

  // We are in Unix Environment.
  returnCd = unlink (_fileName.Str ());
  return  (returnCd == 0);
  #endif  
}  /* DeleteFile */








#ifdef WIN32

bool  KKB::osFileExists (const KKStr&  _fileName)
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


bool  KKB::osFileExists (const KKStr&  _fileName)
{
  struct  stat  buff;
  kkint32       result;

  result  = stat (_fileName.Str (), &buff);
  if  (result == 0)
     return  true;
  else
     return  false;
}

#endif



bool  KKB::osMoveFileBetweenDirectories (const KKStr&  _fileName,
                                         const KKStr&  _srcDir,
                                         const KKStr&  _destDir
                                        )
{
  #ifdef  WIN32

    KKStr  srcName = osAddSlash (_srcDir) + _fileName;
    KKStr  destName = osAddSlash (_destDir) + _fileName;
    return  (MoveFile (srcName.Str (), destName.Str ()) != 0);

  #else
    KKStr errMsg = "KKB::osMoveFileBetweenDirectories   ***ERROR***  not implemented   _fileName[" +
                   _fileName + "]  _srcDir[" + _srcDir + "]  " + "_destDir[" + _destDir  + "].";
    cerr << endl << errMsg << endl << endl;
    return false;
  #endif
}



#ifdef  WIN32
bool  KKB::osCopyFileBetweenDirectories (const KKStr&  _fileName,
                                         const KKStr&  _srcDir,
                                         const KKStr&  _destDir
                                        )
{
  KKStr  existingFile (_srcDir);
  KKStr  destinationName (_destDir);
  BOOL   fail = 1;
  
  osAddLastSlash (existingFile);
  existingFile <<  _fileName;

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

bool  KKB::osCopyFileBetweenDirectories (const KKStr&  _fileName,
                                         const KKStr&  _srcDir,
                                         const KKStr&  _destDir
                                        )
{
  KKStr errMsg = "KB::osCopyFileBetweenDirectories   ***ERROR***    not implemented  _fileName[" +
                 _fileName + "]  _srcDir[" + _srcDir + "]  _destDir[" + _destDir + "].";
  cerr << endl << errMsg << endl << endl;
  return false;
}

#endif




#ifdef  WIN32
bool  KKB::osCopyFile (const KKStr&  srcFileName,
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


bool  KKB::osCopyFile (const KKStr&  srcFileName,
                       const KKStr&  destFileName
                      )
{
  KKStr errMsg = "KKB::osCopyFile    ***ERROR***    'osCopyFile'  not implemented  srcFileName[" +
                 srcFileName + "]   destFileName[" + destFileName + "].";
  cerr << endl << errMsg << endl << endl;
  return false;
}

#endif



#ifdef  WIN32

bool  KKB::osRenameFile (const KKStr&  oldName,
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


bool  KKB::osRenameFile (const KKStr&  oldName,
                         const KKStr&  newName
                        )
{
  
  kkint32  returnCd = std::rename (oldName.Str (), newName.Str ());

  if  (returnCd == 0)
    return true;

  kkint32  errorCode = errno;

  cerr << std::endl
       << "osRenameFile   *** ERROR ***,   Rename Failed"                          << std::endl
       << "               oldName[" << oldName << "]   NewName[" << newName << "]" << std::endl
       << "               errno[" << errorCode << "]"                              << std::endl
       << std::endl;

  return  false;
}  /* osRenameFile */
#endif




void  KKB::osChangeDir (const KKStr&  dirName,
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
  kkint32 errorCd = chdir (dirName.Str ());
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



bool  KKB::osCreateDirectoryPath (KKStr  _pathName)
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

    if  (!KKB::osValidDirectory (pathNameSoFar))
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





bool  KKB::osCreateDirectory (const KKStr&  _dirName)
{
  #ifdef  WIN32
    return  (CreateDirectory (_dirName.Str (), NULL) != 0);
  
  #else  
      
    kkint32  result;

    mode_t mode  = S_IRWXU + S_IRWXG;

    result = mkdir (_dirName.Str (), mode);
    return result == 0;

  #endif 
}




#ifdef  WIN32
 
KKStrPtr  KKB::osGetEnvVariable (const KKStr&  _varName)
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


KKStrPtr  KKB::osGetEnvVariable (const KKStr&  _varName)
{
  char*  envStrValue = NULL;
  
  envStrValue = getenv (_varName.Str ());

  if  (envStrValue)
    return  new KKStr (envStrValue);
  else
    return NULL;

} /* GetEnvVariable */
#endif



/**
 * Searches a string starting from a specified index for the start of an Environment String Specification.
 * @code
 *                         1         2         3         4         5         6
 *  ex:          0123456789012345678901234567890123456789012345678901234567890123456789 
 *        str = "TrainingDirectory\${LarcosHomeDir}\Classifiers\${CurTrainLibrary"
 * 
 *   idx = osLocateEnvStrStart (str, 0);  // returns 18
 *   idx = osLocateEnvStrStart (str, 34)  // returns 47
 * @endcode
 *
 * @param str  Starting that is to be searched.
 * @param startIdx  Index search is to start at.
 * @returns  Index of 1st character of a Environment String specifier or -1 if none was found.
 */
int  osLocateEnvStrStart (const KKStr&  str,
                          kkint32       startIdx  /**<  Index in 'str' to start search from. */
                         )
{
  int  x = startIdx;
  int  y = startIdx + 1;
  int  len = str.Len ();
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

    ++x;
    ++y;
  }

  return  -1;
}  /* LocateEnvStrStart */



KKStr  KKB::osSubstituteInEnvironmentVariables (const KKStr&  src)
{
  kkint64  x = osLocateEnvStrStart (src, 0);
  if  (x < 0)  return  src;

  KKStr  str (src);

  while  (x >= 0)
  {
    char  startChar = src[(kkint16)(x + 1)];
    char  endChar = ')';

    if       (startChar == '(')   endChar = ')';
    else if  (startChar == '{')   endChar = '}';
    else if  (startChar == '[')   endChar = ']';

    KKStr  beforeEnvStr = str.SubStrPart (0, x - 1);
    str = str.SubStrPart (x + 2);
    x = str.LocateCharacter (endChar);
    if  (x < 0)  return  src;

    KKStr  envStrName   = str.SubStrPart (0, x - 1);
    KKStr  afterStrName = str.SubStrPart (x + 1);

    KKStrPtr envStrValue = osGetEnvVariable (envStrName);
    if  (envStrValue == NULL)
      envStrValue = new KKStr ("${" + envStrName + "}");

    kkuint32  idxToStartAtNextTime = beforeEnvStr.Len () + envStrValue->Len ();
    str = beforeEnvStr + (*envStrValue)  + afterStrName;
    delete  envStrValue;
    x = osLocateEnvStrStart (str, idxToStartAtNextTime);
  }

  return  str;
}  /* osSubstituteInEnvironmentVariables */




kkint64  KKB::osLocateLastSlashChar (const KKStr&  fileSpec)
{
  kkint64  lastLeftSlash  = fileSpec.LocateLastOccurrence ('\\');
  kkint64  lastRightSlash = fileSpec.LocateLastOccurrence ('/');

  return  Max (lastLeftSlash, lastRightSlash);
}  /* LastSlashChar */



kkint64  KKB::osLocateFirstSlashChar (const KKStr&  fileSpec)
{
  kkint64  firstForewardSlash  = fileSpec.LocateCharacter ('/');
  kkint64  firstBackSlash = fileSpec.LocateCharacter ('\\');

  if  (firstForewardSlash < 0)
    return firstBackSlash;

  else if  (firstBackSlash < 0)
    return firstForewardSlash;

  return  Min (firstForewardSlash, firstBackSlash);
}  /* LastSlashChar */





void  KKB::osAddLastSlash (KKStr&  fileSpec)
{
  char  c = fileSpec.LastChar ();

  if  ((c != '\\')  &&  (c != '/'))
    fileSpec << DS;
}  /* osAddLastSlash */



KKStr  KKB::osAddSlash (const KKStr&  fileSpec)
{
  KKStr  result (fileSpec);
  if  ((result.LastChar () != '\\')  &&  (result.LastChar () != '/'))
     result << DS;
  return  result;
}  /* OsAddLastSlash */




KKStr  KKB::osMakeFullFileName (const KKStr&  _dirName, 
                                const KKStr&  _fileName
                               )
{
  KKStr  fullFileName (_dirName);

  osAddLastSlash (fullFileName);

  fullFileName << _fileName;

  return  fullFileName;
}



KKStr  KKB::osGetDirNameFromPath (KKStr  dirPath)
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



void   KKB::osParseFileName (KKStr   _fileName, 
                             KKStr&  _dirPath,
                             KKStr&  _rootName, 
                             KKStr&  _extension
                            )
{
  kkint64  x = osLocateLastSlashChar (_fileName);
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



KKStr  KKB::osRemoveExtension (const KKStr&  _fullFileName)
{
  kkint64  lastSlashChar = osLocateLastSlashChar (_fullFileName);
    
  kkint64  lastPeriodChar = _fullFileName.LocateLastOccurrence ('.');

  if  (lastPeriodChar < lastSlashChar)
    return KKStr (_fullFileName);

  if  (lastPeriodChar <= 1)
    return KKStr (_fullFileName);

  return _fullFileName.SubStrPart (0L, lastPeriodChar - 1);
}  /* osRemoveExtension */




KKStr  KKB::osGetRootName (const KKStr&  fullFileName)
{
  kkint64  lastSlashChar = osLocateLastSlashChar (fullFileName);
  kkint64  lastColon     = fullFileName.LocateLastOccurrence (':');

  kkint64  lastSlashOrColon = Max (lastSlashChar, lastColon);
 
  KKStr  lastPart;
  if  (lastSlashOrColon < 0)
  {
    lastPart = fullFileName;
  }
  else
  {
    lastPart = fullFileName.SubStrPart (lastSlashOrColon + 1);
  }

  kkint64  period = lastPart.LocateLastOccurrence ('.');

  if  (period < 0)
    return  lastPart;

  return  lastPart.SubStrPart (0, period - 1);
}  /*  osGetRootName */



KKStr  KKB::osGetRootNameOfDirectory (KKStr  fullDirName)
{
  if  (fullDirName.LastChar () == DSchar)
    fullDirName.ChopLastChar ();

  kkint64  lastSlashChar = osLocateLastSlashChar (fullDirName);
  kkint64  lastColon     = fullDirName.LocateLastOccurrence (':');

  kkint64  lastSlashOrColon = Max (lastSlashChar, lastColon);
 
  KKStr  lastPart;
  if  (lastSlashOrColon < 0)
    lastPart = fullDirName;
  else
    lastPart = fullDirName.SubStrPart (lastSlashOrColon + 1);

  return  lastPart;
}  /*  osGetRootNameOfDirectory */



KKStr  KKB::osGetParentDirectoryOfDirPath (KKStr  path)
{
  if  (path.LastChar () == DSchar) 
    path.ChopLastChar ();

  kkint64  x1 = path.LocateLastOccurrence (DSchar);
  kkint64  x2 = path.LocateLastOccurrence (':');

  kkint64  x = Max (x1, x2);
  if  (x < 1)
    return KKStr::EmptyStr ();

  return  path.SubStrPart (0L, x);
}  /* osGetParentDirectoryOfDirPath */



KKStr  KKB::osGetRootNameWithExtension (const KKStr&  fullFileName)
{
  kkint64  lastSlashChar = osLocateLastSlashChar (fullFileName);
  kkint64  lastColon     = fullFileName.LocateLastOccurrence (':');

  kkint64  lastSlashOrColon = Max (lastSlashChar, lastColon);
 
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





void  KKB::osParseFileSpec (KKStr   fullFileName,
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
  kkint64  driveLetterPos = fullFileName.LocateCharacter (':');
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

  kkint64  lastSlash =  osLocateLastSlashChar (fullFileName);
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

  kkint64  period = fileName.LocateLastOccurrence ('.');
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





KKStr  KKB::osGetPathPartOfFile (KKStr  fullFileName)
{
  kkint64  lastSlash =  osLocateLastSlashChar (fullFileName);

  if  (lastSlash >= 0)
  {
    return  fullFileName.SubStrPart (0, lastSlash - 1);
  }

  kkint64  lastColon = fullFileName.LocateLastOccurrence (':');
  if  (lastColon >= 0)
    return  fullFileName.SubStrPart (0, lastColon);
  else
    return  KKStr ("");
}  /* GetPathPartOfFile */



KKStr    KKB::osGetFileExtension (KKStr  fullFileName)
{
  KKStr   fileName, dirPath, rootName, extension;
  osParseFileName (fullFileName, dirPath, rootName, extension);
  return  extension;
}  /* osGetFileExtension */




KKStr  KKB::osGetParentDirPath (KKStr  dirPath)
{
  if  ((dirPath.LastChar () == '\\')  ||  (dirPath.LastChar () == '/'))
    dirPath.ChopLastChar ();
  
  kkint64 x = Max (dirPath.LocateLastOccurrence ('\\'), dirPath.LocateLastOccurrence ('/'));
  
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



KKStr   KKB::osGetHostName ()
{
#if  defined(KKOS_WINDOWS)
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
  kkint32  returnCd = gethostname (buff, sizeof (buff) - 2);
  if  (returnCd != 0)
    return "";
  else
    return buff;

#endif
}  /* osGetHostName */



KKStr  KKB::osGetProgName ()
{
#if  defined(KKOS_WINDOWS)
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






KKStr  KKB::osGetUserName ()
{
#if  defined(KKOS_WINDOWS)
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





kkint32  KKB::osGetNumberOfProcessors ()
{
#if  defined(KKOS_WINDOWS)
  KKStrPtr numProcessorsStr = osGetEnvVariable ("NUMBER_OF_PROCESSORS");
  kkint32  numOfProcessors = -1;
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




KKStr  KKB::osGetFileNamePartOfFile (KKStr  fullFileName)
{
  if  (fullFileName.LastChar () == DSchar)
    return  KKStr ("");

  kkint64  lastSlash =  osLocateLastSlashChar (fullFileName);
  if  (lastSlash < 0)
  {
   kkint64  colon = fullFileName.LocateCharacter (':');
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



void  KKB::osRunAsABackGroundProcess ()
{
  backGroundProcess = true;
}



bool  KKB::osIsBackGroundProcess ()
{
  return  backGroundProcess;
}



void  KKB::osWaitForEnter ()
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
KKStrListPtr  KKB::osGetListOfFiles (const KKStr&  fileSpec)
{
  WIN32_FIND_DATA     wfd;

  HANDLE  handle = FindFirstFile  (fileSpec.Str (),  &wfd);

  if  (handle == INVALID_HANDLE_VALUE)
  {
    return  NULL;
  }

  KKStrListPtr  nameList = new KKStrList (true);

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

  KKStrListPtr  nameList = new KKB::KKStrList (true);

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



KKStrListPtr  KKB::osGetListOfFiles (const KKStr&  fileSpec)
{
  KKStr  afterLastSlash;
  KKStr  afterStar;
  KKStr  beforeStar;
  KKStr  dirPath;

  kkint64 lastSlash = osLocateLastSlashChar (fileSpec);
    
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

  KKStrListPtr  resultList = new KKStrList (true);

  KKStrPtr  name = NULL;
  KKStrList::iterator  nameIDX;
  for  (nameIDX = allFilesInDirecory->begin ();  nameIDX != allFilesInDirecory->end ();  ++nameIDX)
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




void  KKB::osGetListOfFilesInDirectoryTree (const KKStr&  rootDir,
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



KKStrListPtr  KKB::osGetListOfImageFiles (KKStr  fileSpec)
{
  KKStrListPtr  imageFileNames = new KKStrList (true);

  KKStrListPtr filesInDir = osGetListOfFiles (fileSpec);
  if  (filesInDir)
  {
    KKStrList::iterator  fnIDX;
    for  (fnIDX = filesInDir->begin ();  fnIDX != filesInDir->end ();  ++fnIDX)
    {
      KKStr  fileName (**fnIDX);
      if  (SupportedImageFileFormat (fileName))
        imageFileNames->PushOnBack (new KKStr (fileName));
    }

    delete  filesInDir;
    filesInDir = NULL;
  }

  return  imageFileNames;
}  /* osGetListOfImageFiles */




#ifdef  WIN32
KKStrListPtr  KKB::osGetListOfDirectories (KKStr  fileSpec)
{
  WIN32_FIND_DATA   wfd;
  
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

  KKStrListPtr  nameList = new KKStrList (true);

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
KKStrListPtr  KKB::osGetListOfDirectories (KKStr  fileSpec)
{
  KKStr  rootDirName;
  kkint64  x = fileSpec.LocateCharacter ('*');
  if  (x > 0)
    rootDirName = fileSpec.SubStrPart ((kkint64)0, (kkint64)(x - 1));
  else
    rootDirName = fileSpec;

  osAddLastSlash (rootDirName);

  KKStrListPtr  nameList = new KKStrList (true);

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
double  KKB::osGetSystemTimeUsed ()
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
double  KKB::osGetSystemTimeUsed ()
{
  struct  tms  buff;
  times (&buff);
  double  totalTime = (double)(buff.tms_utime + buff.tms_stime);
  return  (totalTime / (double)(sysconf (_SC_CLK_TCK)));
}
#endif





#ifdef  WIN32
double  KKB::osGetUserTimeUsed ()
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




double  KKB::osGetUserTimeUsed ()
{
  struct  tms  buff;
  times (&buff);
  double  totalTime = (double)(buff.tms_utime);
  return  (totalTime / (double)(sysconf (_SC_CLK_TCK)));
}
#endif




#ifdef  WIN32
double  KKB::osGetKernalTimeUsed ()
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
double  KKB::osGetKernalTimeUsed ()
{
  struct  tms  buff;
  times (&buff);
  double  totalTime = (double)(buff.tms_stime);
  return  (totalTime / (double)(sysconf (_SC_CLK_TCK)));
}  /* osGetSystemTimeUsed */
#endif





#ifdef  WIN32
kkuint64  KKB::osGetSystemTimeInMiliSecs ()
{
  return timeGetTime();
}  /* osGetSystemTimeInMiliSecs */

#else
kkuint64  KKB::osGetSystemTimeInMiliSecs ()
{
  struct timeval now;
  gettimeofday(&now, NULL);
  return now.tv_usec/1000;
}  /* osGetSystemTimeInMiliSecs */
#endif



#ifdef  WIN32
DateTime  KKB::osGetLocalDateTime ()
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
DateTime  KKB::osGetLocalDateTime ()
{
  struct tm  *curTime;
  time_t     long_time;

  time (&long_time);                /* Get time as long integer. */
  curTime = localtime (&long_time); /* Convert to local time. */

  DateTime  dateTime ((kkint16)(curTime->tm_year + 1900),
                      (uchar)(curTime->tm_mon + 1),
                      (uchar)curTime->tm_mday,
                      (uchar)curTime->tm_hour,
                      (uchar)curTime->tm_min,
                      (uchar)curTime->tm_sec
                     );

  return  dateTime;
}  /* osGetCurrentDateTime */
#endif



#ifdef  WIN32
DateTime  KKB::osGetFileDateTime (const KKStr& fileName)
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

  return  DateTime ((kkint16)stLocal.wYear,
                    (uchar)stLocal.wMonth, 
                    (uchar)stLocal.wDay,
                    (uchar)stLocal.wHour,
                    (uchar)stLocal.wMinute,
                    (uchar)stLocal.wSecond
                   );

}  /* osGetFileDateTime */



#else

DateTime  KKB::osGetFileDateTime (const KKStr& fileName)
{
  struct  stat  buf;

  kkint32  returnCd = stat (fileName.Str (), &buf);

  if  (returnCd != 0)
  {
    return  DateTime (0, 0, 0, 0, 0, 0);
  }


  struct tm* dt =  localtime (&(buf.st_mtime));

  return  DateTime ((kkuint16)(1900 + dt->tm_year), 
                    (uchar)(dt->tm_mon + 1),
                    (uchar)dt->tm_mday,
                    (uchar)dt->tm_hour,
                    (uchar)dt->tm_min,
                    (uchar)dt->tm_sec
	           );
}
#endif



#ifdef  WIN32
kkint64  KKB::osGetFileSize (const KKStr&  fileName)
{
  WIN32_FIND_DATA   wfd;

  HANDLE  handle = FindFirstFile  (fileName.Str (), &wfd);
  if  (handle == INVALID_HANDLE_VALUE)
  {
    return  -1;
  }

  return  (kkint64)(wfd.nFileSizeHigh) * (kkint64)MAXDWORD + (kkint64)(wfd.nFileSizeLow);
}


#else

KKB::kkint64 KKB::osGetFileSize (const KKStr&  fileName)
{
  struct  stat  buf;

  kkint32  returnCd = stat (fileName.Str (), &buf);

  if  (returnCd != 0)
  {
    return  -1;
  }

  return  buf.st_size;
}
#endif




#if  defined(WIN32)
void  KKB::osDisplayWarning (KKStr  _message)
{
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
void  KKB::osDisplayWarning (KKStr  _message)
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
KKStr   KKB::osLookForFile (const KKStr&  fileName,
                            const KKStr&  srcDir
                           )
{
  KKStr  fileNameUpper (fileName);
  fileNameUpper.Upper ();

  KKStr  fileSpec = osAddSlash (srcDir) + fileName;
  //KKStr  fileSpec = osAddSlash (srcDir) + "*.*";

  // We will first look at contents of 'srcDir'  and if not there then look at sub directories
  KKStrListPtr files = osGetListOfFiles (fileSpec);
  if  (files)
  {
    for  (KKStrList::iterator nIDX = files->begin ();  nIDX != files->end ();  nIDX++)
    {
      KKStrPtr  fnPtr = *nIDX;
      if  (KKStr::StrEqualNoCase (fileName.Str (), fnPtr->Str ()))
      {
        delete  files;
        files = NULL;
        return srcDir;
      }
    }
    delete  files;
    files = NULL;
  }

  KKStrListPtr  subDirs = osGetListOfDirectories (srcDir);
  if  (subDirs)
  {
    for  (KKStrList::iterator sdIDX = subDirs->begin ();  sdIDX != subDirs->end ();  sdIDX++)
    {
      KKStr  subDirName = osAddSlash (srcDir) + **sdIDX;
      KKStr  resultDir = osLookForFile (fileName, subDirName);
      if  (!resultDir.Empty ())
      {
        delete  subDirs;
        subDirs = NULL;
        return resultDir;
      }
    }

    delete  subDirs;  subDirs = NULL;
  }

  return "";
}  /* osLookForFile */




KKStr  KKB::osCreateUniqueFileName (KKStr  fileName)
{
  if  (fileName.Empty ())
    fileName = "Temp.txt";

  KKStr   dirPath, rootName, extension;

  osParseFileName (fileName, dirPath, rootName, extension);

  kkint32  seqNum = 0;
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





KKStrPtr  KKB::osReadNextLine (FILE*  in)
{
  if  (feof (in))
    return NULL;

  KKStrPtr  buff = new KKStr (100);
  while  (true)
  {
    if  (feof (in))
      break;

    auto  ch = fgetc (in);
    if  (ch == '\r')
    {
      if  (!feof (in))
      {
        auto  nextCh = fgetc (in);
        if  (nextCh != '\n')
          ungetc (nextCh, in);
        break;
      }
    }
    else if  (ch == '\n')
    {
      break;
    }

    buff->Append ((char)ch);
    if  (buff->Len () >= uint16_max)
      break;
  }

  return  buff;
}  /* osReadNextLine */






KKStr  KKB::osReadNextToken (std::istream&  in, 
                             const char*    delimiters,
                             bool&          eof,
                             bool&          eol
                            )
{
  eof = false;
  eol = false;

  char  token[1024];
  kkint32  maxTokenLen = (kkint32)sizeof (token) - 1;

  //kkint32  ch = fgetc (in);  eof = (feof (in) != 0);
  kkint32  ch = in.get ();  
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

  kkint32 tokenLen = 0;

  // Read till first delimiter or eof
  while  ((!eof)  &&  (!strchr (delimiters, ch)))
  {
    if  (ch == '\n')
    {
      in.putback ((char)ch);
      break;
    }
    else
    {
      token[tokenLen] = (char)ch;
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



KKStr  KKB::osReadNextToken (FILE*       in, 
                             const char* delimiters,
                             bool&       eof,
                             bool&       eol
                            )
{
  eof = false;
  eol = false;

  char  token[1024];
  kkint32  maxTokenLen = (kkint32)sizeof (token) - 1;

  kkint32  ch = fgetc (in);  eof = (feof (in) != 0);

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

  kkint32 tokenLen = 0;

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
      token[tokenLen] = (char)ch;
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




KKStr  KKB::osReadNextToken (FILE*       in, 
                             const char* delimiters,
                             bool&       eof
                            )
{
  eof = false;
  char  token[1024];
  kkint32  maxTokenLen = (kkint32)sizeof (token) - 1;

  kkint32  ch = fgetc (in);  eof = (feof (in) != 0);

  if  (eof)
    return "";

  // lets skip leading white space
  while  ((!eof)  &&  ((ch == ' ') || (ch == '\r'))  &&  (ch != '\n'))
    {ch = fgetc (in); eof = (feof (in)!= 0);}

  kkint32 tokenLen = 0;

  // Read till first delimiter or eof
  while  ((!eof)  &&  (!strchr (delimiters, ch)))
  {
    token[tokenLen] = (char)ch;
    tokenLen++;
      
    if  (tokenLen >= maxTokenLen)
      break;

    ch = fgetc (in); eof = (feof (in)!= 0);
  }

  token[tokenLen] = 0;  // Terminating NULL character.


  // Remove Trailing whitespace
  while  (tokenLen > 0)
  {
    if  (strchr (" \n\r", token[tokenLen - 1]) == 0)
      break;
    tokenLen--;
    token[tokenLen] = 0;
  }

  return  token;
}  /* ReadNextToken */



KKB::KKStr  KKB::osReadRestOfLine2 (std::istream&  in,
                                    bool&          eof
                                   )
{
  KKStrPtr l = osReadRestOfLine (in, eof);
  if  (l) {
    KKStr  result(*l);
    delete l;
    l = NULL;
    return  result;
  }
  else {
    return "";
  }
}



KKStrPtr   KKB::osReadRestOfLine (std::istream&  in,
                                  bool&          eof
                                 )
{
  eof = false;

  kkint32  ch = in.get ();  
  eof = in.eof ();

  if  (eof)
    return NULL;

  KKStrPtr  result = new KKStr (1024);

  // Read till first delimiter or eof
  while  (!eof)
  {
    if  (ch == '\n')
    {
      break;
    }
    else
    {
      result->Append ((char)ch);
      if  (result->Len () >= (result->MaxLenSupported ()))
        break;
      ch = in.get ();  eof = in.eof ();
    }
  }

  result->Trim (" \n\r");

  return  result;
}  /* osReadRestOfLine */




KKStrPtr  KKB::osReadRestOfLine (FILE*  in,
                                 bool&  eof
                                )
{
  eof = false;

  kkint32  ch = fgetc (in);  
  eof = (feof (in) != 0);
  if  (eof)  
    return NULL;

  KKStrPtr  result = new KKStr(1024);
  // Read till first delimiter or eof
  while  (!eof)
  {
    if  (ch == '\n')
    {
      break;
    }
    else
    {
      result->Append ((char)ch);
      if  (result->Len () >= result->MaxLenSupported ())
        break;
      ch = fgetc (in);  eof = (feof (in) != 0);
    }
  }

  result->TrimRight (" \r\n");

  return  result;
}  /* osReadRestOfLine */



KKB::KKStr  KKB::osReadRestOfLine2 (FILE*  in,
                                    bool&  eof
                                   )
{
  KKStrPtr l = osReadRestOfLine (in, eof);
  KKStr  result(*l);
  delete l;
  l = NULL;
  return  result;
}




KKStr  KKB::osReadNextQuotedStr (FILE*        in,
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
  kkint32  ch = fgetc (in);

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
         default:   result.Append ((char)ch); break;
        }
      }
    }
    else
    {
      result.Append ((char)ch);
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




void  KKB::osSkipRestOfLine (FILE*  in,
                             bool&  eof
                            )
{
  eof = false;
  kkint32  ch = fgetc (in);  eof = (feof (in) != 0);
  while  ((ch != '\n')  &&  (!eof))
  {
    ch = fgetc (in);  eof = (feof (in) != 0);
  }
}  /* osSkipRestOfLine */




void  KKB::osSkipRestOfLine (std::istream&  in,
                             bool&          eof
                            )
{
  kkint32  ch = in.get ();  
  eof = in.eof ();

  while  ((ch != '\n')  &&  (!eof))
  {
    ch = in.get ();  
    eof = in.eof ();
  }
}  /* osSkipRestOfLine */



kkint32  KKB::osGetProcessId ()
{
#ifdef  WIN32
  //DWORD WINAPI  processId = GetCurrentProcessId ();
  DWORD processId = GetCurrentProcessId();
  return  processId;

#else
  pid_t processID = getpid ();
  return  processID;

#endif
}



kkint32  KKB::osGetThreadId ()
{
#ifdef  WIN32
  //DWORD WINAPI threadId = GetCurrentThreadId ();
  DWORD threadId = GetCurrentThreadId();
  return  threadId;
#else
  return 0;
#endif
}


void  KKB::osSleep (float secsToSleep)
{
  #ifdef  WIN32
  kkint32  miliSecsToSleep = (kkint32)(1000.0f * secsToSleep + 0.5f);
  Sleep (miliSecsToSleep);
  #else
  kkint32  secsToSleepInt = (kkint32)(0.5f + secsToSleep);

  if  (secsToSleepInt < 1)
    secsToSleepInt = 1;
 
  else if  (secsToSleepInt > 3600)
    cout  << "osSleep  secsToSleep[" << secsToSleepInt << "]" << std::endl;

  sleep (secsToSleepInt);
  #endif
}




void  KKB::osSleepMiliSecs (kkuint32  numMiliSecs)
{
  #ifdef  WIN32
    Sleep (numMiliSecs);
  #else
    int  numSecsToSleep = (numMiliSecs / 1000);
    sleep (numSecsToSleep);
  #endif
}



VectorKKStr  KKB::osSplitDirectoryPathIntoParts (const KKStr&  path)
{
  VectorKKStr  parts;

  if  (path.Len () == 0)
    return parts;

  kkuint32  zed = 0;

  if  (path[(kkuint16)1] == ':')
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
      kkint32  startPos = zed;
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


KKStr  KKB::osGetFullPathOfApplication ()
{
#if  defined(WIN32)
  char  szAppPath[MAX_PATH] = "";
  DWORD  result = ::GetModuleFileName (0, szAppPath, MAX_PATH);
  return  szAppPath;
#else
  return  KKStr::EmptyStr ();
#endif
}
