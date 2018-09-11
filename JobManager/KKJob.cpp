#include "FirstIncludes.h"

#include <stdlib.h>
#include <stdio.h>

#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#ifdef  WIN32
#include <io.h>
#include <windows.h>
#else
//#include  <sys/loadavg.h>
#include <unistd.h>
#endif


#include "MemoryDebug.h"

using namespace std;



#include "KKBaseTypes.h"
#include "KKException.h"
#include "KKQueue.h"
#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace KKB;



#include "KKJob.h"
#include "KKJobManager.h"
using namespace  KKJobManagment;



KKJob::KKJob (const KKJob&  j):

  log                  (j.log),
  manager              (j.manager),
  jobId                (j.jobId),
  parentId             (j.parentId),
  numProcessors        (j.numProcessors),
  numPorcessesAllowed  (j.numPorcessesAllowed),
  prerequisites        (j.prerequisites),
  status               (j.status)

{
}
 

  
//  To create a brand new job that has not been proceesed yet.  
KKJob::KKJob (JobManagerPtr  _manager,
              kkint32        _jobId,
              kkint32        _parentId,
              kkint32        _numPorcessesAllowed,
              RunLog&        _log
             ):

  manager              (_manager),
  jobId                (_jobId),
  parentId             (_parentId),
  numProcessors        (0),
  numPorcessesAllowed  (_numPorcessesAllowed),
  prerequisites        (),
  status               (jsOpen),
  log                  (_log)

{
}





KKJob::KKJob (JobManagerPtr  _manager):
  manager              (_manager),
  jobId                (-1),
  parentId             (-1),
  numProcessors        (0),
  numPorcessesAllowed  (1),
  prerequisites        (),
  status               (jsNULL),
  log                  (_manager->Log ())
{
}




KKJob::~KKJob ()
{
}




const char*   KKJob::JobType ()  const
{
  return  "KKJob";
}




KKJobPtr  KKJob::Duplicate ()  const
{
  return new KKJob (*this);
}



void  KKJob::ProcessNode ()
{
  // This metjod should have been overridden by a derived class;  if we end up here then
  // no actual work will get done.
  throw  KKException ("KKJob::ProcessNode   ***ERROR***    ProcessNode was not defined bydecendent class.");
}




typedef  KKJobPtr (*ConstructorPtr)(const KKStr&);

map<KKStr, KKJob::ConstructorPtr>  KKJob::registeredConstructors;



void  KKJob::RegisterConstructor (const KKStr&    _name,
                                  ConstructorPtr  _constructor
                                 )
{
  registeredConstructors.insert (pair<KKStr, ConstructorPtr>(_name, _constructor));
}
                                       



KKJobPtr  KKJob::CallAppropriateConstructor (JobManagerPtr  _manager,
                                             const KKStr&   _jobTypeName,
                                             const KKStr&   _statusStr
                                            )
{
  ConstructorIndex::iterator  idx;
  idx = registeredConstructors.find (_jobTypeName);
  if  (idx == registeredConstructors.end ())
    return NULL;
  KKJobPtr  j = idx->second (_manager);
  j->ProcessStatusStr (_statusStr);
  return  j;
}  /* CallAppropriateConstructor */





void    KKJob::ReFresh (KKJob&  j)
{
  jobId     = j.jobId;
  parentId  = j.parentId;
  manager   = j.manager;
  status    = j.status;

  numProcessors        = j.numProcessors;
  numPorcessesAllowed  = j.numPorcessesAllowed;
  prerequisites        = j.prerequisites;
  status               = j.status;
}  /* ReFresh */



KKStr  KKJob::JobStatusToStr (JobStatus  status)
{
  if  (status == jsOpen)
    return "Open";

  if  (status == jsStarted)
    return "Started";

  if  (status == jsDone)
    return "Done";

  if  (status == jsExpanded)
    return "Expanded";

  return  "";
}  /* JobStatusToStr */




