/* GoalKeeper.cpp -- Implements blocking routines to support thread synchronization.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#include  "FirstIncludes.h"

#include  <errno.h>
#include  <istream>
#include  <iostream>
//#include  <stdio.h>
#include  <vector>

#if  defined(WIN32)
#include  <windows.h>
#else
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#endif
#include  "MemoryDebug.h"

using namespace std;

#include "GoalKeeper.h"
#include "OSservices.h"
using namespace  KKU;


GoalKeeper::GoalKeeper (const KKStr&  _name):
   blocked                (false),
   blockerDepth           (0),
   blockerThreadId        (-1),
   name                   (_name),
   numBlockedThreads      (0),
   numOfBlockingInstances (0)
{
#if  defined(WIN32)
  InitializeCriticalSection (&cs);
#else
  pthread_mutex_init (&mutex, NULL);
#endif
}


GoalKeeper::~GoalKeeper ()
{
#if  defined(WIN32)
  if  (blocked)
    CriticalSectionEnd ();
#else
  pthread_mutex_destroy (&mutex);
#endif
}



int32  GoalKeeper::MemoryConsumedEstimated ()  const
{
  return  (sizeof (GoalKeeper) + name.MemoryConsumedEstimated ());
}



bool   GoalKeeper::Blocked ()
{
  return  blocked;
}  /* Blocked */



bool   GoalKeeper::BlockedByAnotherThread ()
{
  if  (!blocked)
    return false;

  int32  curThreadId = KKU::osGetThreadId ();
  return  (blocked  &&  (curThreadId != blockerThreadId));
}



void  GoalKeeper::CriticalSectionStart ()
{
#if  defined(WIN32)
  EnterCriticalSection (&cs);
#else
  pthread_mutex_lock (&mutex);
#endif
}




void  GoalKeeper::CriticalSectionEnd ()
{
#if  defined(WIN32)
  LeaveCriticalSection (&cs);
#else
  pthread_mutex_unlock (&mutex);
#endif
}



void  GoalKeeper::StartBlock ()
{
  int32  curThreadId = KKU::osGetThreadId ();
  
  bool           firstPassThruLoop = true;
  bool           weAreBlocked      = true;
  unsigned long  milliSecsBlocked  = 0;

  while  (weAreBlocked)
  {
    CriticalSectionStart ();

    if  (firstPassThruLoop)
      numBlockedThreads++;

    if  (blocked)
    {
      if  (curThreadId == blockerThreadId)
      {
        // We are the thread that already holds the block; so ok for us
        // to process.
        blockerDepth++;
        weAreBlocked = false;
        numBlockedThreads--;
      }
      else
      {
        weAreBlocked = true;
        if  (firstPassThruLoop)
          numOfBlockingInstances++;
      }
    }
    else
    {
      // No one holds the lock;  so we can take it.
      blocked = true;
      blockerDepth = 1;
      blockerThreadId = curThreadId;
      weAreBlocked = false;
      numBlockedThreads--;
    }

    CriticalSectionEnd ();

    if  (weAreBlocked)
    {
      if  (milliSecsBlocked < 10)
      {
        osSleep (0.001f);
        milliSecsBlocked++;
      }

      else if  (milliSecsBlocked < 200)
      {
        osSleep (0.010f);
        milliSecsBlocked += 10;
      }

      else if  (milliSecsBlocked < 10000)
      {
        osSleep (0.100f);
        milliSecsBlocked += 100;
      }

      else
      {
        osSleep (0.400f);
        milliSecsBlocked += 400;
      }
    }

    firstPassThruLoop = false;
  }

  return;
}   /* StartBlock */




void   GoalKeeper::EndBlock ()
{
  int32  curProcessorId = KKU::osGetThreadId ();

  int32 errorCode = 0;   // 0=No Error;  
                         // 1=There is no Block
                         // 2=Different thread holds the block
                         // 3=Failure to get a lock

  {
    CriticalSectionStart ();
  
    if  (!blocked)
    {
      errorCode = 1;
    }
    
    else if  (curProcessorId != blockerThreadId)
    {
      errorCode = 2;
    }

    else
    {
      blockerDepth--;
      if  (blockerDepth < 1)
      {
        blocked = false;
        blockerThreadId = -1;
        blockerDepth = 0;
      }
    }

    CriticalSectionEnd ();
  }

  if  (errorCode == 0)
    return;

  else if  (errorCode == 1)
    throw KKStr ("GoalKeeper::EndBlock    Name[" + name + "]  There was no block established.");

  else if  (errorCode == 2)
    throw KKStr ("GoalKeeper::EndBlock    Name[" + name + "]  ThreadId[" + curProcessorId + "] Curenly holds Block;  our ThreadId[" + curProcessorId + "]");

  return;
}  /* EndBlock */




