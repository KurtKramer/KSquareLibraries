/* RasterBuffer.h -- Implements RasterBuffering allowing multiple threads to add and remove Raster 
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

#include <errno.h>
#include <stdlib.h>
#include <istream>
#include <iostream>
#include <queue>
#include <vector>
using namespace std;


#include "BasicTypes.h"
#include "GoalKeeper.h"
#include "KKException.h"
#include "OSservices.h"
#include "RasterBuffer.h"
#include "Raster.h"
using  namespace  KKU;



RasterBuffer::RasterBuffer (const KKStr&  _name,        /* Name of buffer, must be unique */
                            int32         _maxNumOfBuffers
                           ):
  
    buffer               (),
    gateKeeper           (NULL),
    maxNumOfBuffers      (_maxNumOfBuffers),
    memoryConsumed       (0),
    name                 (_name),
    rastersDropped       (0)
{
  gateKeeper = new GoalKeeper (name);
  memoryConsumed = sizeof (RasterBuffer) + gateKeeper->MemoryConsumedEstimated ();
}



RasterBuffer::~RasterBuffer ()
{
  while  (buffer.size () > 0)
  {
    RasterPtr r = buffer.front ();
    buffer.pop ();
    delete  r;  
    r = NULL;
  }

  delete  gateKeeper;  gateKeeper = NULL;
}



void  RasterBuffer::ThrowOutOldestOccupiedBuffer ()
{
  gateKeeper->StartBlock ();

  if  (buffer.size () > 0)
  {
    RasterPtr r = buffer.front ();
    buffer.pop ();
    memoryConsumed = memoryConsumed - r->MemoryConsumedEstimated ();
    delete  r;  
    r = NULL;
    rastersDropped++;
  }

  gateKeeper->EndBlock ();
  return;
}  /* ThrowOutOldestOccupiedBuffer */




int32  RasterBuffer::NumAvailable () const 
{
  return  maxNumOfBuffers - buffer.size ();
}


int32  RasterBuffer::NumPopulated () const
{
  return  buffer.size ();
}


void  RasterBuffer::AddRaster (RasterPtr  raster)
{
  if  (raster == NULL)
  {
    KKStr  errMsg;
    errMsg << "RasterBuffer::AddRaster    raster ==  NULL";
    cerr << std::endl << std::endl << errMsg << std::endl << std::endl;
    throw KKException (errMsg);
  }

  gateKeeper->StartBlock ();

  while  (buffer.size () >= (uint32)maxNumOfBuffers)
    ThrowOutOldestOccupiedBuffer ();

  buffer.push (raster);
  memoryConsumed = memoryConsumed + raster->MemoryConsumedEstimated ();

  gateKeeper->EndBlock ();
}  /* AddFrame */



RasterPtr  RasterBuffer::GetNextRaster ()
{
  RasterPtr  result = NULL;

  gateKeeper->StartBlock ();

  if  (buffer.size () > 0)
  {
    result = buffer.front ();
    buffer.pop ();
    memoryConsumed = memoryConsumed - result->MemoryConsumedEstimated ();
  }

  gateKeeper->EndBlock ();
  return result;
}  /* GetNextRaster */



int32  RasterBuffer::MemoryConsumedEstimated ()
{
  int32  result = 0;
  gateKeeper->StartBlock ();
  result = memoryConsumed;
  gateKeeper->EndBlock ();
  return  result;
}  /* MemoryConsumedEstimated */



RasterPtr  RasterBuffer::GetCopyOfLastImage ()
{
  RasterPtr  result = NULL;
  gateKeeper->StartBlock ();
  if  (buffer.size () > 0)
  {
    result = new Raster (*(buffer.back ()));
  }
  gateKeeper->EndBlock ();
  return  result;
}