KKJob::JobStatus  KKJob::JobStatusFromStr (const KKStr&  statusStr)
{
  if  (statusStr.CompareIgnoreCase ("OPEN") == 0)
    return  jsOpen;

  if  (statusStr.CompareIgnoreCase ("STARTED") == 0)
    return jsStarted;

  if  (statusStr.CompareIgnoreCase ("DONE") == 0)
    return jsDone;

  if  (statusStr.CompareIgnoreCase ("Expanded") == 0)
    return jsExpanded;

  return jsNULL;
}  /* JobStatusToStr */




KKStr  KKJob::StatusStr () const
{
  return  JobStatusToStr (status);
}  /* StatusStr */



KKStr  KKJob::PrerequisitesToStr ()  const
{
  if  (prerequisites.size () < 1)
    return  "None";

  KKStr  s (5 + (kkStrUint)prerequisites.size () * 5);
  for  (kkuint32 x = 0;  x < prerequisites.size ();  ++x)
  {
    if  (x > 0)
      s << ",";
    s << prerequisites[x];
  }
  return  s;
}  /* PrerequisitesToStr */




void  KKJob::PrerequisitesFromStr (const KKStr&  s)
{
  prerequisites.clear ();

  if  (s.CompareIgnoreCase ("None") != 0)
  {
    VectorKKStr fields = s.Split (',');
    for  (kkuint32 x = 0;  x < fields.size ();  ++x)
    {
      kkint32 p = fields[x].ToInt ();
      prerequisites.push_back (p);
    }
  }
}  /* PrerequisitesFromStr */




KKStr  KKJob::ToStatusStr ()
{
  KKStr  statusStr (200);  // Preallocating 200 bytes.

  statusStr << "JobId"                << "\t" << jobId                << "\t"
            << "ParentId"             << "\t" << parentId             << "\t"
            << "Status"               << "\t" << StatusStr ()         << "\t"
            << "NumProcessors"        << "\t" << numProcessors        << "\t"
            << "NumPorcessesAllowed"  << "\t" << numPorcessesAllowed  << "\t"
            << "Prerequisites"        << "\t" << PrerequisitesToStr ();

  return  statusStr;
}  /* ToStatusStr */




void  KKJob::ProcessStatusStr (const KKStr&  statusStr)
{
  log.Level (30) << "KKJob::ProcessStatusStr[" << statusStr << "]" << endl;
  KKStr  fieldName;
  KKStr  fieldValue;

  VectorKKStr fields = statusStr.Split ('\t');
  kkuint32  fieldNum = 0;

  while  (fieldNum < fields.size ())
  {
    fieldName = fields[fieldNum];
    fieldNum++;
    if  (fieldNum < fields.size ())
    {
      fieldValue = fields[fieldNum];
      fieldNum++;
    }
    else
    {
      fieldValue = "";
    }

    fieldName.Upper ();
    fieldValue.TrimLeft ("\n\r\t ");
    fieldValue.TrimRight ("\n\r\t ");

    if  (fieldName.CompareIgnoreCase ("JOBID") == 0)
      jobId = atoi (fieldValue.Str ());

    else  if  (fieldName.CompareIgnoreCase ("PARENTID") == 0)
      parentId = atoi (fieldValue.Str ());

    else  if  (fieldName.CompareIgnoreCase ("STATUS") == 0)
      status = JobStatusFromStr (fieldValue);
   
    else  if  (fieldName.CompareIgnoreCase ("NumProcessors") == 0)
      numProcessors = fieldValue.ToInt ();

    else  if  (fieldName.CompareIgnoreCase ("NumPorcessesAllowed") == 0)
      numPorcessesAllowed = fieldValue.ToInt ();

    else  if  (fieldName.CompareIgnoreCase ("Prerequisites") == 0)
      PrerequisitesFromStr (fieldValue);
      
    else
    {
      ProcessStatusField (fieldName, fieldValue);
    }
  }
}  /* ProcessStatusStr */





