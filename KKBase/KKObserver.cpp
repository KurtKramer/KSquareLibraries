/* KKObserver.cpp -- Used to manage call-back functions in a multi-threaded environment.
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
#include "KKException.h"
#include "GlobalGoalKeeper.h"
#include "OSservices.h"
#include "KKObservable.h"
#include "KKObserver.h"
using namespace KKB;




KKObserver::KKObserver (const KKStr&  _name):
   name (_name)

{
}



KKObserver::~KKObserver ()
{
  // Let KKObservable instnces know that we are not observing them anymore.

  GlobalGoalKeeper::StartBlock ();
  for  (observablesIdx = observables.begin ();  observablesIdx != observables.end ();  ++observablesIdx)
  {
    KKObservablePtr p = observablesIdx->first;
    p->UnRegisterObserver (this);
  }
  GlobalGoalKeeper::EndBlock ();
}



kkint32  KKObserver::MemoryConsumedEstimated ()
{
  return  sizeof (*this);
}



void  KKObserver::RegisterObservable (KKObservablePtr observable)
{
  GlobalGoalKeeper::StartBlock ();
  observables.insert (std::pair<KKObservablePtr,KKObservablePtr>  (observable, observable));
  GlobalGoalKeeper::EndBlock ();
}  /* RegisterObservable */


    
void  KKObserver::UnRegisterObservable (KKObservablePtr observable)
{
  GlobalGoalKeeper::StartBlock ();
 
  observablesIdx = observables.find (observable);
  if  (observablesIdx != observables.end ())
    observables.erase (observablesIdx);

  GlobalGoalKeeper::EndBlock ();
}  /* UnRegisterObservable */




void  KKObserver::Notify (KKObservablePtr  obj)
{
}  /* Notify */

