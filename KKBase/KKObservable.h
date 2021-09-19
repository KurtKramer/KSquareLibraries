/* KKObservable.cpp -- Used to manage call-back functions in a multi-threaded environment.
 * Copyright (C) 2012 - 2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#if  !defined(_KKOBSERVABLE_)
#define  _KKOBSERVABLE_

WarningsLowered
#include <map>
WarningsRestored

#include "MsgQueue.h"



namespace KKB
{
  #if  !defined(_KKOBSERVER_)
    class  KKObserver;
    typedef  KKObserver*  KKObserverPtr;
  #endif

    
  /**
   *@brief   The base class to be used by Observer classes.
   */
  class  KKObservable
  {
  public:
    typedef  KKObservable*  KKObservablePtr;

    KKObservable ();

    virtual ~KKObservable ();

    virtual size_t  MemoryConsumedEstimated () const;

    virtual  void  RegisterObserver (KKObserverPtr observer);
    
    virtual  void  UnRegisterObserver (KKObserverPtr observer);

    virtual  void  NotifyObservers ();

  private:
    std::multimap<KKObserverPtr,KKObserverPtr>            observers;
    std::multimap<KKObserverPtr,KKObserverPtr>::iterator  observersIdx;

    friend class  KKObserver;
  };  /* KKObservable */

  typedef  KKObservable::KKObservablePtr  KKObservablePtr;

}  /* KKB*/

#endif
