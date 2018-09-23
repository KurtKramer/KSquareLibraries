#include "FirstIncludes.h"

#include <fcntl.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "MemoryDebug.h"
using namespace std;

#include <sys/types.h>
#ifdef  WIN32
#include <io.h>
#include <windows.h>
#else
//#include  <sys/loadavg.h>
#include <unistd.h>
#endif

#include "KKBaseTypes.h"
#include "KKQueue.h"
#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace  KKB;

#include "KKJobManager.h"
using namespace  KKJobManagment;



KKJobManager::KKJobManager (const KKJobManager& j):
  KKJob (j),

  cpuTimeLastReported   (j.cpuTimeLastReported),
  cpuTimeTotalUsed      (j.cpuTimeTotalUsed),
  dateTimeStarted       (j.dateTimeStarted),
  dateTimeEnded         (j.dateTimeEnded),
  dateTimeFirstOneFound (j.dateTimeFirstOneFound),
  jobs                  (NULL),
  
  blockLevel              (j.blockLevel),
  lockFile                (j.lockFile),
  lockFileName            (j.lockFileName),
  lockFileOpened          (j.lockFileOpened),
  managerName             (j.managerName),
  expansionCount          (j.expansionCount),
  expansionFirstJobId     (j.expansionFirstJobId),
  nextJobId               (j.nextJobId),
  numJobsAtATime          (j.numJobsAtATime),
  procId                  (j.procId),
  quitRunning             (j.quitRunning),
  restart                 (j.restart),
  statusFileName          (j.statusFileName),
  statusFileNextByte      (j.statusFileNextByte),
  supportCompletedJobData (j.supportCompletedJobData)

{
  jobs = new KKJobList (*j.jobs);
}



// Make sure the the _summaryResultsFileName is deleted before we start processing.
KKJobManager::KKJobManager (JobManagerPtr _manager,                   // Ptr to job that is managing this 'KKJobManager'
                            kkint32       _jobId,
                            kkint32       _parentId,
                            kkint32       _numPorcessesAllowed,
                            const KKStr&  _managerName,               // Name of this 'KKJobManager' ; sttaus and lock file will be based on it.
                            kkint32       _numJobsAtATime,            // The number of jobs that can be allocatd at one time for a single process to execute.
                            RunLog&       _log
                           ):
  KKJob (_manager, _jobId, _parentId, _numPorcessesAllowed, _log),

  blockLevel                (0),
  cpuTimeLastReported       (0.0),
  cpuTimeTotalUsed          (0.0),
  dateTimeStarted           (),
  dateTimeEnded             (),
  dateTimeFirstOneFound     (false),
  expansionCount            (0),
  expansionFirstJobId       (0),
  jobs                      (NULL),
  lockFile                  (-1),
  lockFileName              (),
  lockFileOpened            (false),
  managerName               (_managerName),
  nextJobId                 (0),
  numJobsAtATime            (_numJobsAtATime),
  procId                    (-1), 
  quitRunning               (false),
  restart                   (false),
  statusFileName            (),
  statusFileNextByte        (0),
  supportCompletedJobData   (false)

{
  log.Level (10) << "KKJobManager::KKJobManager   ManagerId[" << JobId () << "]" << endl;
  procId = osGetProcessId ();

  lockFileName   = osGetRootName (managerName) + ".Lock";
  statusFileName = osGetRootName (managerName) + ".Status";

  jobs = new KKJobList (this);

  log.Level (10) << "KKJobManager::KKJobManager    Exiting" << endl;
}





KKJobManager::~KKJobManager ()
{
  delete  jobs;
  //EndBlock ();
}



void  KKJobManager::InitilizeJobManager (bool&  successful)
{
  successful = true;
  Block ();

  cpuTimeLastReported = osGetSystemTimeUsed ();

  if  (!osFileExists (statusFileName))
  {
    try
    {
      LoadRunTimeData ();
    }
    catch  (KKStr  errorMsg)
    {
      log.Level (-1) << endl << endl
                     << "KKJobManager::InitilizeJobManager   ***ERROR***     Load of 'LoadRunTimeData' Failed." << endl
                     << endl
                     << errorMsg << endl
                     << endl;
      successful = false;
      return;
    }
    StatusFileInitialize ();
  }
  else
  {
    StatusFileLoad ();
    try
    {
      LoadRunTimeData ();
    }
    catch  (KKStr  errorMsg)
    {
      log.Level (-1) << endl << endl
                     << "KKJobManager::InitilizeJobManager   ***ERROR***     Load of 'LoadRunTimeData' Failed." << endl
                     << endl
                     << errorMsg << endl
                     << endl;
      successful = false;
      return;
    }
  }

  EndBlock ();
}  /* InitilizeJobManager */




