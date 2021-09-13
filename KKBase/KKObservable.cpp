/* KKObservable.cpp -- Used to manage call-back functions in a multi-threaded environment.
 * Copyright (C) 2012 - 2014 Kurt Kramer
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

#if  defined(WIN32)
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "KKBaseTypes.h"
#include "GlobalGoalKeeper.h"
#include "OSservices.h"
#include "KKObserver.h"
#include "KKObservable.h"
using namespace KKB;


KKObservable::KKObservable ()
{
}



KKObservable::~KKObservable ()
{
}



size_t  KKObservable::MemoryConsumedEstimated () const
{
  return  sizeof (*this);
}



void  KKObservable::RegisterObserver (KKObserverPtr observer)
{
  GlobalGoalKeeper::StartBlock ();

  observers.insert (pair<KKObserverPtr,KKObserverPtr>(observer, observer));
  observer->RegisterObservable (this);

  GlobalGoalKeeper::EndBlock ();
}  /* RegisterObserver */


    
void  KKObservable::UnRegisterObserver (KKObserverPtr observer)
{
  GlobalGoalKeeper::StartBlock ();
 
  observersIdx = observers.find (observer);
  if  (observersIdx != observers.end ())
    observers.erase (observersIdx);

  observer->UnRegisterObservable (this);

  GlobalGoalKeeper::EndBlock ();
}  /* RegisterObserver */




void  KKObservable::NotifyObservers ()
{
  for  (observersIdx = observers.begin ();  observersIdx != observers.end ();  ++observersIdx)
    observersIdx->first->Notify (this);

}


