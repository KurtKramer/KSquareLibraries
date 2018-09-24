/* GoalKeeperSimple.cpp -- Implements blocking routines to support thread synchronization.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"
#include <cstdio>
#include <errno.h>
#include <istream>
#include <iostream>
#include <fstream>
//#include <stdio.h>
#include <vector>
#include "KKBaseTypes.h"

#if  defined(WIN32)
#include <windows.h>
#else
#include <fcntl.h>
#include <semaphore.h>
#endif
#include "MemoryDebug.h"
using namespace std;


#include "GoalKeeperSimple.h"
#include "KKException.h"
#include "OSservices.h"
using namespace  KKB;






volatile GoalKeeperSimpleListPtr  GoalKeeperSimple::existingGoalKeepers = NULL;



GoalKeeperSimple::GoalKeeperSimple (const KKStr&  _name):
   blocked           (false),
   blockerThreadId   (-1),
   levels            (0),
   name              (_name)
{
#if  defined(WIN32)
  InitializeCriticalSection (&cs);
#else
  pthread_mutex_init (&mutex, NULL);
#endif
}



GoalKeeperSimple::~GoalKeeperSimple ()
{
#if  defined(WIN32)
  if  (blocked)
    CriticalSectionEnd ();
#else
  pthread_mutex_destroy (&mutex);
#endif

}






kkMemSize  GoalKeeperSimple::MemoryConsumedEstimated ()  const
{
  return  ((kkMemSize)sizeof (GoalKeeperSimple) + name.MemoryConsumedEstimated ());
}



bool  GoalKeeperSimple::Blocked ()
{
  return  blocked;
}  /* Blocked */



bool   GoalKeeperSimple::BlockedByAnotherThread ()
{
  if  (!blocked)
    return false;

  kkint32  curThreadId = KKB::osGetThreadId ();
  return  (blocked  &&  (curThreadId != blockerThreadId));
}



void  GoalKeeperSimple::CriticalSectionStart ()
{
#if  defined(WIN32)
  EnterCriticalSection (&cs);
#else
  pthread_mutex_lock (&mutex);
#endif
}




void  GoalKeeperSimple::CriticalSectionEnd ()
{
#if  defined(WIN32)
  LeaveCriticalSection (&cs);
#else
  pthread_mutex_unlock (&mutex);
#endif
}



void  GoalKeeperSimple::StartBlock ()
{
  kkint32  curThreadId = KKB::osGetThreadId ();

  if  (blocked  &&  (curThreadId == blockerThreadId))
  {
    ++levels;
  }
  else
  {
    CriticalSectionStart ();
    levels = 1;
    blockerThreadId = curThreadId;
    blocked = true;
  }

  return;
}   /* StartBlock */




void   GoalKeeperSimple::EndBlock ()
{
  kkint32  curThreadId = KKB::osGetThreadId ();
  if  (!blocked)
    throw KKB::KKException ("GoalKeeperSimple::EndBlock    Name[" + name + "]  Is not currently blocked.");

  if  (curThreadId != blockerThreadId)
    throw KKB::KKException ("GoalKeeperSimple::EndBlock    Name[" + name + "]  ThreadId[" + blockerThreadId + "] Currently holds Block;  our ThreadId[" + curThreadId + "]");

  --levels;
  if  (levels < 1)
  {
    blocked = false;
    blockerThreadId = -1;
    CriticalSectionEnd ();
  }
}  /* EndBlock */




