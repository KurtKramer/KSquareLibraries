/* KKThread.cpp -- Manages the threads that perform the image extraction process.
 * Copyright (C) 2012-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"
#include <fstream>
#include <iostream>
#include <map>
#include <ostream>
#include <string>
#include <vector>
#include "MemoryDebug.h"
using namespace std;

#include "KKBaseTypes.h"

#if  defined(WIN32)
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "KKException.h"
#include "MsgQueue.h"
#include "OSservices.h"
#include "KKThread.h"
using namespace KKB;



#if  defined(WIN32)
//
// Usage: SetThreadName (-1, "MainThread");
//
#define MS_VC_EXCEPTION 0x406D1388

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
   DWORD  dwType;     // Must be 0x1000.
   LPCSTR szName;     // Pointer to name (in user addr space).
   DWORD  dwThreadID; // Thread ID (-1=caller thread).
   DWORD  dwFlags;    // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)
#endif



#if  defined(WIN32)

void  KKThread::SetThreadName ()
{
   Sleep(10);
   THREADNAME_INFO info;
   info.dwType = 0x1000;
   info.szName = threadName.Str ();
   info.dwThreadID = windowsThreadId;
   info.dwFlags = 0;

#if  defined(_MSC_VER)
   __try
   {
      RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
   }
#endif

}  /* SetThreadName */
#else
void  KKThread::SetThreadName ()
{

}
#endif




KKThread::KKThread (const KKStr&  _threadName,
                    MsgQueuePtr   _msgQueue
                   ):
   crashed               (false),
   msgQueue              (_msgQueue),
   shutdownFlag          (false),
   shutdownPrerequisites (NULL),
   startPrerequisites    (NULL),
   status                (tsNotStarted),
   priority              (tpNormal),
   terminateFlag         (false),
   threadId              (0),
   threadName            (_threadName)

{
#if  defined(WIN32)
  windowsThreadHandle  = NULL;
  windowsThreadId = 0;
#else
  linuxThreadId = 0;
#endif
}



KKThread::~KKThread ()
{
  delete  shutdownPrerequisites;  shutdownPrerequisites = NULL;
  delete  startPrerequisites;     startPrerequisites    = NULL;
}



int32  KKThread::MemoryConsumedEstimated ()
{
  int32  estMem = sizeof (crashed)                + 
                  sizeof (msgQueue)               + 
                  sizeof (shutdownFlag)           +
                  sizeof (startPrerequisites)     +
                  sizeof (shutdownPrerequisites)  +
                  sizeof (status)                 + 
                  sizeof (terminateFlag)          +
                  threadName.MemoryConsumedEstimated ();

  if  (msgQueue)
    estMem += msgQueue->MemoryConsumedEstimated ();

  if  (startPrerequisites)
    estMem += startPrerequisites->MemoryConsumedEstimated ();

  if  (shutdownPrerequisites)
    estMem += shutdownPrerequisites->MemoryConsumedEstimated ();

  return  estMem;
}


KKStr  KKThread::threadStatusDescs[] = {"NULL", "Started", "Running", "Stopping", "Stopped"};


const KKStr&  KKThread::ThreadStatusToStr (ThreadStatus ts)
{
  if  ((ts < 0)  ||  (ts > tsStopped))
    return KKStr::EmptyStr ();
  else
    return  threadStatusDescs[ts];
}



void  KKThread::AddMsg (KKStrPtr  msg)
{
  if  (msgQueue == NULL)
  {
    cerr << endl 
         << "KKThread::AddMsg   ***ERROR***    msgQuue is not defined."  << endl
         << "                   Msg[" << *msg << "]." << endl
         << endl;
    delete  msg;
    msg = NULL;
  }
  else
  {
    msgQueue->AddMsg (msg);
  }
}  /* AddMsg */




void  KKThread::AddMsg (const KKStr&  msg)
{
  if  (msgQueue == NULL)
  {
    cerr << endl 
         << "KKThread::AddMsg   ***ERROR***    msgQuue is not defined."  << endl
         << "                       Msg[" << msg << "]." << endl
         << endl;
  }
  else
  {
    KKStr msgTemp (msg.Len () + 20);
    msgTemp << osGetThreadId () << " - " << osGetLocalDateTime ().Time () << "->" << msg;
    msgQueue->AddMsg (msgTemp);
  }
}  /* AddMsg */



