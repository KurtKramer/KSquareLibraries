#if  !defined(_KKTHREADMANAGER_)
#define  _KKTHREADMANAGER_

#include <fstream>
#include <iostream>

#include "MsgQueue.h"
#include "RunLog.h"
using namespace  KKU;



namespace  KKU
{ 
#if !defined(_KKTHREAD_)
  class  KKTHread;
  typedef  KKTHread*  KKTHreadPtr;
#endif


  /**
   *@details
   *
   *@code
   *  There are three ways to stop Thread processing with the first being the most graceful and the last being the most abrupt.
   *  1) 'ShutdownProcessing'  The preferred method;  each thread is flagged for 'Shutdown' after all its prerequisite shutdown threads 
   *     have shutdown.
   *  2) 'TerminateProcessing' Similar to 'ShutdownProcessing' except the prerequisites are not checked; each thread is immediately flagged
   *     to Terminate which indicates to the thread that it is to cease all processing and exit while releasing all resources.
   *  3) 'KillAllRunningThreads' A last resort;  you would call this if for some reason threads are not terminating on there own.
   *@endcode
   */
  class  KKThreadManager:  public  KKThread
  {
  public:
    KKThreadManager (const KKStr&  _managerName,
                     uint32        _maxNumThreads,
                     MsgQueuePtr   _msgQueue
                    );

    ~KKThreadManager ();

    const bool&  ShutdownFlag  ()  const  {return shutdownFlag;}
    const bool&  Crashed       ()  const  {return crashed;}
    const bool&  DoneExecuting ()  const  {return doneExecuting;}

    bool  AllThreadsTerminated ();

    void  AddThread (KKThreadPtr  _thread);  /**< Add thread that is to be managed;  will take ownership */

    bool  AnyProcessorsCrashed ();

    void  KillAllRunningThreads ();  /**< Will STOP ALL running threads;  NOT Gracefull;  */

    void  ManageThreads (bool&  successful);

    void  RequestShutdown ();

    void  RequestTerminate ();

    virtual  void  Run ();

    /** @brief Shutdown all threads in a orderly way; observing prerequisite ordering. */
    void  ShutdownProcessing (int32 miliSecsToWait);

    void  TerminateProcessing ();       /**< Flag all All running threads to Terminate ASAP and release all allocated resources. */

  private:
    void  StartThreads (bool&  successful);

    bool            crashed;            /**< Indicates if any one of the threads crashed. */
    bool            doneExecuting;      /**< The last thing this instance will do in 'ManageTheExtraction'is set this flag to true. */
    uint32          maxNumThreads;
    bool            shutdownFlag;
    bool            shutdownRequested;
    bool            terminateFlag;
    bool            terminateRequested;

    KKThreadListPtr threads;           /**<  List of all threads created under this instance of 'KKThreadManager'.It will own them. */

  };  /* KKThreadManager */

  typedef  KKThreadManager*  KKThreadManagerPtr;


}  /* KKU */


#endif

