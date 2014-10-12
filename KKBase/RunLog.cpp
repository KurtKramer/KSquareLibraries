/* RunLog.cpp -- Logging Class.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"

#include <stdio.h>
#include <string>
#include <time.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <string.h>
#include "MemoryDebug.h"
using namespace std;

#include "KKBaseTypes.h"
#include "DateTime.h"
#include "GoalKeeper.h"
#include "GlobalGoalKeeper.h"
#include "MsgQueue.h"
#include "RunLog.h"
#include "OSservices.h"
using namespace KKB;





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
  msgQueue    (NULL),
  ourLogFile  (NULL)
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



RunLog::RunLog (std::ostream& logStream):
  msgQueue    (NULL),
  ourLogFile  (NULL)
{
  fileName   = "";
  lineCount  = 0;
  ourLogFile = NULL;
  logFile    = &logStream;
  GetLoggingLevel ();
}



RunLog::RunLog (std::ostream*  logStream):
  msgQueue   (NULL),
  ourLogFile (NULL)
{
  fileName   = "";
  lineCount  = 0;
  ourLogFile = NULL;
  logFile    = logStream;
  GetLoggingLevel ();
}



RunLog::RunLog (MsgQueuePtr  _msgQueue):
  msgQueue   (_msgQueue),
  ourLogFile (NULL)
{
  logFile = NULL;
  ourLogFile = NULL;
  GetLoggingLevel ();
  lineCount = 0;
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
  if  ((curLevel > loggingLevel)  ||  (str == NULL))
    return;

  GlobalGoalKeeper::StartBlock ();

  procId = osGetThreadId ();

  if  (logFile)
  {
    if  (lineEmpty)
      DisplayTimeStamp ();
    *logFile << str;
  }

  if  (strcmp (str, "\n") == 0)
  {
    if  (msgQueue)
    {
      KKStrPtr  msgStr = new KKStr (curLine.Len () + 10);
      *msgStr << procId << " - " << osGetLocalDateTime ().Time () << "->" << curLine;
      msgQueue->AddMsg (msgStr);
    }
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

  GlobalGoalKeeper::EndBlock ();
}  /* Append */



void  RunLog::Flush ()
{
  if  (logFile)
  {
    (*logFile).flush ();
  }
}


RunLog&  RunLog::operator<< (bool  right)
{
  if  (right)
    Append ("True");
  else
    Append ("False");
  return *this;
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

# ifdef  USE_SECURE_FUNCS
    sprintf_s (buff, sizeof (buff), "%f", right);
# else
    sprintf (buff, "%f", right);
# endif

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



RunLog&  RunLog::operator<< (const KKStrPtr  right)
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



void  RunLog::WriteLine (const KKStr&  s)
{
  GlobalGoalKeeper::StartBlock ();

  if  (!curLine.Empty ())
    Append ("\n");

  if  (logFile)
  {
    logFile->write (s.Str (), s.Len ());
    (*logFile) << endl;
  }

  if  (msgQueue)
    msgQueue->AddMsg (s);

  lastLine = s;
  curLine = "";
  lineCount++;
  lineEmpty = true;

  GlobalGoalKeeper::EndBlock ();
}



void  RunLog::WriteLine (const char* s)
{
  GlobalGoalKeeper::StartBlock ();

  if  (curLine.Empty ())
    Append ("\n");

  if  (logFile)
    (*logFile) << s << endl;

  if  (msgQueue)
    msgQueue->AddMsg (s);

  lastLine = s;
  curLine = "";
  lineCount++;
  lineEmpty = true;

  GlobalGoalKeeper::EndBlock ();
}
