/* Tokenizer.cpp -- Class to Manage Token Parsing
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#include "FirstIncludes.h"
#include <iostream>
#include <fstream>
#include <map>
#include <string.h>
#include <string>
#include <vector>

#include "MemoryDebug.h"
using namespace std;


#include "Tokenizer.h"
#include "Str.h"
#include "TokenBuffer.h"
using namespace KKU;



Tokenizer::Tokenizer (TokenBufferPtr  _in):

  atEndOfFile           (false),
  in                    (_in),
  secondCharAtEndOfFile (false),
  tokenList             (true),
  weOwnTokenBuffer      (false)

{
  Initialize ();
}



Tokenizer::Tokenizer (const KKStr&  _str):

  atEndOfFile           (false),
  in                    (NULL),
  secondCharAtEndOfFile (false),
  tokenList             (true),
  weOwnTokenBuffer      (false)

{
  in = new TokenBufferStr (_str);
  weOwnTokenBuffer = true;
  Initialize ();
}



Tokenizer::Tokenizer (const KKStr&  _fileName,
                      bool&         _fileOpened
                     ):

  atEndOfFile           (false),
  in                    (NULL),
  secondCharAtEndOfFile (false),
  tokenList             (true),
  weOwnTokenBuffer      (false)

{
  in = new TokenBufferStream (_fileName);
  _fileOpened = (in->Valid ());
  if  (_fileOpened)
  {
    weOwnTokenBuffer = true;
    Initialize ();
  }
}




Tokenizer::~Tokenizer ()
{
  if  (weOwnTokenBuffer)
  {
    delete  in;
    in = NULL;
  }
}





void  Tokenizer::Initialize ()
{
  GetNextChar ();
  GetNextChar ();

  tokenListLen = 10;

  while  (tokenList.QueueSize () < tokenListLen)
  {
    ReadInNextLogicalToken ();
  }
}  /* Initialize */




KKStrPtr  Tokenizer::GetNextToken ()
{
  while  (tokenList.QueueSize () < 1)
    ReadInNextLogicalToken ();

  KKStrPtr t = tokenList.PopFromFront ();
  return  t;
}  /* GetNextToken */



/**
 @brief  Will return a list of tokens up to and including the first occurrence if 'delToken'.
 */
KKStrListPtr  Tokenizer::GetNextTokens (const KKStr& delToken)
{
  if  (delToken.Empty ())
    return NULL;

  KKStrListPtr  tokens = new KKStrList (true);
  KKStrPtr  t = GetNextToken ();
  if  (t == NULL)
    return NULL;

  while  ((t != NULL)  &&  (*t != delToken))
  {
    tokens->PushOnBack (t);
    t = GetNextToken ();
  }

  if  (t)
    tokens->PushOnBack (t);

  return  tokens;
}  /* GetNextTokens */





KKStrConstPtr  Tokenizer::Peek (uint32 idx)
{
  while  ((tokenList.QueueSize () < (int32)(idx + 1))  &&  !atEndOfFile)
    ReadInNextLogicalToken ();

  if  (idx >= tokenList.size ())
    return NULL;

  return  tokenList.IdxToPtr ((int32)idx);
}  /* Peek */





bool  Tokenizer::EndOfFile ()
{
//  if  (tokenList.QueueSize () == 0)
//    return true;
  while  ((tokenList.QueueSize () < 1)  &&  (!atEndOfFile))
    ReadInNextLogicalToken ();

  return  (tokenList.QueueSize () < 1);
}  /* EndOfFile */





char  Tokenizer::GetNextChar ()
{
  if  (atEndOfFile)
  {
    firstChar   = 0;
    secondChar  = 0;
  }

  else if  (secondCharAtEndOfFile)
  {
    firstChar   = 0;
    secondChar  = 0;
    atEndOfFile = true;
  }

  else
  {
    firstChar  = secondChar;
    if  (in->EndOfFile ())
    {
      secondChar = 0;
      secondCharAtEndOfFile = true;
    }
    else
    {
      secondChar = in->GetNextChar ();
    }
  }

  return  firstChar;
}  /* GetNextChar */




void  Tokenizer::ReadInNextLogicalToken ()
{
  KKStrPtr  t = GetNextTokenRaw ();
  if  (t == NULL)
  {
    //tokenList.PushOnBack (new Token (tokEndOfFile, "EndOfFile"));
  }
  else
  {
    tokenList.PushOnBack (t);
  }
}  /* ReadInNextLogicalToken */



bool  Tokenizer::WhiteSpaceChar (char c)  const
{
  if  (strchr (" ", c) == NULL)
    return false;
  else
    return true;
}  /* WhiteSpaceChar */




bool  Tokenizer::DelimiterChar (char c)  const
{
  if  (strchr ("\n\r\t", c) == NULL)
    return true;
  else
    return false;
}




KKStrPtr  Tokenizer::GetNextTokenRaw ()
{
  if  (atEndOfFile)
    return NULL;

  // Lets skip whitespace
  while  ((firstChar == ' ')  &&  (!atEndOfFile))
    GetNextChar ();

  if  (firstChar == '\n')
  {
    if  (secondChar == '\r')
      GetNextChar ();
  }
  else if  (firstChar == '\r')
  {
    if  (secondChar == '\n')
      GetNextChar ();
  }

  if  (atEndOfFile)
  {
    return  NULL;
  }
 
  KKStrPtr  nextRawToken = NULL;


  if  ((firstChar == '"')  ||  (firstChar == '\''))
  {
    // We are at the start of a strng
    nextRawToken = ProcessStringToken (firstChar);
  }
  else
  {
    nextRawToken = ProcessFieldToken ();
  }

  return  nextRawToken;
}  /* Get Next Token */




KKStrPtr  Tokenizer::ProcessStringToken (char strDelChar)
{
  if  (firstChar  == strDelChar)
    GetNextChar ();

  KKStr  str (20);

  // Scan until we hit another '"' character,  or end of KKStr.
  while  (!atEndOfFile)
  {
    if (firstChar == strDelChar)
    {
      // We reached the end of the string
      GetNextChar ();
      break;
    }

    else if  (firstChar  == '\\')
    {
      GetNextChar ();
      // We have a escape character.
      switch  (firstChar)
      {
      case '\'': str.Append ('\'');  break;
      case  '"': str.Append ('"');   break;
      case  't': str.Append ('\t');  break;
      case  'n': str.Append ('\n');  break;
      case  'r': str.Append ('\r');  break;
      default:   str.Append (firstChar); break;
      } 
    }

    else
    {
      str.Append (firstChar);
    }

    GetNextChar ();
  }
  return new KKStr (str);

}  /* ProcessStringToken */





KKStrPtr  Tokenizer::ProcessFieldToken ()
{
  // We have a token that we don't recognize. We will create a token 
  // of type tokNULL and place all characters up till the next whitespace 
  // or delimiter character.
  KKStrPtr  field = new KKStr (10);
  while  ((!WhiteSpaceChar (firstChar)) &&  
          (!DelimiterChar (firstChar))  &&  
          (!atEndOfFile)
         )
  {
    field->Append (firstChar);
    GetNextChar ();
  }

  return  field;
}  /* ProcessFieldToken */




KKStrConstPtr  Tokenizer::operator[](uint32 idx)
{
  return Peek (idx);
}  /* operator[] */