KKStrListPtr  KKThread::GetMsgs ()
{
  KKStrListPtr  results = new KKStrList (true);
  KKStrPtr  msg = msgQueue->GetNextMsg ();
  while  (msg)
  {
    results->PushOnBack (msg);
    msg = msgQueue->GetNextMsg ();
  }

  return results;
}  /* GetMsgs */



void  KKThread::TerminateThread ()
{
  terminateFlag = true;
}



bool  KKThread::ThreadStillProcessing ()  const
{
  return  ((status == KKThread::tsRunning)   ||  
           (status == KKThread::tsStarting)  ||
           (status == KKThread::tsStopping)
          );
}



void  KKThread::ShutdownThread ()
{
  shutdownFlag = true;
}



void  KKThread::WaitForThreadToStop (uint32  maxTimeToWait)
{
  if  ((status == tsNotStarted)  || 
       (status == tsStopped)     ||
       (status == tsNULL)
      )
  {
    // Thread is not running;  we can return.
    return;
  }

  uint64  startTime = KKB::osGetLocalDateTime ().Seconds ();
  uint32  timeWaitedSoFar = 0;
  while  ((status == tsRunning)  ||  (status == tsStopping))
  {
	osSleepMiliSecs (50);
    if  (maxTimeToWait > 0)
    {
      uint64 now = osGetLocalDateTime ().Seconds ();
      timeWaitedSoFar = (uint32)(now - startTime);
      if  (timeWaitedSoFar > maxTimeToWait)
        break;
    }
  }


  if  ((status != tsNotStarted)  &&
       (status != tsStopped)     &&
       (status != tsNULL)
      )
  {
    Kill ();
  }

}  /* WaitForThreadToStop */




#if  defined(WIN32)
extern "C" 
{
  unsigned int ThreadStartCallBack (void* param) 
  {
    KKThreadPtr  tp = (KKThreadPtr)param;
    tp->Status (KKThread::tsStarting);
    tp->Run ();
    tp->Status (KKThread::tsStopped);
    return 0;
  }
}
#else


void*  ThreadStartCallBack (void* param)
{
  KKThreadPtr  tp = (KKThreadPtr)param;
  tp->Status (KKThread::tsStarting);
  tp->Run ();
  tp->Status (KKThread::tsStopped);
  return 0;
}


#endif



#if  defined(WIN32)

void  KKThread::Start (ThreadPriority  _priority,
                       bool&           successful
                      )
{
  priority = _priority;

  windowsThreadHandle = (unsigned long*)CreateThread (NULL,
                                                0,
                                                (LPTHREAD_START_ROUTINE)ThreadStartCallBack,
                                                (KKThreadPtr)this,
                                                0,
                                                &windowsThreadId
                                               );
  if  (windowsThreadHandle == NULL)
  {
    successful = false;
    throw KKException ("Failed to create thread");
  }
  else
  {
    threadId = (int32)windowsThreadId;

    switch  (priority)
    {
    case  tpNULL:
    case  tpNormal:
      SetThreadPriority (windowsThreadHandle, THREAD_PRIORITY_BELOW_NORMAL);
      break;

    case  tpLow:
      SetThreadPriority (windowsThreadHandle, THREAD_PRIORITY_NORMAL);
      break;

    case  tpHigh:
      SetThreadPriority (windowsThreadHandle, THREAD_PRIORITY_ABOVE_NORMAL);
      break;
    }
    SetThreadName ();
    successful = true;
  }
}  /* Start */

#else

void  KKThread::Start (ThreadPriority  _priority,
                       bool&           successful
                      )
{
  int returnCd = pthread_create (&linuxThreadId,
                                 NULL,                   // const pthread_attr_t * attr,
                                 ThreadStartCallBack,   // void * (*start_routine)(void *),
                                 (void*)this
                                );
  if  (returnCd != 0)
  {
	  successful = false;
    throw KKException ("Failed to create thread");
  }
  else 
  {
    threadId = (int32)linuxThreadId;
    SetThreadName ();
    successful = true;
  }
}  /* Start */

