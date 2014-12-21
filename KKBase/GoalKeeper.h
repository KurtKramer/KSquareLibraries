/* GoalKeeper.h -- Implements blocking routines to support thread synchronization.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#if  !defined(_KKU_GOALKEEPER_)
#define  _KKU_GOALKEEPER_

//#define  GOALKEEPER_DEBUG


#if  defined(WIN32)
#include  <windows.h>
#else
#include  <fcntl.h>
#include  <semaphore.h>
#endif

#include "DateTime.h"
#include "KKStr.h"

namespace  KKB
{
  class  GoalKeeperList;
  typedef  GoalKeeperList*  GoalKeeperListPtr;

  /**
   * @brief Used to deal with issues related to thread synchronization issues when sharing the same variables.  
   * @details Use the GoalKeeper::Create method to create an instance of this class.  The Create method will ensure 
   * that two different threads don't accidently create two different instances of the same GoalKeeper object.<p>
   *
   * GoalKeeper has a concept of levels of depth. That is a particular thread may call 'StartBlock' more than once, 
   * each time doing so it would be one level deeper.  In this case it would have to call 'EndBlock' an equal number
   * of times to actually release the block.  Imagine that you have a function that Starts and Ends a Block but can
   * be called from the middle of another function that also Starts and Ends a block on the same GoalKeeper object.<p>
   *
   * This class is meant to function the same under Windows or Linux
   */
  class GoalKeeper
  {
  public:
    typedef  GoalKeeper*  GoalKeeperPtr;

  private:
    /**  @brief Constructs a GoalKeeper object; best to do this via the GoalKeeper::Create method. */
    GoalKeeper (const KKStr&  _name);
   
    ~GoalKeeper ();

  public:
    friend class KKQueue<GoalKeeper>;


    /**
    *@brief Create a GoalKeeper object and avoid a race condition doing it.  
    *@details In case two different threads try to create the same GoalKeeper at the same time you only 
    *         want one of them to succeed and the other to use same GaolKeeper object.
    *@param[in]     _name          Name of Goal Keeper object that you want to create.
    *@param[in,out] _newGoalKeeper You pass this in. Create will block out the critical region that 
    *                              creates the GoalKeeper object.  If it was already created, that 
    *                              is != NULL will just return not changing its value.  If it is 
    *                              still NULL in the critical section it will create a new instance 
    *                              and set this parameter to its address.
    */
    static  void  Create (const KKStr&             _name,
                          volatile GoalKeeperPtr&  _newGoalKeeper
                         );  


    /**
    *@brief Create a GoalKeeper object and avoid a race condition doing it.
    *@details Similar to 'Create' except it will also call the StartBlock method. There is also 
    *         an additional parameter that will let you know if your call was responsible for
    *         creating it. 
    *
    *         In case two different threads try to create the same GoalKeeper at the same time 
    *         you only want one of them to succeed and the other to use same GaolKeeper object.
    *@param[in]     _name          Name of Goal Keeper object that you want to create.
    *
    *@param[in,out] _newGoalKeeper You pass this in. Create will block out the critical region that 
    *                              creates the GoalKeeper object.  If it was already created, that 
    *                              is != NULL will just return not changing its value.  If it is 
    *                              still NULL in the critical section it will create a new instance 
    *                              and set this parameter to its address.
    *
    *@param[out]    _didNotExistYet Indicates if this call had to create the GoalKeeper instance; if it 
    *                              already existed will return as false.
    */
    static  void  CreateAndStartBlock (const KKStr&             _name,
                                       volatile GoalKeeperPtr&  _newGoalKeeper,
                                       bool&                    _didNotExistYet
                                      );

    /**
     *@brief  Destroys an existing instance of GoalKeeper.
     *@details  Use this method rather than calling the destructor directly.  This way the 
     * 'existingGoalKeepers' data member can be kepped up to date.  If for some reason two 
     * different threads managed to call this method for the same GoalKeeper instance only 
     * one of them will actually detroy the instance.
     *@param[in,out]  _goalKeeperInstance  Instance of GoalKeeper that is to be destroyed.  Upon return
     * it will be set to NULL.
     */
    static  void  Destroy (volatile GoalKeeperPtr&  _goalKeeperInstance);

    
    /**
     *@brief  Will return true if any thread lock on this instance of "GoalKeeper".
     */
    bool   Blocked ();


    /**
     *@brief  Returns true if a different thread has this instance of "GoalKeeper" locked.
     *@details  GoalKeeper keeps track of which thread has a lock on this instance of 'GoalKeeper'.  
     *  This way we know if the calling thread is not the one to have a lock on the thread.
     */
    bool   BlockedByAnotherThread ();
  

    kkint32  BlockerThreadId ();  /**< @brief  ThreadId of thread that currently holds the Block  -1 indicates no Block */

    /**
     *@brief Ends the block and allows other threads to pass through StatBlock.
     *@details Decrements the variable 'blockerDepth' by one.  Once 'blockerDepth' is equal zero
     *  the Block on this instance is removed.  The idea is that for each time in a row a Thread
     *  calls StartBlock it has to call EndBlock the same number of times.
     */
    void   EndBlock ();

    kkint32  MemoryConsumedEstimated ()  const;

    const  KKStr&  Name ()  const  {return  name;}


    /** @brief  Returns the number of threads that are waiting to establish a lock on this instance. */
    kkint32  NumBlockedThreads ();


    /**
     *@brief Initiates a Block as long as another thread has not already locked this object.  
     *@details If it is already blocked processor will sleep and then try again. As long as the variable 
     *  'blockerDepth'is greater than zero this instance will be considered blocked.  Once a thread has the 
     *  instance blocked it will increment 'blockerDepth' and return to caller.
     */
    void   StartBlock ();

    /**
     *@brief  Will be registered with 'atexit' so that it will be called when program is unloaded from memory
     */
    static  void   FinalCleanUp ();

  private:
    void     CriticalSectionStart ();
    
    void     CriticalSectionEnd ();


    volatile bool  blocked;    /**< @brief 'true' = Currently Blocked.  */

    kkint32  blockerDepth;     /**< @brief Indicates how many times the thread that currently holds the block has
                                * called  "StartBlock".  For every time the thread that holds the Block calls 
                                * "StartBlock"  it will have to call "EndBlock"  before the block is actually
                                * released.
                                */

    kkint32  blockerThreadId;  /**< @brief ThreadId of thread that currently holds the Block  -1 indicates no Block */

    KKStr    name;

    kkint32  numBlockedThreads; /**< @brief  The number of threads waiting in 'StartBlock'  for the current block to end */

    static  GoalKeeperListPtr  existingGoalKeepers;

#if  defined(GOALKEEPER_DEBUG)
    class  BlockedStat;
    typedef  BlockedStat*  BlockedStatPtr;
    class  BlockedStatList;
    typedef  BlockedStatList*      BlockedStatListPtr;

    BlockedStatListPtr  blockedStats;
    void  ReportBlockedStats ();
#endif

#if defined(WIN32)
    CRITICAL_SECTION cs;
#else 
    pthread_mutex_t  mutex;
#endif

  };  /* GoalKeeper */

  typedef  GoalKeeper::GoalKeeperPtr  GoalKeeperPtr;



  class  GoalKeeperList:  public  KKQueue<GoalKeeper>
  {
  public:  
    GoalKeeperList (bool _owner): KKQueue<GoalKeeper> (_owner)  {}

    ~GoalKeeperList ()
    {}
  };


}

#endif
