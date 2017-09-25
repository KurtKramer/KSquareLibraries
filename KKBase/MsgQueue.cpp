/* MsgQueue.h -- Implements a Message Queue allowing multiple threads to add and remove messages using KKStr 
 *                   instances without corrupting memory.
 * Copyright (C) 2011-2014  Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include  "FirstIncludes.h"
#if  defined(WIN32)
#include <windows.h>
#else
#include <fcntl.h>
#include <semaphore.h>
#endif

#include <errno.h>
#include <istream>
#include <iostream>
#include <queue>
#include <vector>
using namespace std;


#include "MsgQueue.h"
#include "KKException.h"
#include "GoalKeeperSimple.h"
#include "OSservices.h"
using  namespace  KKB;


MsgQueue::MsgQueue (const KKStr&  _name):        /* Name of buffer, must be unique */
    gateKeeper     (NULL),
    memoryConsumed (0),
    name           (_name),
    queue          ()
{
  GoalKeeperSimple::Create ("MsgQueue_" + name, gateKeeper);
  memoryConsumed = sizeof (MsgQueue) + gateKeeper->MemoryConsumedEstimated ();
}



MsgQueue::~MsgQueue ()
{
  while  (this->queue.size () > 0)
  {
    KKStrPtr  m = this->queue.front ();
    this->queue.pop ();
    delete  m;  
    m = NULL;
  }

  GoalKeeperSimple::Destroy (gateKeeper);  
  gateKeeper = NULL;
}




void  MsgQueue::AddMsg (KKStrPtr  msg)
{
  if  (msg == NULL)
  {
    KKStr  errMsg;
    errMsg << "MsgQueue::AddMsg ==  NULL";
    cerr << std::endl << std::endl << errMsg << std::endl << std::endl;
    throw KKException (errMsg);
  }

  gateKeeper->StartBlock ();
  queue.push (msg);
  memoryConsumed = memoryConsumed +  msg->MemoryConsumedEstimated ();
  gateKeeper->EndBlock ();
}  /* AddFrame */



void  MsgQueue::AddMsg (const KKStr&  msg)
{
  gateKeeper->StartBlock ();
  queue.push (new KKStr (msg));
  memoryConsumed = memoryConsumed + msg.MemoryConsumedEstimated ();
  gateKeeper->EndBlock ();
}  /* AddFrame */



void  MsgQueue::AddMsgs (const KKStrListPtr  msgs,
                         bool                takeOwnership
                        )
{
  gateKeeper->StartBlock ();
  KKStrList::iterator idx;
  for  (idx = msgs->begin ();  idx != msgs->end ();  ++idx)
  {
    KKStrPtr  msg = *idx;
    if  (takeOwnership)
      queue.push (msg);
    else
      queue.push (new KKStr (*msg));

    memoryConsumed = memoryConsumed + msg->MemoryConsumedEstimated ();
  }
  gateKeeper->EndBlock ();
}



KKStrPtr  MsgQueue::GetNextMsg ()
{
  gateKeeper->StartBlock ();

  KKStrPtr  msg = NULL;
  if  (queue.size () > 0)
  {
    msg = queue.front ();
    queue.pop ();
    memoryConsumed = memoryConsumed - msg->MemoryConsumedEstimated ();
  }

  gateKeeper->EndBlock ();
  return  msg;
}  /* GetNextMsg */



KKStrListPtr  MsgQueue::GetAllMsgs ()
{
  KKStrListPtr  result = new KKStrList ();
  gateKeeper->StartBlock ();

  KKStrPtr  msg = NULL;
  while  (queue.size () > 0)
  {
    msg = queue.front ();
    queue.pop ();
    result->PushOnBack (msg);
    memoryConsumed = memoryConsumed - msg->MemoryConsumedEstimated ();
  }

  gateKeeper->EndBlock ();

  return  result;
}  /* GetAllMsgs */



kkMemSize  MsgQueue::MemoryConsumedEstimated ()  const
{
  kkMemSize  result = 0;
  gateKeeper->StartBlock ();
  result = memoryConsumed;
  gateKeeper->EndBlock ();
  return  result;
}  /* MemoryConsumedEstimated */



KKStrPtr  MsgQueue::GetCopyOfLastMsg ()
{
  KKStrPtr  msg = NULL;
  gateKeeper->StartBlock ();
  if  (queue.size () > 0)
  {
    msg = new KKStr (*(queue.back ()));
  }
  gateKeeper->EndBlock ();
  return  msg;
}