void  GoalKeeper::Create (const KKStr&             _name,
                          volatile GoalKeeperPtr&  _newGoalKeeper
                         )
{
#if  defined(WIN32)
  HANDLE  mutexCreateHandle = CreateMutex (NULL,                 // default security attributes
                                           false,                // initially not owned
                                           "GoalKeeper_Create"
                                          ); 

  WaitForSingleObject (mutexCreateHandle, INFINITE);

  if  (_newGoalKeeper == NULL)
    _newGoalKeeper = new GoalKeeper (_name);

  ReleaseMutex (mutexCreateHandle);
  CloseHandle(mutexCreateHandle);
#else
  sem_t*  semHandle = sem_open ("GoalKeeper", O_CREAT, 0644, 1);
  if  (semHandle == SEM_FAILED)
  {
    cout << std::endl
         << "GoalKeeper::Create  Error[" << errno << "] opening '/GoalKeeper' Semaphore." << std::endl
         << std::endl;

    perror("GoalKeeper::Create   Error Opening Semaphore  'GoalKeeper'");

    throw "GoalKeeper::Create    Error opening 'GoalKeeper'.";
  }

  sem_wait (semHandle);

  if  (_newGoalKeeper == NULL)
    _newGoalKeeper = new GoalKeeper (_name);

  sem_post (semHandle);
  sem_close (semHandle);
#endif
}  /* Create */




/**
 *@brief Create a new instance of a GoalKeeper object if it has not already been done and locks it if we create it.
 *@param[in]     _name            Name to be assigned to GoalKeeper object.
 *@param[in,out] _newGoalKeeper   A pointer to the GoalKeeper that already exists or to new one that got created. 
 *@param[out]    _didNotExistYet  Indicates if the call to 'CreateAndStartBlock' had to create a new instance.
 */
void  GoalKeeper::CreateAndStartBlock(const KKStr&             _name,
                                      volatile GoalKeeperPtr&  _newGoalKeeper,
                                      bool&                    _didNotExistYet
                                     )
{
#if  defined(WIN32)
  HANDLE  mutexCreateHandle = CreateMutex (NULL,                 // default security attributes
                                           false,                // initially not owned
                                           "GoalKeeper_Create"
                                          ); 

  WaitForSingleObject (mutexCreateHandle, INFINITE);

  if  (_newGoalKeeper == NULL)
  {
    _didNotExistYet = true;
    _newGoalKeeper = new GoalKeeper (_name);
  }
  else
  {
    _didNotExistYet = false;
  }

  _newGoalKeeper->StartBlock ();

  ReleaseMutex (mutexCreateHandle);
  CloseHandle(mutexCreateHandle);
#else
  sem_t*  semHandle = sem_open ("GoalKeeper", O_CREAT, 0644, 1);
  if  (semHandle == SEM_FAILED)
  {
    cout << std::endl
         << "GoalKeeper::Create  Error[" << errno << "] opening '/GoalKeeper' Semaphore." << std::endl
         << std::endl;

    perror("GoalKeeper::Create   Error Opening Semaphore  'GoalKeeper'");

    throw "GoalKeeper::Create    Error opening 'GoalKeeper'.";
  }

  sem_wait (semHandle);

  if  (_newGoalKeeper == NULL)
  {
    _didNotExistYet = true;
    _newGoalKeeper = new GoalKeeper (_name);
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




int32  GoalKeeper::NumBlockedThreads ()
{
  int32  x = 0;
  x =  numBlockedThreads;
  return  x;
}



int32  GoalKeeper::BlockerThreadId ()
{
  int32  x = 0;
  x =  blockerThreadId;
  return  x;
}
