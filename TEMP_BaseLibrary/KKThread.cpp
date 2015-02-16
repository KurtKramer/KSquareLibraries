/* KKThread.cpp -- Manages the threads that perform the image extraction process.
 * Copyright (C) 2012 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in ImageExtractionManager.h
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

#if  defined(WIN32)
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "BasicTypes.h"
#include "KKException.h"
#include "MsgQueue.h"
#include "KKThread.h"
using namespace KKU;



#if  defined(WIN32)
//
// Usage: SetThreadName (-1, "MainThread");
//
#define MS_VC_EXCEPTION 0x406D1388

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
   DWORD  dwType;     /**< Must be 0x1000.                          */
   LPCSTR szName;     /**< Pointer to name (in user address space). */
   DWORD  dwThreadID; /**< Thread ID (-1=caller thread).            */
   DWORD  dwFlags;    /**< Reserved for future use, must be zero.   */
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





KKThread::KKThread (const KKStr&        _threadName,
                    KKThreadManagerPtr  _threadManager,
                    MsgQueuePtr         _msgQueue
                   ):
   crashed               (false),
   msgQueue              (_msgQueue),
   shutdownPrerequisites (NULL),
   startPrerequisites    (NULL),
   status                (tsNotStarted),
   shutdownFlag          (false),
   terminateFlag         (false),
   threadName            (_threadName)

{
#if  defined(WIN32)
  windowsThread  = NULL;
  windowsThreadId = 0;
#else
  threadId = 0;
#endif


}



KKThread::~KKThread ()
{
  delete  shutdownPrerequisites;  shutdownPrerequisites = NULL;
  delete  startPrerequisites;     startPrerequisites    = NULL;
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
    msgQueue->AddMsg (threadName + "::" + msg);
  }
}  /* AddMsg */



void  KKThread::TerminateThread ()
{
  terminateFlag = true;
}



void  KKThread::ShutdownThread ()
{
  shutdownFlag = true;
}


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

void  KKThread::Start (bool&  successful)
{
  windowsThread = (unsigned long*)CreateThread (NULL,
                                                0,
                                                (LPTHREAD_START_ROUTINE)ThreadStartCallBack,
                                                (KKThreadPtr)this,
                                                0,
                                                &windowsThreadId
                                               );
  if  (windowsThread == NULL)
  {
	successful = false;
    throw KKException ("Failed to create thread");
  }
  else
  {
    SetThreadPriority (windowsThread, THREAD_PRIORITY_NORMAL);
    SetThreadName ();
  }
}  /* Start */

#else
void  KKThread::Start (bool&  successful)
{
  int returnCd = pthread_create (&threadId,
                                 NULL,                  // const pthread_attr_t * address,
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
    SetThreadName ();
  }
}  /* Start */

#endif



/**
 *@brief   stops the running thread and frees the thread handle
 **/

#if  defined(WIN32)
void  KKThread::Kill ()
{
	if  (windowsThread == NULL)
    return;	

	WaitForSingleObject (windowsThread, INFINITE);
	CloseHandle (windowsThread);
	windowsThread = NULL;
}  /* Kill */
#else
void  KKThread::Kill ()
{
  pthread_cancel (this->threadId);
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

