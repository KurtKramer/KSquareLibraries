/* GlobalGoalKeeper.cpp -- Implements a single GoalKeeper that can be used anywhere in the application.
 * Copyright (C) 1994-2013 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 *
 *  This module originally developed as part of the KKU library; brought into this library for convenience sake.
 */
#include "FirstIncludes.h"

#include <errno.h>
#include <iostream>
#include <vector>
#include "MemoryDebug.h"

using namespace std;

#include "GoalKeeper.h"
#include "GlobalGoalKeeper.h"
using namespace  KKU;



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
  delete  globalGoalie;
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