const char*   KKJobManager::JobType ()  const
{
  return  "KKJobManager";
}



void    KKJobManager::Block ()
{
  blockLevel++;
  log.Level (20) << "KKJobManager::Block    blockLevel[" << blockLevel << "]" << endl;
  if  (blockLevel > 1)
  {
    // We already have a Block established;  so there is no need to establish it now.
    return;
  }


  if  (lockFileOpened)
  {
    // We have out Lock and EndLock calls out of order.
    log.Level (-1)  << endl << endl 
                    << "KKJobManager::Block      *** WE ALREADY HAVE A BLOCK ESTABLISHED ***." << endl
                    << endl;
    return;
  }
  
  kkint32  count = 0;

  do  {
    lockFile = _open (lockFileName.Str (), O_WRONLY | O_CREAT | O_EXCL);
    if  (lockFile < 0)
    {
      count++;
      float  zed = (float)((procId + rand ()) % 10) + 2.0f;
      log.Level (10) << "KKJobManager::Block - We are locked out[" << count << "]  for [" << zed << "] secs."  << endl;
      osSleep (zed);
    }
  }  while  (lockFile < 0);

  if  (count > 0)
    log.Level (10) << "KKJobManager::Block   Lock has been established." << endl;

  lockFileOpened = true;

  log.Level (20) << "KKJobManager::Block - Lock is Established." << endl;
}  /* Block */



void   KKJobManager::EndBlock ()
{
  blockLevel--;

  #ifndef  WIN32
  kkint32  returnCd;
  #endif
  
   log.Level (20) << "KKJobManager::EndBlock - Ending Block    blockLevel[" << blockLevel << "]" << endl;

  if  (blockLevel > 0)
  {
    // We are still nested in Block Levels.  must reach 'blockLevel == 0' before we can UnBlock.
    return;
  }

  if  (!lockFileOpened)
  {
    log.Level (-1) << endl << endl << endl
                   << "KKJobManager::EndBlock          *** Lock file is not opened ***" << endl;
    return;
  }

  close (lockFile);
  lockFileOpened = false;

  #ifdef  WIN32
  if  (!DeleteFile (lockFileName.Str ()))
  {
     DWORD fileAttributes = GetFileAttributes (lockFileName.Str ());
     fileAttributes = FILE_ATTRIBUTE_NORMAL;
     if  (!SetFileAttributes (lockFileName.Str (), fileAttributes))
     {
       DWORD  lastErrorNum = GetLastError ();
       log.Level (-1) << "KKJobManager::EndBlock - *** ERROR *** Could not set Lock File  to  Normal  lastErrorNum: " << (kkuint64)lastErrorNum << endl;
     }
     else
     {
       if  (!DeleteFile (lockFileName.Str ()))
       {
         DWORD  lastErrorNum = GetLastError ();
         log.Level (-1) << "KKJobManager::EndBlock - Error["  << (kkuint32)lastErrorNum << "] deleting Lock File." << endl;
       }
     }
  }
  #else
  returnCd = unlink (lockFileName.Str ());
  #endif  

  log.Level (20) << "EndBlock - Unlocking" << endl;
  return;
}  /* EndBlock */



kkint32  KKJobManager::GetNextJobId ()
{
  kkint32  jobIdToReturn = nextJobId;
  nextJobId++;
  return  jobIdToReturn;
}



void  KKJobManager::StatusFileProcessLineJobStatusChange (KKStr&    statusLineStr)
{
  kkint32 expandedJobId = statusLineStr.ExtractTokenInt ("\t");
  KKJobPtr  j = jobs->LookUpByJobId (expandedJobId);
  if  (!j)
  {
    log.Level (-1) << endl << endl << endl 
                   << "ProcessStatusLineJobStatusChange    ***Error***    Could not locate Expanded" << endl
                   << endl
                   << "                                    JobId[" << expandedJobId << "]"  << endl
                   << endl;
    EndBlock ();
    osWaitForEnter ();
    exit (-1);
  }

  KKStr statusStr = statusLineStr.ExtractToken2 ("\t");
  statusStr.TrimLeft ();
  statusStr.TrimRight ();
  KKJob::JobStatus  jobStatus = KKJob::JobStatusFromStr (statusStr);
  if  (jobStatus == jsNULL)
  {
    log.Level (-1) << endl << endl << endl 
                   << "ProcessStatusLineJobStatusChange    ***Error***      Invalid Status Specified" << endl
                   << endl
                   << "                                    JobId["  << expandedJobId << "]"  << endl
                   << "                                    Status[" << statusStr     << "]"  << endl
                   << endl;
    EndBlock ();
    osWaitForEnter ();
    exit (-1);
  }
  
  j->Status (jobStatus);
}  /* ProcessStatusLineJobStatusChange */



