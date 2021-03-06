/* TokenBuffer.cpp -- Class to Manage Token Parsing
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <iostream>
//#include  <istream>

#include "MemoryDebug.h"
using namespace std;


#include "TokenBuffer.h"
using namespace KKB;



TokenBuffer::TokenBuffer ()
{
}



TokenBuffer::~TokenBuffer ()
{
}



TokenBufferStr::TokenBufferStr (const KKStr&  _buff):
    buff        (_buff),
    nextCharPos (0)

{
}


TokenBufferStr::~TokenBufferStr ()
{
}



bool  TokenBufferStr::Valid ()
{
  return true;
}



char  TokenBufferStr::GetNextChar ()
{
  if  (nextCharPos >= buff.Len ())
    return 0;

  char  c = buff[nextCharPos];
  nextCharPos++;
  return c;
}  /* GetNextChar */



char  TokenBufferStr::PeekNextChar ()
{
  if  (nextCharPos >= buff.Len ())
    return 0;
  return  buff[nextCharPos];
}



void  TokenBufferStr::UnGetNextChar ()
{
  if  (nextCharPos > 0)
    --nextCharPos;
}



bool  TokenBufferStr::EndOfFile ()
{
  if  (nextCharPos >= buff.Len ())
    return true;
  else
    return false;
}  /* EndOfFile */






TokenBufferStream::TokenBufferStream (istream*  _in):
    TokenBuffer (),
    endOfFile   (false),
    fileName    (),
    fileStream  (NULL),
    in          (_in)
{
}


TokenBufferStream::TokenBufferStream (const KKStr&  _fileName):
    TokenBuffer (),
    endOfFile   (false),
    fileName    (_fileName),
    fileStream  (NULL),
    in          (NULL)
{
  fileStream  = new ifstream (fileName.Str ());
  if  (!fileStream->is_open ())
  {
    delete fileStream;
    fileStream = NULL;
  }
  else
  {
    in = fileStream;
  }
}



TokenBufferStream::~TokenBufferStream ()
{
  if  (fileStream)
  {
    in = NULL;
    delete  fileStream;
    fileStream = NULL;
  }
}





bool  TokenBufferStream::Valid ()
{
  if  (fileStream)
  {
    return  fileStream->is_open ();
  }

  return (in != NULL);
}  /* Valid */




char  TokenBufferStream::GetNextChar ()
{
  if  (endOfFile)
    return 0;

  char c = (char)(in->get ());
  if  (in->eof())
  {
    endOfFile = true;
    c = 0;
  }
  return c;
}  /* GetNextChar */



char  TokenBufferStream::PeekNextChar ()
{
  if  (in->eof ())
    return 0;
  return  (char)(in->peek ());
}




void  TokenBufferStream::UnGetNextChar ()
{
  in->unget ();
}



bool  TokenBufferStream::EndOfFile ()
{
  return  endOfFile;
}
