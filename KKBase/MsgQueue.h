/* MsgQueue.h -- Implements KKStr  message queue meant to work in a multi-threaded environment.
  * Copyright (C) 2011-2014  Kurt Kramer
  * For conditions of distribution and use, see copyright notice in KKB.h
 */
#if  !defined(_KKU_MSGQUEUE_)
#define  _KKU_MSGQUEUE_
#include <queue>
using namespace  std;

#include  "GoalKeeperSimple.h"

namespace KKB
{
  /**
   *@class  MsgQueue
   *@brief  Will manage a buffer that will allow multiple threads to add and remove messages to a queue.
   *@details  A 'GoalKeepeer' object 'gateKeeper' will be used to enforce integrity in the Multi-Threaded
   * environment.  It will guarantee that only one thread at a time can access the Queue.
   */
  class  MsgQueue 
  {
  public:
    typedef  MsgQueue*  MsgQueuePtr;

    MsgQueue (const KKStr&  _name);

    ~MsgQueue ();


    /**
     *@brief Returns an estimate of the amount of memory consumed in bytes by this instance.
     *@details This will help managed objects keep track of how much memory they are using in the 
     * unmanaged world.
     */
    kkint32  MemoryConsumedEstimated ();  

    /**
     *@brief  Take ownership of 'msg' and add to end of the queue.
     *@param[in]  msg  Pointer to message that is to be added to end of queue.  The caller will 
     *            pass ownership of this string to this instance of MsgQueue.
     */
    void  AddMsg (KKStrPtr  msg);     /**<  Taking ownership of 'msg'        */


    /*@brief A copy of the message 'msg' will be added to the end of the queue.  */
    void  AddMsg (const KKStr&  msg);


    /**
     *@brief Adds the contents of 'msgs' to the message queue and depending on 'takeOwnership' will
     * either assume that its owns (taking ownership) the KKStr instances or create duplicates.
     *@details If  'takeOwnership' is true will and the KKStr instances in 'msgs' to the queue 
     * otherwise it will make duplicates of the KKStr instances.  It is up to the caller to make
     * sure that they set the Owner flag on the 'msgs' instance; for example if 'takeOwnership' is 
     * 'true' then caller should set 'msgs.Owner (false);'.
     */
    void  AddMsgs (const KKStrListPtr  msgs,
                   bool                takeOwnership
                  );


    /** 
     *@brief Returns a duplicate of the last string added to the message queue.
     *@details This will not effect the current copy of the message queue.  The returned string
     * will be owned by the caller who will be responsible for deleting it.
     */
    KKStrPtr  GetCopyOfLastMsg ();

    /**
     *@brief  Returns all messages that are currently in the queue.
     */
    KKStrListPtr  GetAllMsgs ();

    /**
     *@brief  Removes from the queue the oldest message added to the queue that has not been removed.
     *@details The caller will get ownership of the string and be responsible for deleting it.
     */
    KKStrPtr  GetNextMsg ();


  private:
    GoalKeeperSimplePtr    gateKeeper;       /**< Used to manage synchronization amongst different threads to this queue. */
    kkint32                memoryConsumed;
    KKStr                  name;             /**< Name of msgQueue. */
    std::queue<KKStrPtr>   queue;
  };  /* MsgQueue */


  typedef  MsgQueue::MsgQueuePtr  MsgQueuePtr;
}  /* KKB */

#endif