void  GoalKeeperSimple::Create (const KKStr&                   _name,
                                volatile GoalKeeperSimplePtr&  _newGoalKeeper
                               )
{
#if  defined(WIN32)
  HANDLE  mutexCreateHandle = CreateMutex (NULL,                 /**< default security attributes */
                                           false,                /**< initially not owned */
                                           "GoalKeeperClass"
                                          );
  if  (!mutexCreateHandle)
  {
    KKStr errMsg = "GoalKeeperSimple::Create   ***ERROR***   CreateMutex failed; returned back NULL;  _name:" + _name;
    cerr << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  WaitForSingleObject (mutexCreateHandle, INFINITE);

  if  (_newGoalKeeper == NULL)
  {
    _newGoalKeeper = new GoalKeeperSimple (_name);
    if  (existingGoalKeepers == NULL)
    {
      existingGoalKeepers = new GoalKeeperSimpleList (true);
      atexit (GoalKeeperSimple::FinalCleanUp);
    }
    existingGoalKeepers->PushOnBack (_newGoalKeeper);
  }

  ReleaseMutex (mutexCreateHandle);
  CloseHandle(mutexCreateHandle);
#else
  sem_t*  semHandle = sem_open ("GoalKeeperClass", O_CREAT, 0644, 1);
  if  (semHandle == SEM_FAILED)
  {
    cout << std::endl
         << "GoalKeeperSimple::Create  Error[" << errno << "] opening '/GoalKeeperSimple' Semaphore." << std::endl
         << std::endl;

    std::perror("GoalKeeperSimple::Create   Error Opening Semaphore  'GoalKeeperSimple'");

    throw "GoalKeeperSimple::Create    Error opening 'GoalKeeperSimple'.";
  }

  sem_wait (semHandle);

  if  (_newGoalKeeper == NULL)
  {
    _newGoalKeeper = new GoalKeeperSimple (_name);
    if  (existingGoalKeepers == NULL)
    {
      existingGoalKeepers = new GoalKeeperSimpleList (true);
      atexit (GoalKeeperSimple::FinalCleanUp);
    }
    existingGoalKeepers->PushOnBack (_newGoalKeeper);
  }

  sem_post (semHandle);
  sem_close (semHandle);
#endif
}  /* Create */



/**
 *@brief Create a new instance of a GoalKeeperSimple object if it has not already been done and locks it if we create it.
 *@param[in]     _name            Name to be assigned to GoalKeeperSimple object.
 *@param[in,out] _newGoalKeeper   A pointer to the GoalKeeperSimple that already exists or to new one that got created. 
 *@param[out]    _didNotExistYet  Indicates if the call to 'CreateAndStartBlock' had to create a new instance.
 */
void  GoalKeeperSimple::CreateAndStartBlock (const KKStr&                   _name,
                                             volatile GoalKeeperSimplePtr&  _newGoalKeeper,
                                             bool&                          _didNotExistYet
                                            )
{
#if  defined(WIN32)
  HANDLE  mutexCreateHandle = CreateMutex (NULL,                 /**< default security attributes. */
                                           false,                /**< initially not owned.         */
                                           "GoalKeeperClass"
                                          ); 
  if  (!mutexCreateHandle)
  {
    KKStr errMsg = "GoalKeeperSimple::CreateAndStartBlock   ***ERROR***   CreateMutex failed; returned back NULL;  _name:" + _name;
    cerr << endl << errMsg << endl << endl;
    throw KKException(errMsg);
  }

  WaitForSingleObject (mutexCreateHandle, INFINITE);

  if  (_newGoalKeeper == NULL)
  {
    _didNotExistYet = true;
    _newGoalKeeper = new GoalKeeperSimple (_name);

    if  (existingGoalKeepers == NULL)
    {
      existingGoalKeepers = new GoalKeeperSimpleList (true);
      atexit (GoalKeeperSimple::FinalCleanUp);
    }
    existingGoalKeepers->PushOnBack (_newGoalKeeper);
  }
  else
  {
    _didNotExistYet = false;
  }

  _newGoalKeeper->StartBlock ();

  ReleaseMutex (mutexCreateHandle);
  CloseHandle(mutexCreateHandle);
#else
  sem_t*  semHandle = sem_open ("GoalKeeperClass", O_CREAT, 0644, 1);
  if  (semHandle == SEM_FAILED)
  {
    cout << std::endl
         << "GoalKeeperSimple::Create  Error[" << errno << "] opening '/GoalKeeperSimple' Semaphore." << std::endl
         << std::endl;

    std::perror("GoalKeeperSimple::Create   Error Opening Semaphore  'GoalKeeperSimple'");

    throw "GoalKeeperSimple::Create    Error opening 'GoalKeeperSimple'.";
  }

  sem_wait (semHandle);

  if  (_newGoalKeeper == NULL)
  {
    _didNotExistYet = true;
    _newGoalKeeper = new GoalKeeperSimple (_name);
    if  (existingGoalKeepers == NULL)
    {
      existingGoalKeepers = new GoalKeeperSimpleList (true);
      atexit (GoalKeeperSimple::FinalCleanUp);
    }
    existingGoalKeepers->PushOnBack (_newGoalKeeper);
  }
  else
  {
    _didNotExistYet = false;
  }

  _newGoalKeeper->StartBlock ();

  sem_post (semHandle);
  sem_close (semHandle);
#endif
}  /* CreateAndStartBlock */



void  GoalKeeperSimple::Destroy (volatile GoalKeeperSimplePtr&  _goalKeeperInstance)
{
#if  defined(WIN32)
  HANDLE  mutexCreateHandle = CreateMutex (NULL,                 /**< default security attributes */
                                           false,                /**< initially not owned */
                                           "GoalKeeperClass"
                                          ); 
  if (!mutexCreateHandle)
  {
    KKStr errMsg = "GoalKeeperSimple::Destroy   ***ERROR***   CreateMutex failed; returned back NULL";
    cerr << endl << errMsg << endl << endl;
    throw KKException(errMsg);
  }

  WaitForSingleObject (mutexCreateHandle, INFINITE);

  if  (_goalKeeperInstance == NULL)
  {
    // Some other thread managed to destroy this instance.
  }
  else
  {
    auto  existingInstanceIdx =  existingGoalKeepers->PtrToIdx (_goalKeeperInstance);
    if  (!existingInstanceIdx)
    {
      // If not in list then a  different thread beat us to destroying this instance or it was never created to start with.
    }
    else
    {
      existingGoalKeepers->DeleteEntry (_goalKeeperInstance);
      delete  _goalKeeperInstance;
      _goalKeeperInstance = NULL;
    }
  }

  ReleaseMutex (mutexCreateHandle);
  CloseHandle(mutexCreateHandle);
#else
  sem_t*  semHandle = sem_open ("GoalKeeperClass", O_CREAT, 0644, 1);
  if  (semHandle == SEM_FAILED)
  {
    cout << std::endl
         << "GoalKeeperSimple::Create  Error[" << errno << "] opening '/GoalKeeperSimple' Semaphore." << std::endl
         << std::endl;

    std::perror("GoalKeeperSimple::Create   Error Opening Semaphore  'GoalKeeperSimple'");

    throw "GoalKeeperSimple::Create    Error opening 'GoalKeeperSimple'.";
  }

  sem_wait (semHandle);

  if  (_goalKeeperInstance == NULL)
  {
    // Some other thread managed to destroy this instance.
  }
  else
  {
    kkint32  existingInstanceIdx =  existingGoalKeepers->PtrToIdx (_goalKeeperInstance);
    if  (existingInstanceIdx >= 0)
    {
      // If not in list then a  different thread beat us to destroying this instance or it was never created to start with.
    }
    else
    {
      existingGoalKeepers->DeleteEntry (_goalKeeperInstance);
      delete  _goalKeeperInstance;
      _goalKeeperInstance = NULL;
    }
  }

  sem_post (semHandle);
  sem_close (semHandle);
#endif

}  /* Destroy */



void   GoalKeeperSimple::FinalCleanUp ()
{
  if  (existingGoalKeepers)
  {
    delete  existingGoalKeepers;
    existingGoalKeepers = NULL;
  }
}



kkint32  GoalKeeperSimple::BlockerThreadId ()
{
  kkint32  x = 0;
  x =  blockerThreadId;
  return  x;
}


