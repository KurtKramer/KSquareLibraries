/* KKObserver.cpp -- Used to manage call-back functions in a multi-threaded environment.
 * Copyright (C) 2012 - 2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */

#if  !defined(_KKOBSERVER_)
#define  _KKOBSERVER_

#include "MsgQueue.h"



namespace KKB
{
  #if  !defined(_KKOBSERVABLE_)
    class  KKObservable;
    typedef  KKObservable*  KKObservablePtr;
  #endif


  /**
   *@brief   The base class to be used by Observer classes.
   *@details  And application would register instances of a Observer
   */
  class  KKObserver
  {
  public:
    typedef  KKObserver*  KKObserverPtr;

    KKObserver (const KKStr&  _name);

    virtual ~KKObserver ();


    kkint32  MemoryConsumedEstimated ();

    virtual  void  Notify (KKObservablePtr  obj);

    const KKStr&   Name    ()  const  {return name;}

  private:

    /** @brief  Called by "KKObserver::RegisterObservable.  */
    void  RegisterObservable (KKObservablePtr observable);

    /** @brief  Called by "KKObserver::RegisterObservable.  */
    void  UnRegisterObservable (KKObservablePtr observable);


    KKStr  name;
    multimap<KKObservablePtr,KKObservablePtr>  observables;  /**< List of Observable instances that we are registered with. */
    multimap<KKObservablePtr,KKObservablePtr>::iterator  observablesIdx;

    friend class  KKObservable;
  };  /* KKObserver */

  typedef  KKObserver::KKObserverPtr  KKObserverPtr;


}  /* KKB*/

#endif
