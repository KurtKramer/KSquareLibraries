/* TokenBuffer.cpp -- Class to Manage Token Parsing
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#include  "FirstIncludes.h"
#include  <stdio.h>
#include  <stdlib.h>
#include  <vector>
#include  <fstream>
#include  <iostream>
//#include  <istream>

#include  "MemoryDebug.h"
using namespace std;


#include  "TokenBuffer.h"
using namespace KKU;



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
  char c;
  if  (endOfFile)
    return 0;

  c = in->get ();
  if  (in->eof())
    endOfFile = true;
  return c;
}  /* GetNextChar */



bool  TokenBufferStream::EndOfFile ()
{
  return  endOfFile;
}