#endif



/**
 *@brief   stops the running thread and frees the thread handle
 **/

#if  defined(WIN32)
void  KKThread::Kill ()
{
	if  (windowsThreadHandle == NULL)
    return;	

	WaitForSingleObject (windowsThreadHandle, INFINITE);
	CloseHandle (windowsThreadHandle);
	windowsThreadHandle = NULL;
}  /* Kill */
#else
void  KKThread::Kill ()
{
  pthread_cancel (linuxThreadId);
}  /* Kill */
#endif





void  KKThread::Run ()
{
  // This method should have been over ridden by a derived class.
 const char*  msg = "KKThread::Run   ***ERROR***       This method should have been over ridden by a derived class.";

  cerr << endl << endl << msg << endl
       << endl;
  AddMsg (msg);
}



bool  KKThread::ThereIsACircularReferenceStart (KKThreadPtr  _thread)  const
{
  if  (!startPrerequisites)
    return false;

  KKThreadList::const_iterator  idx;
  for  (idx = startPrerequisites->begin ();  idx != startPrerequisites->end ();  ++idx)
  {
    KKThreadPtr  preReq = *idx;
    if  (preReq == _thread)
      return true;

    else if  (preReq->ThereIsACircularReferenceStart (_thread))
      return true;
  }
  return  false;
}  /* ThereIsACircularReferenceStart */




bool  KKThread::ThereIsACircularReferenceShutdown (KKThreadPtr  _thread)  const
{
  if  (!shutdownPrerequisites)
    return false;

  KKThreadList::const_iterator  idx;
  for  (idx = shutdownPrerequisites->begin ();  idx != shutdownPrerequisites->end ();  ++idx)
  {
    KKThreadPtr  preReq = *idx;
    if  (preReq == _thread)
      return true;

    else if  (preReq->ThereIsACircularReferenceStart (_thread))
      return true;
  }
  return  false;
}  /* ThereIsACircularReferenceShutdown */




void  KKThread::AddStartPrerequistite (KKThreadPtr  _thread)
{
  if  (!startPrerequisites)
    startPrerequisites = new KKThreadList (false);
  startPrerequisites->PushOnBack (_thread);
}



void  KKThread::AddShutdownPrerequistite (KKThreadPtr  _thread)
{
  if  (!shutdownPrerequisites)
    shutdownPrerequisites = new KKThreadList (false);
  shutdownPrerequisites->PushOnBack (_thread);
}



bool  KKThread::OkToShutdown ()  const
{
  if  (!shutdownPrerequisites)
    return true;

  KKThreadList::const_iterator  idx;
  for  (idx = shutdownPrerequisites->begin ();  idx != shutdownPrerequisites->end ();  ++idx)
  {
    KKThreadPtr  preReq = *idx;
    if  (preReq->Status () != tsStopped)
      return false;
  }
  return  true;
}  /* OkToShutdown */



bool  KKThread::OkToStart ()  const
{
  if  (!startPrerequisites)
    return true;

  KKThreadList::const_iterator  idx;
  for  (idx = startPrerequisites->begin ();  idx != startPrerequisites->end ();  ++idx)
  {
    KKThreadPtr  preReq = *idx;
    if  (preReq->Status () != tsRunning)
      return false;
  }
  return  true;
}  /* OkToStart */





KKThreadList::KKThreadList (bool _owner):
    KKQueue<KKThread> (_owner)
{
}



KKThreadList::KKThreadList (const KKThreadList&  list):
    KKQueue<KKThread> (list)
{
}



KKThreadList::~KKThreadList ()
{
}



int32  KKThreadList::MemoryConsumedEstimated ()  const
{
  int32  memEst = sizeof (KKThreadList);

  KKThreadList::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  ++idx)
  {
    KKThreadPtr  t = *idx;
    memEst += t->MemoryConsumedEstimated ();
  }
  return  memEst;
}
