/* MsgQueue.h -- Implements a Message Queue allowing multiple threads to add and remove messages using KKStr 
 *                   instances without corrupting memory.
 * Copyright (C) 2011-2011  Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#include  "FirstIncludes.h"
#if  defined(WIN32)
#include  <windows.h>
#else
#include  <fcntl.h>
#include  <semaphore.h>
#endif

#include  <errno.h>
#include  <istream>
#include  <iostream>
#include  <queue>
#include  <vector>
using namespace std;


#include "MsgQueue.h"
#include "KKException.h"
#include "GoalKeeper.h"
#include "OSservices.h"
using  namespace  KKU;


MsgQueue::MsgQueue (const KKStr&  _name):        /* Name of buffer, must be unique */
    gateKeeper     (NULL),
    memoryConsumed (0),
    name           (_name),
    queue          ()
{
  gateKeeper = new GoalKeeper (name);
  memoryConsumed = sizeof (MsgQueue) + gateKeeper->MemoryConsumedEstimated ();
}



MsgQueue::~MsgQueue ()
{
  while  (queue.size () > 0)
  {
    KKStrPtr  m = queue.front ();
    queue.pop ();
    delete  m;  
    m = NULL;
  }

  delete  gateKeeper;  gateKeeper = NULL;
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
  memoryConsumed = memoryConsumed + (sizeof (KKStr) + msg->Len ());
  gateKeeper->EndBlock ();
}  /* AddFrame */



void  MsgQueue::AddMsg (const KKStr&  msg)
{
  gateKeeper->StartBlock ();
  queue.push (new KKStr (msg));
  memoryConsumed = memoryConsumed + (sizeof (KKStr) + msg.Len ());
  gateKeeper->EndBlock ();
}  /* AddFrame */



KKStrPtr  MsgQueue::GetNextMsg ()
{
  gateKeeper->StartBlock ();

  KKStrPtr  msg = NULL;
  if  (queue.size () > 0)
  {
    msg = queue.front ();
    queue.pop ();
    memoryConsumed = memoryConsumed - (sizeof (KKStr) + msg->Len ());
  }

  gateKeeper->EndBlock ();
  return  msg;
}  /* GetNextMsg */



int32  MsgQueue::MemoryConsumedEstimated ()
{
  int32  result = 0;
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
