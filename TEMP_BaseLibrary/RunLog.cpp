/* RunLog.cpp -- Logging Class.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#include "FirstIncludes.h"

#include <stdio.h>
#include <string>
#include <time.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <string>
#include <sstream>
#include <vector>
#include "MemoryDebug.h"
using namespace std;

#include "BasicTypes.h"
#include "DateTime.h"
#include "MsgQueue.h"
#include "RunLog.h"
#include "OSservices.h"
using namespace KKU;





RunLog::RunLog ():
  msgQueue   (NULL),
  ourLogFile (NULL)
{
  logFile = &cout;
  ourLogFile = NULL;
  GetLoggingLevel ();
  lineCount = 0;
}



RunLog::RunLog (const char*  _fileName):
  msgQueue   (NULL),
  ourLogFile (NULL)
{
  fileName   = _fileName;
  lineCount  = 0;

  ourLogFile = new ofstream (fileName.Str ());
  logFile = ourLogFile;
  GetLoggingLevel ();
}


RunLog::RunLog (const KKStr&  _fileName):
  msgQueue   (NULL),
  ourLogFile (NULL)
{
  fileName   = _fileName;
  lineCount  = 0;
  ourLogFile = new ofstream (fileName.Str ());
  logFile = ourLogFile;
  GetLoggingLevel ();
}



RunLog::RunLog (ostream& logStream):
  msgQueue   (NULL),
  ourLogFile (NULL)
{
  fileName   = "";
  lineCount  = 0;
  ourLogFile = NULL;
  logFile    = &logStream;
  GetLoggingLevel ();
}



RunLog::RunLog (ostream*  logStream):
  msgQueue   (NULL),
  ourLogFile (NULL)
{
  fileName   = "";
  lineCount  = 0;
  ourLogFile = NULL;
  logFile    = logStream;
  GetLoggingLevel ();
}



RunLog::~RunLog ()
{
  if  (ourLogFile)
    delete  ourLogFile;
}



int32  RunLog::MemoryConsumedEstimated ()  const
{
  return  sizeof (RunLog) + 
          curLine.MemoryConsumedEstimated   () +
          fileName.MemoryConsumedEstimated  () +
          lastLine.MemoryConsumedEstimated  () +
          sizeof (*ourLogFile);
}  /* MemoryConsumedEstimated */



void  RunLog::AttachFile (const KKStr&  _fileName)
{
  fileName = _fileName;

  ofstream*  newLogFile = new ofstream (fileName.Str ());
  
  logFile = newLogFile;

  if  (ourLogFile)
    delete  ourLogFile;

  ourLogFile = newLogFile;
}  /* AttachFile */



void  RunLog::AttachFileAppend (const KKStr&  _fileName)
{
  fileName = _fileName;

  ofstream*  newLogFile = new ofstream (fileName.Str (), ios::app);
  
  logFile = newLogFile;

  if  (ourLogFile)
    delete  ourLogFile;

  ourLogFile = newLogFile;
}  /* AttachFileAppend */



void  RunLog::AttachMsgQueue (MsgQueuePtr  _msgQueue)
{
  msgQueue = _msgQueue;
}



void  RunLog::AttachFile (std::ostream&  _logFile)
{
  fileName = "";

  if  (ourLogFile)
  {
    delete  ourLogFile;
    ourLogFile = NULL;
  }

  logFile = &_logFile;
}  /* AttachFile */



void  RunLog::DetachFile ()
{
  fileName = "";
  logFile = &cout;

  if  (ourLogFile)
    delete  ourLogFile;
  ourLogFile = NULL;
}



KKStr  RunLog::FileName ()
{
  return  fileName;
}



void  RunLog::GetLoggingLevel ()
{
  curLevel     = 0;
  loggingLevel = 10;
  lineEmpty    = true;

  procId = osGetProcessId ();
}



void  RunLog::SetLoggingLevel (int32 _loggingLevel)
{
  curLevel     = 0;
  loggingLevel = _loggingLevel;
  lineEmpty    = true;
}



RunLog&  RunLog::Level (int32 _level)
{
  if  (!lineEmpty)
    Append ("\n");

  curLevel  = _level;
  lineEmpty = true;
  return  *this;
}



void  RunLog::DisplayTimeStamp ()
{
  if  (!logFile)
    return;

  if  (!lineEmpty)
    return;

  if  (curLevel > loggingLevel)
    return;

  DateTime curTime = osGetLocalDateTime ();
  if  (procId > 0)
    (*logFile) << procId << " - " ;
  (*logFile) << curTime.Time () << "->";

  lineEmpty = false;
}  /* DisplayTimeStamp */



void  RunLog::Append (const char*  str)
{
  if  (!logFile)
    return;

  if  ((curLevel <= loggingLevel)  &&  (str != NULL))
  {
    DisplayTimeStamp ();
    *logFile << str;
    //if  (ourLogFile)
    //  cout << str;

    if  (strcmp (str, "\n") == 0)
    {
      if  (msgQueue)
        msgQueue->AddMsg (curLine);
      lastLine = curLine;
      curLine = "";
      lineCount++;
      lineEmpty = true;
    }
    else
    {
      curLine << str;
      lineEmpty = false;
    }

    if  (curLevel <= 10)
      Flush ();
  }
}  /* Append */



void  RunLog::Flush ()
{
  if  (!logFile)
    return;

  (*logFile).flush ();
  //if  (ourLogFile)
  //  cout.flush ();
}



RunLog&  RunLog::operator<< (int16  right)
{
  KKStr  s (30);
  s = StrFormatInt (right, "0");
  Append (s.Str ());
  return  *this;
}



RunLog&  RunLog::operator<< (uint16 right)
{
  KKStr  s (30);
  s = StrFormatInt (right, "0");
  Append (s.Str ());
  return  *this;
}



RunLog&  RunLog::operator<< (int32  right)
{
  KKStr  s (30);
  s.AppendInt32 (right);
  Append (s.Str ());
  return  *this;
}



RunLog&  RunLog::operator<< (uint32  right)
{
  KKStr  s (30);
  s.AppendUInt32 (right);
  Append (s.Str ());
  return  *this;
}



RunLog&  RunLog::operator<< (int64  right)
{
  KKStr  s (30);
  s = StrFormatInt64 (right, "0");
  Append (s.Str ());
  return  *this;
}



RunLog&  RunLog::operator<< (uint64  right)
{
  KKStr  s (30);
  s = StrFormatInt64 (right, "0");
  Append (s.Str ());
  return  *this;
}



RunLog&  RunLog::operator<< (double  right)
{
  char  buff[50];
  SPRINTF (buff, sizeof (buff), "%.17g", right);
  Append (buff);
  return  *this;
}



RunLog&  RunLog::operator<< (char  right)
{
  char  buff[20];
  buff[0] = right;
  buff[1] = 0;
  Append (buff);
  return  *this;
}



RunLog&  RunLog::operator<< (const char*  right)
{
  Append (right);
  return  *this;
}



RunLog&  RunLog::operator<< (const KKStr&  right)
{
  Append (right.Str ());
  return  *this;
}



RunLog&  RunLog::operator<< (KKStrConstPtr  right)
{
  if  (right)
    Append (right->Str ());
  return  *this;
}


RunLog& RunLog::operator<< (ostream& (* mf)(ostream &))
{
  if  (curLevel <= loggingLevel)
  {
    ostringstream  o;
    mf (o);
    Append (o.str ().c_str ());
  }
  return  *this;
}