void  KKJobManager::StatusFileProcessLine (const KKStr&  ln,
                                           istream&      statusFile
                                          )
{
  if  (ln.StartsWith ("//"))
  {
    // A coment line;  we can ignore it.
    return;
  }

  KKStr  statusStr (ln);
  KKStr  fieldName = statusStr.ExtractToken2 ("\t");
  if  (fieldName.Empty ())
  {
    // A empty line we will ignore it.
    return;
  }

  statusStr.TrimLeft ("\n\r\t ");
  statusStr.TrimRight ("\n\r\t ");

  if  (fieldName.CompareIgnoreCase ("JOB") == 0)
  {
    // We have a KKJob entr line;  the next field determines JobType fllowed by parameters for that JobType constructor.
    KKStr  jobTypeName = fieldName = statusStr.ExtractToken2 ("\t");

    KKJobPtr  j = KKJob::CallAppropriateConstructor (this, jobTypeName, statusStr);
    KKJobPtr  existingJob = jobs->LookUpByJobId (j->JobId ());
    if  (existingJob)
    {
      existingJob->ReFresh (*j);
      delete  j;  j = NULL;
    }
    else
    {
      jobs->PushOnBack (j);
    }
  }

  else if  (fieldName.EqualIgnoreCase ("CPUTIMEUSED"))
  {
    double  cpuTimeUsed = statusStr.ExtractTokenDouble ("\t");
    cpuTimeTotalUsed += cpuTimeUsed;
  }

  else if  (fieldName.EqualIgnoreCase ("CURRENTDATETIME"))
  {
    KKB::DateTime  dateTime = KKB::DateTime (statusStr);
    if  (!dateTimeFirstOneFound)
    {
      dateTimeFirstOneFound = true;
      dateTimeStarted = dateTime;
    }
    dateTimeEnded = dateTime;
  }

  else if  (fieldName.EqualIgnoreCase ("ExpansionCount"))
    expansionCount = statusStr.ToInt ();

  else if  (fieldName.EqualIgnoreCase ("ExpansionFirstJobId"))
    expansionFirstJobId = statusStr.ToInt ();

  else if  (fieldName.EqualIgnoreCase ("JobStatusChange"))
    StatusFileProcessLineJobStatusChange (statusStr);

  else if  (fieldName.EqualIgnoreCase ("NextJobId"))
    nextJobId = statusStr.ExtractTokenInt ("\t");

  else if  (fieldName.EqualIgnoreCase ("QuitRunning"))
    quitRunning = true;

  else if  (fieldName.EqualIgnoreCase ("Restart"))
    restart = false;

  else if  (fieldName.EqualIgnoreCase ("Status"))
    status = KKJob::JobStatusFromStr (statusStr);

  else
  {
    log.Level (-1) << "KKJobManager::StatusFileProcessLine  Invalid Field Name[" << fieldName << "]." << endl;
  }
}  /* StatusFileProcessLine */



void  KKJobManager::StatusFileLoad ()
{
  log.Level (10) << "KKJobManager::StatusFileLoad." << endl;

  statusFileNextByte = 0;
  StatusFileRefresh ();

  log.Level (20) << "KKJobManager::StatusFileLoad    Exiting." << endl;
}  /* StatusFileLoad */



