/* Compressor.cpp -- Compresses and de-compresses data using zLib Library
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#include "FirstIncludes.h"
#include <iostream>
#include <fstream>
#include <vector>
#include "MemoryDebug.h"
using namespace std;

#ifdef  WIN32
#include "zlib.h"
#endif

#include "Compressor.h"
#include "GlobalGoalKeeper.h"
#include "OSservices.h"
using namespace KKU;

void*  Compressor::CreateCompressedBuffer (void*  source,
                                           uint32 sourceLen,
                                           uint32&  compressedBuffLen
                                          )
{
#ifdef ZLIB_H

  // following code was lifted from example provided by zlib  "zpipe.c"
  int32     ret;
  z_stream  strm;


  Bytef*  outputBuffer = NULL;
  uint32  outputBufferSize = sourceLen * 2;

  outputBuffer = new Bytef[outputBufferSize];

  uchar*  compressedBuff = NULL;
  compressedBuffLen     = 0;

  // The first thing we do is to initialize the zlib state for compression using deflateInit(). This must be done before the first use
  // of deflate(). The zalloc, zfree, and opaque fields in the strm structure must be initialized before calling deflateInit(). Here
  // they are set to the zlib constant Z_NULL to request that zlib use the default memory allocation routines. An application may also
  // choose to provide custom memory allocation routines here. deflateInit() will allocate on the order of 256K bytes for the internal
  // state. (See zlib Technical Details.)
  //
  // deflateInit() is called with a pointer to the structure to be initialized and the compression level, which is an integer in the
  // range of -1 to 9. Lower compression levels result in faster execution, but less compression. Higher levels result in greater
  // compression, but slower execution. The zlib constant Z_DEFAULT_COMPRESSION, equal to -1, provides a good compromise between
  // compression and speed and is equivalent to level 6. Level 0 actually does no compression at all, and in fact expands the data
  // slightly to produce the zlib format (it is not a byte-for-byte copy of the input). More advanced applications of zlib may use
  // deflateInit2() here instead. Such an application may want to reduce how much memory will be used, at some price in compression.
  // Or it may need to request a gzip header and trailer instead of a zlib header and trailer, or raw encoding with no header or
  // trailer at all.

  // We must check the return value of deflateInit() against the zlib constant Z_OK to make sure that it was able to allocate memory
  // for the internal state, and that the provided arguments were valid. deflateInit() will also check that the version of zlib that
  // the zlib.h file came from matches the version of zlib actually linked with the program. This is especially important for environments
  // in which zlib is a shared library.

  // Note that an application can initialize multiple, independent zlib streams, which can operate in parallel. The state information
  // maintained in the structure allows the zlib routines to be reentrant.


  /* allocate deflate state */
  strm.zalloc  = Z_NULL;
  strm.zfree   = Z_NULL;
  strm.opaque  = Z_NULL;
  ret = deflateInit (&strm, Z_DEFAULT_COMPRESSION);   // 'Z_DEFAULT_COMPRESSION' will balance between speed and compression;  0 = no compression,  9 = best compression
  if  (ret != Z_OK)
  {
    // Initialization Failed
    compressedBuffLen = 0;
    return NULL;
  }

  //  With the pleasantries out of the way, now we can get down to business. The outer do-loop reads all of the input file and exits
  // at the bottom of the loop once end-of-file is reached. This loop contains the only call of deflate(). So we must make sure that
  // all of the input data has been processed and that all of the output data has been generated and consumed before we fall out of
  // the loop at the bottom.

  {
    // This was originally a a loop that read from a file;  but now we have a input buffer with a known amount of data.
    strm.avail_in = sourceLen;       // Number of bytes in 'source'  to compress.
    strm.next_in  = (Bytef*)source;  // pointer to data that needs to be compressed.
    
    do
    { 
      // We will stay in this loop until all data in source is processed.  If the output buffer is not large enough 
      // we may need to make more than one iteration.

      strm.avail_out = outputBufferSize;
      strm.next_out  = outputBuffer;
      
      // Now we call the compression engine itself, deflate(). It takes as many of the avail_in bytes at next_in as it can process,
      // and writes as many as avail_out bytes to next_out. Those counters and pointers are then updated past the input data consumed
      // and the output data written. It is the amount of output space available that may limit how much input is consumed. Hence the
      // inner loop to make sure that all of the input is consumed by providing more output space each time. Since avail_in and next_in
      // are updated by deflate(), we don't have to mess with those between deflate() calls until it's all used up.
      // The parameters to deflate() are a pointer to the strm structure containing the input and output information and the internal
      // compression engine state, and a parameter indicating whether and how to flush data to the output. Normally deflate will consume
      // several K bytes of input data before producing any output (except for the header), in order to accumulate statistics on the
      // data for optimum compression. It will then put out a burst of compressed data, and proceed to consume more input before the
      // next burst. Eventually, deflate() must be told to terminate the stream, complete the compression with provided input data, and
      // write out the trailer check value. deflate() will continue to compress normally as long as the flush parameter is Z_NO_FLUSH.
      // Once the Z_FINISH parameter is provided, deflate() will begin to complete the compressed output stream. However depending on
      // how much output space is provided, deflate() may have to be called several times until it has provided the complete compressed
      // stream, even after it has consumed all of the input. The flush parameter must continue to be Z_FINISH for those subsequent
      // calls.

      ret = deflate (&strm, Z_FINISH);
      if  (ret == Z_STREAM_ERROR)
      {
        cerr << "Compressor::CreateCompressedBuffer  ***ERROR**   defalte returned error[" << ret << "]." << std::endl;
        compressedBuffLen = 0;
        return NULL;
      }
      
      // Now we compute how much output deflate() provided on the last call, which is the difference between how much space was provided before the call, and how much output space is still available after the call. Then that data, if any, is written to the output file. We can then reuse the output buffer for the next call of deflate(). Again if there is a file i/o error, we call deflateEnd() before returning to avoid a memory leak. 
      int32 have = outputBufferSize - strm.avail_out;
      {
        if  (compressedBuff == NULL)
        {
          compressedBuff = new uchar[have];
          compressedBuffLen = have;
          memcpy (compressedBuff, outputBuffer, have);
        }
        else
        {
          uint32  newCompressedBuffLen = compressedBuffLen + have;
          uchar*  newCompressedBuff = new uchar[newCompressedBuffLen];
          memcpy (newCompressedBuff, compressedBuff, compressedBuffLen);
          memcpy (newCompressedBuff + compressedBuffLen, outputBuffer, have);
          delete  compressedBuff;
          compressedBuff    = newCompressedBuff;
          compressedBuffLen = newCompressedBuffLen;
        }
      }
    } while (strm.avail_out == 0);
   

    if  (strm.avail_in != 0)
    {
      // There is still input data;  so something went wrong. We will print a diagnostic message and return NULL
      cerr << "Compressor::CreateCompressedBuffer   -  The input buffer was not fully processed." << std::endl;
      delete  compressedBuff;  compressedBuff  = NULL;
      delete  outputBuffer;    outputBuffer    = NULL;
      return  NULL;
    }
  }

  // The process is complete, but we still need to deallocate the state to avoid a memory leak (or rather more like a memory hemorrhage
  // if you didn't do this). Then finally we can return with a happy return value.
  /* clean up and return */
 (void)deflateEnd (&strm);

 delete  outputBuffer;  outputBuffer = NULL;

 return  compressedBuff;

