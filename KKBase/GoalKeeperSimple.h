/* GoalKeeperSimple.h -- Implements blocking routines to support thread synchronization.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#if  !defined(_KKU_GOALKEEPERSIMPLE_)
#define  _KKU_GOALKEEPERSIMPLE_

#define  GOALKEEPER_DEBUG


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
  class  GoalKeeperSimpleList;
  typedef  GoalKeeperSimpleList*  GoalKeeperSimpleListPtr;

  /**
   * @brief  A simple/ light-weight implementation of critical section blocking.
   */
  class GoalKeeperSimple
  {
  public:
    typedef  GoalKeeperSimple*  GoalKeeperSimplePtr;

  private:
    /**  @brief Constructs a GoalKeeperSimple object; best to do this via the GoalKeeperSimple::Create method. */
    GoalKeeperSimple (const KKStr&  _name);
   
    ~GoalKeeperSimple ();

  public:
    friend class KKQueue<GoalKeeperSimple>;


    /**
    *@brief Create a GoalKeeperSimple object and avoid a race condition doing it.  
    *@details In case two different threads try to create the same GoalKeeperSimple at the same time you only 
    *         want one of them to succeed and the other to use same GaolKeeper object.
    *@param[in]     _name          Name of Goal Keeper object that you want to create.
    *@param[in,out] _newGoalKeeper You pass this in. Create will block out the critical region that 
    *                              creates the GoalKeeperSimple object.  If it was already created, that 
    *                              is != NULL will just return not changing its value.  If it is 
    *                              still NULL in the critical section it will create a new instance 
    *                              and set this parameter to its address.
    */
    static  void  Create (const KKStr&                   _name,
                          volatile GoalKeeperSimplePtr&  _newGoalKeeper
                         );  


    /**
    *@brief Create a GoalKeeperSimple object and avoid a race condition doing it.
    *@details Similar to 'Create' except it will also call the StartBlock method. There is also 
    *         an additional parameter that will let you know if your call was responsible for
    *         creating it. 
    *
    *         In case two different threads try to create the same GoalKeeperSimple at the same time 
    *         you only want one of them to succeed and the other to use same GaolKeeper object.
    *@param[in]     _name          Name of Goal Keeper object that you want to create.
    *
    *@param[in,out] _newGoalKeeper You pass this in. Create will block out the critical region that 
    *                              creates the GoalKeeperSimple object.  If it was already created, that 
    *                              is != NULL will just return not changing its value.  If it is 
    *                              still NULL in the critical section it will create a new instance 
    *                              and set this parameter to its address.
    *
    *@param[out]    _didNotExistYet Indicates if this call had to create the GoalKeeperSimple instance; if it 
    *                              already existed will return as false.
    */
    static  void  CreateAndStartBlock (const KKStr&             _name,
                                       volatile GoalKeeperSimplePtr&  _newGoalKeeper,
                                       bool&                    _didNotExistYet
                                      );

    /**
     *@brief  Destroys an existing insdtance of GoalKeeperSimple.
     *@details  Use this method rather tna calling the destructor directly.  This way the 
     * 'existingGoalKeepers' data member can be kepped up to date.  If for some reason two 
     * different threads managed to call this method for the same GoalKeeperSimple instance only 
     * one of them will actually detroy the instance.
     *@param[in,out]  _goalKeeperInstance  Instance of GoalKeeperSimple that is to be destroyed.  Upon return
     * it will be set to NULL.
     */
    static  void  Destroy (volatile GoalKeeperSimplePtr&  _goalKeeperInstance);

    
    /**
     *@brief  Will return true if any thread lock on this instance of "GoalKeeperSimple".
     */
    bool   Blocked ();


    /**
     *@brief  Returns true if a different thread has this instance of "GoalKeeperSimple" locked.
     *@details  GoalKeeperSimple keeps track of which thread has a lock on this instance of 'GoalKeeperSimple'.  
     *  This way we know if the calling thread is not the one to have a lock on the thread.
     */
    bool   BlockedByAnotherThread ();
  

    int32  BlockerThreadId ();  /**< @brief  ThreadId of thread that currently holds the Block  -1 indicates no Block */

    /**
     *@brief Ends the block and allows other threads to pass through StatBlock.
     *@details Decrements the variable 'blockerDepth' by one.  Once 'blockerDepth' is equal zero
     *  the Block on this instance is removed.  The idea is that for each time in a row a Thread
     *  calls StartBlock it has to call EndBlock the same number of times.
     */
    void   EndBlock ();

    int32  MemoryConsumedEstimated ()  const;

    const  KKStr&  Name ()  const  {return  name;}


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

    static  volatile  GoalKeeperSimpleListPtr  existingGoalKeepers;


    volatile bool   blocked;    /**< @brief 'true' = Currently Blocked.  */

    volatile int32  blockerThreadId;  /**< @brief ThreadId of thread that currently holds the Block  -1 indicates no Block */

    volatile uint32 levels;  /**< Number of levels deep that a thread blocked; this is the number of times "EndBlock" must be called 
                              * to release this block.
                              */

    KKStr           name;


#if defined(WIN32)
    CRITICAL_SECTION cs;
#else 
    pthread_mutex_t  mutex;
#endif

  };  /* GoalKeeperSimple */

  typedef  GoalKeeperSimple::GoalKeeperSimplePtr  GoalKeeperSimplePtr;



  class  GoalKeeperSimpleList:  public  KKQueue<GoalKeeperSimple>
  {
  public:  
    GoalKeeperSimpleList (bool _owner): KKQueue<GoalKeeperSimple> (_owner)  {}

    ~GoalKeeperSimpleList ()
    {}
  };


}

#endif