void  KKJobManager::StatusFileRefresh ()
{
  // we only want to read in any new changes to the status file.
  log.Level (10) << "KKJobManager::StatusFileRefresh     statusFileName[" << statusFileName << "]"  << endl;

  
  ifstream*  statusFile = new ifstream (statusFileName.Str ());
  if  (!statusFile->is_open ())
  {
     log.Level (-1) << endl
                    << "KKJobManager::LoadCurrentStatusFile   ***ERROR***    Can not open Status File[" 
                    << statusFileName << "]." 
                    << endl;
     EndBlock ();
     delete  statusFile;  statusFile = NULL;
     osWaitForEnter ();
     exit (-1);
  }

  char  buff[20480];
  KKStr  statusStr (512);

  if  (statusFileNextByte >= 0)
    statusFile->seekg (statusFileNextByte);

  //fseek (statusFile, statusFileNextByte, SEEK_SET);

  statusFile->getline (buff, sizeof (buff));
  while  (!statusFile->eof ())
  {
    statusStr = buff;
    if  (statusStr.StartsWith ("<KKJob "))
    {
      ProcessJobXmlBlockOfText (statusStr, *statusFile);
    }
    else
    {
      StatusFileProcessLine (statusStr, *statusFile);
    }

    wstreampos  zed = statusFile->tellg ();
    if  (zed >= 0)
      statusFileNextByte = zed;

    if  (!statusFile->eof ())  
      statusFile->getline (buff, sizeof (buff));
  }

  statusFile->close ();
  delete  statusFile;
  statusFile = NULL;

  log.Level (20) << "KKJobManager::StatusFileRefresh     Exiting."  << endl;
}  /* StatusFileRefresh */



void   KKJobManager::ProcessJobXmlBlockOfText (const KKStr&  startStr,
                                               istream&      i
                                              )
{
  if  ((startStr.SubStrPart (0, 4) != "<KKJob ")  ||  (startStr.LastChar () != '>'))
  {
    log.Level (-1) << endl 
                   << "KKJobManager::ProcessJobXmlBlockOfText   ***ERROR***   StartStr[" << startStr << "] is not a KKJob String." << endl
                   << endl;
    return;
  }

  KKStr s = startStr.SubStrPart (5);
  s.TrimLeft ();
  s.ChopLastChar ();

  KKStr  jobTypeStr = "";
  kkint32 thisJobId = -1;

  VectorKKStr  parameters = s.Split (',');
  for  (kkuint32 x = 0;  x < parameters.size ();  ++x)
  {
    KKStr  parameterStr = parameters[x];
    parameterStr.TrimLeft ();
    parameterStr.TrimRight  ();

    KKStr  fieldName = parameterStr.ExtractToken2 ("=");
    fieldName.TrimLeft  ();   fieldName.TrimRight  ();

    KKStr  fieldValue = parameterStr.ExtractToken2 ("=");
    fieldValue.TrimLeft ();   fieldValue.TrimRight ();

    if  (fieldName.CompareIgnoreCase ("JobType") == 0)
      jobTypeStr = fieldValue;

    else if  (fieldName.CompareIgnoreCase ("JobId") == 0)
      thisJobId = fieldValue.ToInt ();
  }

  
  if  (jobTypeStr.Empty () ||  (thisJobId < 0))
  {
    log.Level (-1) << endl 
                   << "KKJobManager::ProcessJobXmlBlockOfText   ***ERROR***   StartStr[" << startStr << "]." << endl
                   << "                             JobType and/or JobId were not provided."               << endl
                   << endl;
    return;
  }


  KKJobPtr  j = jobs->LookUpByJobId (thisJobId);
  if  (j == NULL)
  {
    // We do not have this job in memory yet.  We will have to create it now.
    KKStr  emptyStatusStr = "JobId\t" + StrFormatInt (thisJobId, "ZZZZ0");
    j = KKJob::CallAppropriateConstructor (this, jobTypeStr, emptyStatusStr);
  }


  j->CompletedJobDataRead (i);
}  /* ProcessJobXmlBlockOfText */



ofstream*   KKJobManager::StatusFileOpen (ios::openmode  openMode)
{
  log.Level (20) << "KKJobManager::StatusFileOpen."  << endl;

  kkint32  openAttempts = 0;

  ofstream*  statusFile  = new ofstream (statusFileName.Str (), openMode);

  while  ((!statusFile->is_open ()) &&  (openAttempts < 4))
  {
    openAttempts++;
    osSleep (2.0f);
    log.Level (10) << "StatusFileOpen   Open Attempt[" << openAttempts << "]." << endl;
    statusFile  = new ofstream (statusFileName.Str (), openMode);
    if  (!statusFile->is_open ())
    {
      log.Level (0) << "StatusFileOpen    -   *** WARNING ***  Can Not Open Status File[" 
                    << statusFileName.Str () << "]   -Will Retry-." 
                    << endl;
    }
  }

  if  (!statusFile->is_open ())
  {
     log.Level (-1) << "StatusFileOpen   ***ERROR***  Can not Create Status File[" << statusFileName.Str () << "]." << endl;
     EndBlock ();
     osWaitForEnter ();
     exit (-1);
  }

  log.Level (20) << "KKJobManager::StatusFileOpen     Exiting"  << endl;

  return  statusFile;
}  /* StatusFileOpen */



