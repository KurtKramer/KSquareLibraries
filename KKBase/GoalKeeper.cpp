/* GoalKeeper.cpp -- Implements blocking routines to support thread synchronization.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"

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

#include "KKException.h"
#include "GoalKeeper.h"
#include "OSservices.h"
using namespace  KKB;




#if  defined(GOALKEEPER_DEBUG)
class  GoalKeeper::BlockedStat
{
public:
  BlockedStat (kkint32 _blockedThreadId,
               kkint32 _blockerThreadId,
               kkuint32  _milliSecsBlocked,
               kkint32 _numBlockedThreads
              ):
    blockedThreadId   (_blockedThreadId),
    blockerThreadId   (_blockerThreadId),
    milliSecsBlocked  (_milliSecsBlocked),
    numBlockedThreads (_numBlockedThreads),
    dateTimeBlocked   (osGetLocalDateTime ())
  {
  }

  kkint32   blockedThreadId;   /**< Thread Being Blocked. */
  kkint32   blockerThreadId;   /**< Thread holding Block. */
  kkuint32  milliSecsBlocked;
  kkint32   numBlockedThreads;
  DateTime  dateTimeBlocked;
};


class  GoalKeeper::BlockedStatList: public  KKQueue<BlockedStat>
{
public:
  BlockedStatList (bool _owner): KKQueue<BlockedStat> (_owner)  {}
};

 
#endif


GoalKeeperListPtr  GoalKeeper::existingGoalKeepers = NULL;



GoalKeeper::GoalKeeper (const KKStr&  _name):
   blocked           (false),
   blockerDepth      (0),
   blockerThreadId   (-1),
   name              (_name),
   numBlockedThreads (0)
{
#if  defined(WIN32)
  InitializeCriticalSection (&cs);
#else
  pthread_mutex_init (&mutex, NULL);
#endif

#if  defined(GOALKEEPER_DEBUG)
  blockedStats = new BlockedStatList (true);
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

#if  defined(GOALKEEPER_DEBUG)
  ReportBlockedStats ();
  delete  blockedStats;
  blockedStats = NULL;
#endif

}





#if  defined(GOALKEEPER_DEBUG)
void  GoalKeeper::ReportBlockedStats ()
{
  HANDLE  mutexCreateHandle = CreateMutex (NULL,                 /**< default security attributes. */
                                           false,                /**< initially not owned.         */
                                           "GoalKeeper_ReportBlockedStats"
                                          ); 

  WaitForSingleObject (mutexCreateHandle, INFINITE);

  KKStr  path = "C:\\Temp\\BlockedStats";

  KKB::osCreateDirectoryPath (path);

  KKStr  fileName = osAddSlash (path) + "BlockedStats.txt";

  ofstream  o (fileName.Str (), ios_base::app);
  
  o << "Name"
    << "\t" << "Count"
    << "\t" << "BlockedThreadId"
    << "\t" << "BlockerThreadId"
    << "\t" << "DateTimeBlocked"
    << "\t" << "MilliSecsBlocked"
    << "\t" << "NumBlockedThreads"
    << endl;

  int  c = 0;
  BlockedStatList::iterator  idx;
  for  (idx = blockedStats->begin (), c = 0;  idx != blockedStats->end ();  ++idx, ++c)
  {
    BlockedStat*  bs = *idx;
    o << name
      << "\t" << c
      << "\t" << bs->blockedThreadId 
      << "\t" << bs->blockerThreadId 
      << "\t" << bs->dateTimeBlocked.Date () << " " << bs->dateTimeBlocked.Time ()
      << "\t" << bs->milliSecsBlocked
      << "\t" << bs->numBlockedThreads
      << endl;
  }

  o.close ();

  ReleaseMutex (mutexCreateHandle);
  CloseHandle(mutexCreateHandle);
}  /* ReportBlockedStats */
#endif



kkint32  GoalKeeper::MemoryConsumedEstimated ()  const
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

  kkint32  curThreadId = KKB::osGetThreadId ();
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
  kkint32  curThreadId = KKB::osGetThreadId ();
  
  bool    firstPassThruLoop = true;
  bool    weAreBlocked      = true;
  kkuint32  milliSecsBlocked  = 0;

  while  (weAreBlocked)
  {
    CriticalSectionStart ();

    if  (firstPassThruLoop)
      numBlockedThreads++;

    if  (blocked)
    {
      if  (curThreadId == blockerThreadId)
      {
        // We are the thread that already holds the block;  so okay for us 
        // to process.
        blockerDepth++;
        weAreBlocked = false;
        numBlockedThreads--;
      }
      else
      {
        weAreBlocked = true;
#if  defined(GOALKEEPER_DEBUG)
        blockedStats->PushOnBack (new BlockedStat (curThreadId, blockerThreadId, milliSecsBlocked, numBlockedThreads));
#endif
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
        osSleepMiliSecs (1);
        milliSecsBlocked++;
      }

      else if  (milliSecsBlocked < 200)
      {
        osSleepMiliSecs (10);
        milliSecsBlocked += 10;
      }

      else if  (milliSecsBlocked < 10000)
      {
        osSleepMiliSecs (100);
        milliSecsBlocked += 100;
      }

      else
      {
        osSleepMiliSecs (400);
        milliSecsBlocked += 400;
      }
    }

    firstPassThruLoop = false;
  }

  return;
}   /* StartBlock */




