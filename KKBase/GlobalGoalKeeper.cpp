/* GlobalGoalKeeper.cpp -- Implements a single GoalKeeper that can be used anywhere in the application.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"

#include <errno.h>
#include <vector>
#include "MemoryDebug.h"

using namespace std;

#include "GoalKeeper.h"
#include "GlobalGoalKeeper.h"
using namespace  KKB;



GoalKeeperPtr  GlobalGoalKeeper::globalGoalie = NULL;


void  GlobalGoalKeeper::CreateGlobalGoalie ()
{
  bool  didNotExist = false;
  GoalKeeper::CreateAndStartBlock ("GlobalGoalie", globalGoalie, didNotExist);

  if  (didNotExist)
    atexit (GlobalGoalKeeper::CleanUp);

  globalGoalie->EndBlock ();
}


void  GlobalGoalKeeper::CleanUp ()
{
  GoalKeeper::Destroy (globalGoalie);
  globalGoalie = NULL;
}



void  GlobalGoalKeeper::StartBlock ()
{
  if  (!globalGoalie)
    CreateGlobalGoalie ();
  globalGoalie->StartBlock ();
}



void  GlobalGoalKeeper::EndBlock ()
{
  if  (!globalGoalie)
    CreateGlobalGoalie ();
  globalGoalie->EndBlock ();
}