void  KKJobManager::StatusFileWrite ()
{
  log.Level (10) << "KKJobManager::StatusFileWrite" << endl;

  ofstream*  statusFile = StatusFileOpen (ios::out);

  *statusFile << "// Date/Time [" << osGetLocalDateTime () << "]." << endl
              << "//" << endl
              << endl;

  *statusFile << "Status"              << "\t" << KKJob::JobStatusToStr (status)  << endl
              << "NextJobId"           << "\t" << nextJobId                     << endl
              << "CurrentDateTime"     << "\t" << osGetLocalDateTime ()         << endl
              << "ExpansionCount"      << "\t" << expansionCount                << endl
              << "ExpansionFirstJobId" << "\t" << expansionFirstJobId           << endl
              << endl;

  kkint32  x;
  for  (x = 0;  x < jobs->QueueSize ();  x++)
  {
    KKJobPtr  j = jobs->IdxToPtr (x);
    *statusFile << "KKJob" << "\t" << j->JobType () << "\t" << j->ToStatusStr () << endl;
  }

  statusFile->flush ();
  statusFile->close ();
  delete  statusFile;

  log.Level (10) << "KKJobManager::StatusFileWrite    Exiting" << endl;
}  /* StatusFileWrite */



void  KKJobManager::ReportCpuTimeUsed (ofstream* statusFile)
{
  // While we have the status file open lets report CPU time used so far
  double  currentCpuTime = osGetSystemTimeUsed ();
  double  cpuTimeUsed = currentCpuTime - cpuTimeLastReported;
  cpuTimeLastReported = currentCpuTime;
  *statusFile << "CpuTimeUsed"     << "\t" << cpuTimeUsed   << "\t"
              << "ProcId"          << "\t" << procId        << "\t"
              << endl
              << "CurrentDateTime" << "\t" << osGetLocalDateTime () << endl;
}  /* ReportCpuTimeUsed */



void  KKJobManager::StatusFileInitialize ()
{
  log.Level (10) << "KKJobManager::InializeStatusFile" << endl;

  delete  jobs;
  jobs = new KKJobList (this);

  ofstream*  statusFile = StatusFileOpen (ios::out);

  *statusFile << "// Date/Time [" << osGetLocalDateTime () << "]." << endl
              << "//" << endl
              << "//" << endl;


  *statusFile << "Status"              << "\t" << KKJob::JobStatusToStr (status)  << endl
              << "NextJobId"           << "\t" << nextJobId                     << endl
              << "CurrentDateTime"     << "\t" << osGetLocalDateTime ()         << endl
              << "ExpansionCount"      << "\t" << expansionCount                << endl
              << "ExpansionFirstJobId" << "\t" << expansionFirstJobId           << endl;

  
  StatusFileInitialize (*statusFile);  // Have derived classes do there initilization.

  delete  jobs;  jobs = NULL;
  jobs = JobsCreateInitialSet ();

  if  (jobs)
  {
    KKJobList::iterator  idx;
    for  (idx = jobs->begin ();  idx != jobs->end ();  idx++)
    {
      KKJobPtr j = *idx;
      *statusFile << "KKJob" << "\t" << j->JobType ()  << "\t" << j->ToStatusStr () << endl;
    }
  }
  else
  {
    jobs = new KKJobList (this);
  }

  statusFile->flush ();
  statusFile->close ();
  delete  statusFile;

  log.Level (10) << "KKJobManager::InializeStatusFile    Exiting" << endl;
}  /* StatusFileInitialize */



kkint32  KKJobManager::AllocateNextJobId ()
{
  kkint32  jobIdToReturn = nextJobId;
  nextJobId++;
  return  jobIdToReturn;
}



void   KKJobManager::Update (JobManagerPtr  p)
{
  status = p->status;
}



