/* KKThread.cpp -- Manages the threads that perform the image extraction process.
 * Copyright (C) 2012-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#if  !defined(_KKTHREAD_)
#define  _KKTHREAD_

#include "MsgQueue.h"



namespace KKB
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

    enum  class  ThreadPriority: int 
                   {Null,
                    Low,
                    Normal,
                    High
                   };

    enum  class  ThreadStatus: int
                   {Null,
                    NotStarted,
                    Starting,
                    Running,
                    Stopping,
                    Stopped
                   };


    KKThread (const KKStr&        _threadName,
              KKThreadManagerPtr  _threadManager,
              MsgQueuePtr         _msgQueue
             );

    virtual ~KKThread ();

    kkint32  MemoryConsumedEstimated ();

    static  const KKStr&     ThreadStatusToStr (ThreadStatus);

    VolConstBool&            CancelFlag    ()  const  {return terminateFlag;}    /**<  Another name for 'TerminateFlag'. */

    bool                     Crashed       ()  const  {return crashed;}          /**< Signifies that this thread had to terminate on its own because of an abnormal 
                                                                                  * situation; such as a memory corruption. 
                                                                                  */

    const KKStr&             ExceptionText ()  const  {return exceptionText;}   /**<  If the overloaded 'Run' method of a derived class generates a exception that 
                                                                                 * is not caught; then the ''ThreadStartCallBack' will catch it and store the related
                                                                                 * text in this field.  In this case the 'Crashed()' method will return 'true'.
                                                                                 */

    KKB::MsgQueuePtr         MsgQueue      ()         {return msgQueue;}
    ThreadStatus             Status        ()  const  {return status;}
    const KKStr&             StatusStr     ()  const  {return ThreadStatusToStr (status);}

    VolConstBool&            ShutdownFlag  ()  const  {return shutdownFlag;}     /**< Indicates that the application wants this thread to complete what work is queued up 
                                                                                   * for it top process. Threads need to monitor this flag; if it goes true they are to 
                                                                                   * complete all processing that is queued up and then shutdown. This is 
                                                                                   */

    VolConstBool&            TerminateFlag ()  const  {return terminateFlag;}    /**< Indicates that thread is to stop processing ASAP, release any resources 
                                                                                  * it holds and terminate. Threads need to monitor this flag; if it goes 'true'
                                                                                  * they need to terminate ASAP releasing all allocated resources. Typically set 
                                                                                  * to true when user request to cancel processing.
                                                                                  */

    kkint32                  ThreadId      ()  const  {return threadId;}
    const KKStr&             ThreadName    ()  const  {return threadName;}

    void    Crashed       (bool          _crashed)        {crashed       = _crashed;}
    void    ExceptionText (const KKStr&  _exceptionText)  {exceptionText = _exceptionText;}
    void    Status        (ThreadStatus  _status)         {status        = _status;}

    KKStrListPtr  GetMsgs ();

    virtual  void  Run ();
    
    /**
     *@brief  Specify threads that must start before this thread is started.
     *@details This method can be called multiple times;  the information is used by the 'KKTHreadManager' instance to control the
     *  start of the thread.
     *@param[in]  _thread  A thread that needs to be in the 'Running' status before this thread can start;  we do NOT take ownership.
     */
    void  AddStartPrerequistite (KKThreadPtr  _thread);

    /**
     *@brief  Specify threads that must stop before this thread is started.
     *@details This method can be called multiple times;  the information is used by the 'KKTHreadManager' instance to control the
     *  orderly shutdown of the controlling 'KKTHreadManager' instance.
     *@param[in]  _thread  A thread that needs to be in the 'Stopped' status before this thread can be shutdown;  we do NOT take ownership.
     */
    void  AddShutdownPrerequistite (KKThreadPtr  _thread);

    void  Kill ();

    bool  OkToShutdown ()  const;     /**< Returns 'true' if all shutdown prerequisites are in the 'Stopped' status. */

    bool  OkToStart ()  const;        /**< Returns 'true' if all start prerequisites are in the 'Running'   status. */

    bool  ThreadStillProcessing ()  const;

    void  Start (ThreadPriority _priority,
                 bool&          successful
                );                    /**<  Call this method to start thread; it will call the method "Run". */

    void  TerminateThread ();         /**< Call this method to have its thread stop right away and exit. */

    /**
     *@brief  Will be called whenever the value of 'terminateFlag' is changed; derived classes should override this method if
     * they need to be aware that the terminaeFlag has changed.  This give them a chance to let other objects/methods know that
     * the flag has changed.
     */
    virtual  void  TerminateFlagChanged ();

    void  ShutdownThread ();          /**< Call this method to have its thread finish what ever is in the queue and then exit. */

    /**
     *@brief  Called by separate thread; will stay in loop until the thread controlled by this instance shutdown or it waited the 'maxTimeToWait'.
     *@details  Does not call the 'ShutdownThread' or  'TerminateThread'  methods.
     *@param[in]  maxTimeToWait  Will wait 'maxTimeToWait' seconds before giving up on waiting at which point will try to kill thread.
     */
    void  WaitForThreadToStop (kkuint32  maxTimeToWait);


  protected:
    void  AddMsg (KKStrPtr  msg);      /**<  Taking ownership of 'msg' and will append to 'msgQueue'.           */
    void  AddMsg (const KKStr&  msg);  /**<  A copy of the message 'msg' will be added to the end of msgQueue.  */

    bool  ShutdownOrTerminateRequested ()  {return  (terminateFlag || shutdownFlag);}

    bool  ThereIsACircularReferenceStart (KKThreadPtr  _thread)  const;

    bool  ThereIsACircularReferenceShutdown (KKThreadPtr  _thread)  const;

  private:
    static  KKStr  threadStatusDescs[];

    void  SetThreadName ();


    bool                   crashed;            /**< Signifies that this thread had to terminate on its own because 
                                                * of an abnormal situation. 
                                                */

    KKStr                  exceptionText;      /**< If the 'ThreadStartCallBack' method catches a exception; the text of the exception 
                                                * will be stored hear.
                                                */

    MsgQueuePtr            msgQueue;           /**< This MsgQueue instance will be owned by 'ExtractionManager' we will just use
                                                * it to communicate messages to the controlling process.
                                                */

    volatile bool          shutdownFlag;       /**< Indicates that the application wants this thread to complete what work is queued up 
                                                * for it top process. Threads need to monitor this flag; if it goes true they are to 
                                                * complete all processing that is queued up and then shutdown. This is 
                                                */

    KKThreadListPtr        shutdownPrerequisites;

    KKThreadListPtr        startPrerequisites;

    ThreadStatus           status;

    ThreadPriority         priority;

    volatile bool          terminateFlag;      /**< Indicates that thread is to stop processing ASAP, release any resources 
                                                * it holds and terminate. Threads need to monitor this flag; if it goes 'true'
                                                * they need to terminate ASAP releasing all allocated resources. Typically set 
                                                * to true when user request to cancel processing.
                                                */

    kkint32                threadId;

    KKThreadManagerPtr     threadManager;

    KKStr                  threadName;

#if  defined(WIN32)
    //  Data structures and methods specific to Windows thread management.
    unsigned long*         windowsThreadHandle;
    DWORD                  windowsThreadId;
#else
    pthread_t              linuxThreadId;
#endif

  };  /* KKThread */



  typedef  KKThread::KKThreadPtr  KKThreadPtr;

  class  KKThreadList:  public  KKQueue<KKThread>
  {
  public:
    KKThreadList (bool _owner = true);
    KKThreadList (const KKThreadList&  list);
    ~KKThreadList ();

    kkint32  MemoryConsumedEstimated ()  const;
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