void   GoalKeeper::EndBlock ()
{
  kkint32  curProcessorId = KKB::osGetThreadId ();

  kkint32 errorCode = 0;   // 0=No Error;  
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
    throw KKStr ("GoalKeeper::EndBlock    Name[" + name + "]  ThreadId[" + curProcessorId + "] Currently holds Block;  our ThreadId[" + curProcessorId + "]");

  return;
}  /* EndBlock */




void  GoalKeeper::Create (const KKStr&             _name,
                          volatile GoalKeeperPtr&  _newGoalKeeper
                         )
{
#if  defined(WIN32)
  HANDLE  mutexCreateHandle = CreateMutex (NULL,                 /**< default security attributes */
                                           false,                /**< initially not owned */
                                           "GoalKeeperClass"
                                          );
  if  (mutexCreateHandle == NULL)
    throw KKException ("GoalKeeper::Create  failed to get handle to Mutex Object 'GoalKeeperClass'.");

  WaitForSingleObject (mutexCreateHandle, INFINITE);

  if  (_newGoalKeeper == NULL)
  {
    _newGoalKeeper = new GoalKeeper (_name);
    if  (existingGoalKeepers == NULL)
    {
      existingGoalKeepers = new GoalKeeperList (true);
      atexit (GoalKeeper::FinalCleanUp);
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
         << "GoalKeeper::Create  Error[" << errno << "] opening '/GoalKeeper' Semaphore." << std::endl
         << std::endl;

    perror("GoalKeeper::Create   Error Opening Semaphore  'GoalKeeper'");

    throw "GoalKeeper::Create    Error opening 'GoalKeeper'.";
  }

  sem_wait (semHandle);

  if  (_newGoalKeeper == NULL)
  {
    _newGoalKeeper = new GoalKeeper (_name);
    if  (existingGoalKeepers == NULL)
    {
      existingGoalKeepers = new GoalKeeperList (true);
      atexit (GoalKeeper::FinalCleanUp);
    }
    existingGoalKeepers->PushOnBack (_newGoalKeeper);
  }

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
void  GoalKeeper::CreateAndStartBlock (const KKStr&             _name,
                                       volatile GoalKeeperPtr&  _newGoalKeeper,
                                       bool&                    _didNotExistYet
                                      )
{
#if  defined(WIN32)
  HANDLE  mutexCreateHandle = CreateMutex (NULL,                 /**< default security attributes. */
                                           false,                /**< initially not owned.         */
                                           "GoalKeeperClass"
                                          );
  if (mutexCreateHandle == NULL)
    throw KKException("GoalKeeper::CreateAndStartBlock  failed to get handle to Mutex Object 'GoalKeeperClass'.");

  WaitForSingleObject (mutexCreateHandle, INFINITE);

  if  (_newGoalKeeper == NULL)
  {
    _didNotExistYet = true;
    _newGoalKeeper = new GoalKeeper (_name);

    if  (existingGoalKeepers == NULL)
    {
      existingGoalKeepers = new GoalKeeperList (true);
      atexit (GoalKeeper::FinalCleanUp);
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
    if  (existingGoalKeepers == NULL)
    {
      existingGoalKeepers = new GoalKeeperList (true);
      atexit (GoalKeeper::FinalCleanUp);
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





void  GoalKeeper::Destroy (volatile GoalKeeperPtr&  _goalKeeperInstance)
{
#if  defined(WIN32)
  HANDLE  mutexCreateHandle = CreateMutex (NULL,                 /**< default security attributes */
                                           false,                /**< initially not owned */
                                           "GoalKeeperClass"
                                          ); 
  if (mutexCreateHandle == NULL)
    throw KKException("GoalKeeper::Destroy  failed to get handle to Mutex Object 'GoalKeeperClass'.");

  WaitForSingleObject (mutexCreateHandle, INFINITE);

  if  (_goalKeeperInstance == NULL)
  {
    // Some other thread managed to destroy this instance.
  }
  else
  {
    kkint32  existingInstanceIdx =  existingGoalKeepers->PtrToIdx (_goalKeeperInstance);
    if  (existingInstanceIdx < 0)
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
         << "GoalKeeper::Create  Error[" << errno << "] opening '/GoalKeeper' Semaphore." << std::endl
         << std::endl;

    perror("GoalKeeper::Create   Error Opening Semaphore  'GoalKeeper'");

    throw "GoalKeeper::Create    Error opening 'GoalKeeper'.";
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



void   GoalKeeper::FinalCleanUp ()
{
  if  (existingGoalKeepers)
  {
    delete  existingGoalKeepers;
    existingGoalKeepers = NULL;
  }
}





kkint32  GoalKeeper::NumBlockedThreads ()
{
  kkint32  x = 0;
  x =  numBlockedThreads;
  return  x;
}



kkint32  GoalKeeper::BlockerThreadId ()
{
  kkint32  x = 0;
  x =  blockerThreadId;
  return  x;
}