#else
 // For right now the linux version will not be doing compression.
 return  NULL;
#endif
}   /* CreateCompressedBuffer*/




void*   Compressor::Decompress (const void*  compressedBuff,
                                uint32       compressedBuffLen,
                                uint32&      unCompressedLen
                               )
{
#ifdef ZLIB_H
  if  (compressedBuff == NULL)
    return  NULL;

  GlobalGoalKeeper::StartBlock ();

  uint32     have              = 0;
  uchar*     unCompressedBuff  = NULL;

  Bytef*     outBuffer    = NULL;
  int32      outBufferLen = 0;

  int32      ret;
  z_stream   strm;

  /* allocate inflate state */
  strm.zalloc   = Z_NULL;
  strm.zfree    = Z_NULL;
  strm.opaque   = Z_NULL;
  strm.avail_in = 0;
  strm.next_in  = Z_NULL;

  ret = inflateInit (&strm);
  if  (ret != Z_OK)
  {
    cerr << "Compressor::Decompress  ***ERROR***  zlib function call 'inflateInit'  failed." << std::endl;
    unCompressedLen = 0;
    GlobalGoalKeeper::EndBlock ();
    return NULL;
  }


  outBufferLen = compressedBuffLen * 4;
  outBuffer    = new Bytef[outBufferLen];

  strm.avail_in = compressedBuffLen;
  strm.next_in  = (Bytef*)compressedBuff;  // Bytef = a far pointer to a unsigned char.    
                                           // Not sure if on some platforms this may cause a problem.

  /* run inflate() on input until output buffer not full */
  do 
  {
    strm.avail_out = outBufferLen;
    strm.next_out  = outBuffer;

    ret = inflate (&strm, Z_NO_FLUSH);
    if  (ret == Z_STREAM_ERROR)
    {
      cerr << "Compressor::Decompress  ***ERROR***  zlib function call 'inflate'  failed."  << std::endl;
      delete  outBuffer;  outBuffer = NULL;
      delete  unCompressedBuff;     unCompressedBuff    = NULL;
      unCompressedLen = 0;
      GlobalGoalKeeper::EndBlock ();
      return NULL;
    }

    switch (ret) 
    {
    case Z_NEED_DICT:  ret = Z_DATA_ERROR;     /* and fall through */
    case Z_DATA_ERROR:
    case Z_MEM_ERROR:
      {
        cerr << "Compressor::Decompress  ***ERROR***  zlib function call 'inflate'  failed."  << std::endl;
        (void)inflateEnd (&strm);
        delete  outBuffer;          outBuffer        = NULL;
        delete  unCompressedBuff;   unCompressedBuff = NULL;
        unCompressedLen = 0;
        GlobalGoalKeeper::EndBlock ();
        return NULL;
      }
    }

    have = outBufferLen - strm.avail_out;
    if  (unCompressedBuff == NULL)
    {
      unCompressedBuff = new uchar[have];
      unCompressedLen = have;
      memcpy (unCompressedBuff, outBuffer, have);
    }
    else
    {
      int32  newUnCompressedLen   = unCompressedLen + have;
      uchar* newUnCompressedBuff  = new uchar[newUnCompressedLen];
      memcpy (newUnCompressedBuff, unCompressedBuff, unCompressedLen);
      memcpy (newUnCompressedBuff + unCompressedLen, outBuffer, have);
      delete  unCompressedBuff;
      unCompressedBuff = newUnCompressedBuff;
      unCompressedLen = newUnCompressedLen;
      newUnCompressedBuff = NULL;
    }
  }
  while (strm.avail_out == 0);

  /* clean up and return */
  (void)inflateEnd(&strm);

  delete  outBuffer;
  outBuffer = NULL;

  GlobalGoalKeeper::EndBlock ();

  return  unCompressedBuff;
#else
  return NULL;
#endif
}  /* Decompress */