void  KKJob::ProcessStatusField (const KKStr&  fieldName,
                                 const KKStr&  fieldValue
                                )
{
   log.Level (-1) << "KKJob::ProcessStatusField  Invalid Field Name[" << fieldName << "]." << endl;
}  /* ProcessStatusField */




void  KKJob::CompletedJobDataWrite (ostream& o)
{
}


 
// Works with 'WriteCompletedJobData';  You use this to load in data written by 'WriteCompletedJobData'
void  KKJob::CompletedJobDataRead (istream& i)
{
}





KKJobList::KKJobList (JobManagerPtr  _manager):

   KKQueue<KKJob> (true),
   log            (_manager->Log ()),
   manager        (_manager)

{
}



KKJobList::KKJobList (const KKJobList&  jobs):
     KKQueue<KKJob>      (jobs.Owner ()),
     jobIdLookUpTable    (),
     jobIdLookUpTableIdx (),
     log                 (jobs.log),
     manager             (jobs.manager)
  
{
  KKJobList::const_iterator  idx;
  for  (idx = jobs.begin ();  idx != jobs.end ();  idx++)
  {
    KKJobPtr j = *idx;
    if  (Owner ())
      PushOnBack (j->Duplicate ());
    else
      PushOnBack (j);
  }
}



KKJobPtr  KKJobList::LookUpByJobId (kkint32  jobId)
{
  jobIdLookUpTableIdx = jobIdLookUpTable.find (jobId);
  if  (jobIdLookUpTableIdx == jobIdLookUpTable.end ())
    return  NULL;

  return  jobIdLookUpTableIdx->second;
}  /* LookUpByJobId */





bool  KKJobList::AllPrequisitesDone (KKJobPtr job)
{
  const VectorInt&  p = job->Prerequisites ();
  for  (kkuint32 z = 0;  z < p.size ();  ++z)
  {
    kkint32 jobId = p[z];
    KKJobPtr pj = LookUpByJobId (jobId);
    if  ((pj->Status () != KKJob::jsDone)  &&  (pj->Status () != KKJob::jsExpanded))
      return false;
  }
  return true;
}  /* AllPrequisitesDone */




KKJobPtr  KKJobList::LocateOpenJob ()
{
  kkint32  x;
  for  (x = 0;  x < QueueSize (); x++)
  {
    KKJobPtr  j = IdxToPtr (x);
    if  (j->Status () == KKJob::jsOpen)
      return  j;
  }

  return  NULL;
}  /* LocateOpenJob */




bool  KKJobList::AreAllJobsDone ()
{
  kkint32  x;
  for  (x = 0;  x < QueueSize ();  ++x)
  {
    KKJobPtr  j = IdxToPtr (x);
    if  ((j->Status () != KKJob::jsDone)  &&  (j->Status () != KKJob::jsExpanded))
      return false;
  }

  return  true;
}  /* AreAllJobsAreDone */








void   KKJobList::PushOnBack (KKJobPtr  j)
{
  jobIdLookUpTableIdx = jobIdLookUpTable.find (j->JobId ());
  if  (jobIdLookUpTableIdx != jobIdLookUpTable.end ())
  {
    log.Level (-1) << endl
                   << endl
                   << "KKJobList::PushOnBack      ***ERROR***" << endl 
                   << endl 
                   << "KKJobList::PushOnBack        Duplicate JobId[" << j->JobId () << "]" << endl
                   << endl;
    osWaitForEnter ();
    exit (-1);
  }

  KKQueue<KKJob>::PushOnBack (j);
  jobIdLookUpTable.insert (JobIdLookUpTablePair (j->JobId (), j));

  return;
}  /* PushOnBack */






bool  KKJobList::JobsStillRunning ()
{
  KKJobList::iterator  idx;
  for  (idx = begin ();  idx != end ();  ++idx)
  {
    KKJobPtr  j = *idx;
    if  (j->Status () == KKJob::jsStarted)
      return true;
  }
  return  false;
}  /* JobsStillRunning */