KKStr  KKJobManager::ToStatusStr ()
{
  KKStr  statusStr (200);  // PreAllocate 200 bytes

  statusStr << KKJob::ToStatusStr ()                                            << "\t"
            << "Status"               << "\t" << KKJob::JobStatusToStr (status) << "\t"
            << "NextJobId"            << "\t" << nextJobId                    << "\t"
            << "ExpansionCount"       << "\t" << expansionCount               << "\t"
            << "ExpansionFirstJobId"  << "\t" << expansionFirstJobId;

  return  statusStr;
}  /* ToStatusStr */



void  KKJobManager::ProcessRestart ()
{
  log.Level (10) << "ProcessRestart" << endl;

  Block ();

  StatusFileLoad ();

  if  (this->Status () != jsDone)
  {
    ofstream* statusFile = StatusFileOpen (ios::app);

    status = KKJob::jsOpen;
    *statusFile << "ReStart" << endl;
    *statusFile << "Status"  << "\t" << StatusStr () << endl;

    KKJobList::iterator  idx;
    for  (idx = jobs->begin ();  idx != jobs->end ();   idx++)
    {
      KKJobPtr  j = *idx;
      if  (j->Status () == jsStarted)
      {
        j->Status (jsOpen);
        *statusFile << "JobStatusChange" << "\t" << j->JobId () << "\t" << j->StatusStr () << endl;
      }
    }

    statusFile->flush ();
    statusFile->close ();
    delete  statusFile;
  }
 
  EndBlock ();
}  /* ProcessRestart */



void  KKJobManager::SetQuitRunningFlag ()
{
  Block ();
  ofstream* statusFile = StatusFileOpen (ios::app);

  *statusFile << "QuitRunning" << endl;
  quitRunning = true;

  statusFile->flush ();
  statusFile->close ();
  delete  statusFile;  statusFile = NULL;

  EndBlock ();
}  /* SetQuitRunningFlag */



void   KKJobManager::ProcessNextExpansion (ostream&  o)
{
  KKJobListPtr jobsJustCompleted = new KKJobList (this);
  jobsJustCompleted->Owner (false);
  {
    // Add jobs completed since last expansion to this list.
    KKJobList::iterator  idx;
    for  (idx = jobs->begin ();  idx != jobs->end ();  idx++)
    {
      KKJobPtr  j = *idx;
      if  (j->JobId () >= expansionFirstJobId)
        jobsJustCompleted->PushOnBack (j);
    }
  }

  // Derived class will now peform expansion.  
  KKJobListPtr  expandedJobs = JobsExpandNextSetOfJobs (jobsJustCompleted);
  {
    if  (expandedJobs)
    {
      expandedJobs->Owner (false);
      KKJobList::iterator  idx;
      for  (idx = expandedJobs->begin ();  idx != expandedJobs->end ();  idx++)
      {
        KKJobPtr  j = *idx;
        if  (j->JobId () < expansionFirstJobId)
          expansionFirstJobId = j->JobId ();

        jobs->PushOnBack (j);
        o << "KKJob" << "\t" << j->JobType () << "\t" << j->ToStatusStr () << endl;
      }
    }
    delete  expandedJobs;
    expandedJobs = NULL;
  }

  delete  jobsJustCompleted;
  jobsJustCompleted = NULL;

  o << "NextJobId"            << "\t" << nextJobId            << endl
    << "ExpansionCount"       << "\t" << expansionCount       << endl
    << "ExpansionFirstJobId"  << "\t" << expansionFirstJobId  << endl;

}  /* ProcessNextExpansion */



