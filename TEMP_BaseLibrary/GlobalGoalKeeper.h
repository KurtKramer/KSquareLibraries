/* GlobalGoalKeeper.h -- Implements blocking routines to support thread synchronization.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef  _KKU_GLOBALGOALKEEPER_
#define  _KKU_GLOBALGOALKEEPER_


#include  "Str.h"

namespace  KKU
{
  #if  !defined(_KKU_GOALKEEPER_)
  class  GoalKeeper;
  typedef  GoalKeeper*  GoalKeeperPtr;
  #endif


  /**
   *@brief Maintains one instance of a GoalKeeper object that can be used anywhere in the application.
   *@details  It is meant to be used for simple cases.
   */
  class GlobalGoalKeeper
  {
  public:
    static  void  StartBlock ();
    static  void  EndBlock ();

  private:
    static  void  CreateGlobalGoalie ();
    static  void  CleanUp ();

    static  GoalKeeperPtr  globalGoalie;
  };  /* GlobalGoalKeeper */
}

#endif
