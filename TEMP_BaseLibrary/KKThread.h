#if  !defined(_KKTHREAD_)
#define  _KKTHREAD_

#include "MsgQueue.h"



namespace KKU 
{
  #if  !defined(_KKTHREADMANAGER_)
  class  KKThreadManager;
  typedef  KKThreadManager*  KKThreadManagerPtr;
  #endif

  class  KKThreadList;
  typedef  KKThreadList*  KKThreadListPtr;


  /**
   *@brief   The base class to be used any thread.
   *@details  This will be the base class for each one of the different types of processing.
   */
  class  KKThread
  {
  public:
    typedef  KKThread*  KKThreadPtr;

    typedef  enum  {tsNotStarted,
                    tsStarting,
                    tsRunning,
                    tsStopping,
                    tsStopped
                   } 
                     ThreadStatus;


    KKThread (const KKStr&        _threadName,
              KKThreadManagerPtr  _threadManager,
              MsgQueuePtr         _msgQueue
             );

    virtual ~KKThread ();








    volatile const bool&   CancelFlag    ()  const  {return terminateFlag;}
    bool                   Crashed       ()  const  {return crashed;}
    ThreadStatus           Status        ()  const  {return status;}
    volatile const bool&   ShutdownFlag  ()  const  {return shutdownFlag;}
    bool                   TerminateFlag ()  const  {return terminateFlag;}
    const KKStr&           ThreadName    ()  const  {return threadName;}

    void    Crashed (bool         _crashed)  {crashed = _crashed;}
    void    Status  (ThreadStatus _status)   {status  = _status;}

    virtual  void  Run ();
    
    /**
     *@brief  Specify threads that must start before this thread is started.
     *@details This method can be called multiple times;  the information is used by the 'KKTHreadManager' instance to control the
     *  start of the thread.
     *@param[in]  _thread  A thread that needs to be in the the 'tsRunning' status before this thread can start;  we do NOT take ownership.
     */
    void  AddStartPrerequistite (KKThreadPtr  _thread);

    /**
     *@brief  Specify threads that must stop before this thread is started.
     *@details This method can be called multiple times;  the information is used by the 'KKTHreadManager' instance to control the
     *  orderly shutdown of the controlling 'KKTHreadManager' instance.
     *@param[in]  _thread  A thread that needs to be in the the 'tsStopped' status before this thread can be shutdown;  we do NOT take ownership.
     */
    void  AddShutdownPrerequistite (KKThreadPtr  _thread);

    void  Kill ();

    bool  OkToShutdown ()  const;     /**< Returns 'true' if all shutdown prerequisites are in the 'tsStopped' status. */

    bool  OkToStart ()  const;        /**< Returns 'true' if all start prerequisites are in the 'tsRunning'   status. */

    void  Start (bool&  successful);  /**<  Call this method to start thread; it will call the method "Run". */

    void  TerminateThread ();         /**< Call this method to have its thread stop right away and exit. */

    void  ShutdownThread ();          /**< Call this method to have its thread finish what ever is in the queue and then exit. */


  protected:
    void  AddMsg (KKStrPtr  msg);      /**<  Taking ownership of 'msg' and will append to 'msgQueue'.           */
    void  AddMsg (const KKStr&  msg);  /**<  A copy of the message 'msg' will be added to the end of msgQueue.  */

    bool  ShutdownOrTerminateRequested ()  {return  (terminateFlag || shutdownFlag);}

    bool  ThereIsACircularReferenceStart (KKThreadPtr  _thread)  const;

    bool  ThereIsACircularReferenceShutdown (KKThreadPtr  _thread)  const;

  private:


    void  SetThreadName ();


    bool                   crashed;            /**< Signifies that this thread had to terminate on its own because 
                                                * of an abnormal situation. 
                                                */

    MsgQueuePtr            msgQueue;           /**< This MsgQueue instance will be owned by 'ExtractionManager' we will just use
                                                * it to communicate messages to the controlling process.
                                                */

    volatile bool          shutdownFlag;       /**< Threads need to monitor this flag; if it goes true they are to
                                                * complete all processing that is queued up and then terminate.
                                                */

    KKThreadListPtr        startPrerequisites;

    KKThreadListPtr        shutdownPrerequisites;

    ThreadStatus           status;

    volatile bool          terminateFlag;      /**< Threads need to monitor this flag;  if it goes 'true'  they need
                                                * to terminate as quick as possible releasing all allocated resources.
                                                */

    KKThreadManagerPtr     threadManager;

    KKStr                  threadName;

#if  defined(WIN32)
    //  Data structures and methods specific to Windows thread management.
    unsigned long*         windowsThread;
    DWORD                  windowsThreadId;
#else
    pthread_t              threadId;
#endif

  };  /* KKThread */



  typedef  KKThread::KKThreadPtr  KKThreadPtr;

  class  KKThreadList:  public  KKQueue<KKThread>
  {
  public:
    KKThreadList (bool _owner = true);
    KKThreadList (const KKThreadList&  list);
    ~KKThreadList ();
  };

  typedef  KKThreadList*    KKThreadListPtr;

#if  defined(WIN32)
  // global function called by the thread object.
  // this in turn calls the overridden run()
  extern "C" 
  {
	  unsigned int ThreadStartCallBack(void* param);
  }
#endif


}  /* ImageExtractionManager*/

#endif