KKJobListPtr  KKJobManager::GetNextSetOfJobs (KKJobListPtr  completedJobs)
{
  log.Level (20) << "KKJobManager::GetNextSetOfJobs." << endl;

  Block  ();

  StatusFileRefresh ();
 
  if  (completedJobs)
  {
    // We will first write out results of jobs that have been completed,
    ofstream* statusFile = StatusFileOpen (ios::app);

    KKJobList::iterator  idx;
    for  (idx = completedJobs->begin ();  idx != completedJobs->end ();  idx++)
    {
      KKJobPtr  j = *idx;

      *statusFile << "KKJob" << "\t" << j->JobType () << "\t" << j->ToStatusStr () << endl;
      if  (supportCompletedJobData)
      {
        *statusFile << "<KKJob JobType=" << j->JobType () << ", " << "JobId=" << j->JobId () << ">" << endl;
        j->CompletedJobDataWrite (*statusFile);
        *statusFile << "</job>" << endl;
      }

      KKJobPtr  existingJob = jobs->LookUpByJobId (j->JobId ());
      if  (existingJob)
      {
        existingJob->ReFresh (*j);
      }
      else
      {
        log.Level (-1) << endl << endl << endl
                       << "GetNextSetOfJobs     *** ERROR ***"  << endl
                       << endl
                       << "           Could not locate KKJob[" << j->JobId () << "]" << endl
                       << endl;
        return  NULL;
      }
    }

    statusFile->close ();
    delete  statusFile;  statusFile = NULL;
  }
  
  KKJobListPtr  jobsToExecute = new KKJobList (this);
  jobsToExecute->Owner (false);

  if  (!quitRunning)
  {
    ofstream* statusFile = StatusFileOpen (ios::app);

    KKJobPtr  nextJob = jobs->LocateOpenJob ();

    if  (!nextJob)
    {
      if  (jobs->AreAllJobsDone ())
      {
        // There are no jobs to do;  we will have to expand some existing jobs then
        ProcessNextExpansion (*statusFile);
        nextJob = jobs->LocateOpenJob ();
      }
      else
      {
        // There are still some jobs that are running.   We are going to go to
        // for now and try again later.
        //  
        // By leaving  "nextJob = NULL" we will drop strait through the rest of
        // this method and return to the caller with 'jobsToExecute' empty
        // signaling that it will need to sleep for a while before calling 
        // us again.
      }
    }

    while  (nextJob  &&  (jobsToExecute->QueueSize () < numJobsAtATime))
    {
      jobsToExecute->PushOnBack (nextJob);
      nextJob->Status (jsStarted);
      *statusFile << "JobStatusChange" << "\t" << nextJob->JobId () << "\t" << nextJob->StatusStr () << endl;
      nextJob = jobs->LocateOpenJob ();
    }

    statusFile->close ();
  }

  if  (jobsToExecute->QueueSize () < 1)
  {
    delete  jobsToExecute;
    jobsToExecute = NULL;
  }

  EndBlock ();

  log.Level (20) << "KKJobManager::GetNextSetOfJobs   Exiting." << endl;

  return  jobsToExecute;
}  /* GetNextSetOfJobs */



bool  KKJobManager::AreAllJobsDone ()
{
  if  (jobs)
  {
    return  jobs->AreAllJobsDone ();
  }
  else
  {
    // Will assume false for now.
    return  false;
  }
}  /* AreAllJobsAreDone */



void   KKJobManager::Run ()
{
  log.Level (10) << "KKJobManager::Run." << endl;

  bool  keepOnRunning = true;

  KKJobListPtr  executedJobs  = NULL;
  KKJobListPtr  jobsToExecute = GetNextSetOfJobs (NULL);

  while  (keepOnRunning  &&  (!quitRunning))
  {
    if  (jobsToExecute)
    {
      delete  executedJobs;  executedJobs = NULL;

      executedJobs = new KKJobList (this);
      executedJobs->Owner (true);

      KKJobList::iterator  idx;
      for  (idx = jobsToExecute->begin ();  idx != jobsToExecute->end ();  idx++)
      {
        KKJobPtr  j = *idx;
        j->ProcessNode ();
        executedJobs->PushOnBack (j->Duplicate ());
      }
      delete  jobsToExecute;  jobsToExecute = NULL;
    }
    else
    {
      if  (!(jobs->JobsStillRunning ()))
      {
        keepOnRunning = false;
      }
      else
      {
        // We will sleep for a bit until there are more jobs to run
        log.Level (10) << "KKJobManager::Run     No jobs avaialble to run; will sleep a bit." << endl;
        osSleep ((float)(30 + rand () % 10)); 
      }
    }

    if  (keepOnRunning)
      jobsToExecute = GetNextSetOfJobs (executedJobs);

    delete  executedJobs;  executedJobs = NULL;
  }

  Block ();
  StatusFileRefresh ();
  if  ((!quitRunning)  &&  (status != KKJob::jsDone))
  {
    if  (status != KKJob::jsDone)
    {
      GenerateFinalResultsReport ();
      
      status = KKJob::jsDone;
    
      ofstream*  statusFile = StatusFileOpen (ios::app);

      *statusFile << "Status" << "\t" << StatusStr () << endl;

      ReportCpuTimeUsed (statusFile);

      statusFile->close ();
      delete  statusFile;
    }
  }
  EndBlock ();

  log.Level (10) << "KKJobManager::Run    Exiting." << endl;
}  /* Run */
