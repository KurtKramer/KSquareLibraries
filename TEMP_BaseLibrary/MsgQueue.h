/* MsgQueue.h -- Implements Camera Frame Buffering allowing a camera thread to unload frames quickly for a separate thread to process.
 * Copyright (C) 2011-2011  Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKLineScanner.h
 */
#if  !defined(_KKU_MSGQUEUE_)
#define  _KKU_MSGQUEUE_
#include  <queue>

#include  "GoalKeeper.h"

namespace KKU
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
    int32  MemoryConsumedEstimated ();  

    /**
     *@brief  Take ownership of 'msg' and add to end of the queue.
     *@param[in]  msg  Pointer to message that is to be added to end of queue. The caller will
     *            pass ownership of this string to this instance of MsgQueue.
     */
    void  AddMsg (KKStrPtr  msg);     /**<  Taking ownership of 'msg'        */

    /*@brief A copy of the message 'msg' will be added to the end of the queue.  */
    void  AddMsg (const KKStr&  msg);

    /** 
     *@brief Returns a duplicate of the last string added to the message queue.
     *@details This will not effect the current copy of the message queue. The returned string
     * will be owned by the caller who will be responsible for deleting it.
     */
    KKStrPtr  GetCopyOfLastMsg ();

    /**
     *@brief  Removes from the queue the oldest message added to the queue that has not been removed.
     *@details The caller will get ownership of the string and be responsible for deleting it.
     */
    KKStrPtr  GetNextMsg ();


  private:
    GoalKeeperPtr          gateKeeper;       /**< Used to manage synchronization amongst different threads to this queue. */
    int32                  memoryConsumed;
    KKStr                  name;             /**< Name of msgQueue. */
    std::queue<KKStrPtr>   queue;
  };  /* MsgQueue */


  typedef  MsgQueue::MsgQueuePtr  MsgQueuePtr;
}  /* KKU */

#endif

