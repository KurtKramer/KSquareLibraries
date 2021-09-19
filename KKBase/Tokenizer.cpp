/* Tokenizer.cpp -- Class to Manage Token Parsing
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string.h>
#include "MemoryDebug.h"
using namespace std;

#include "KKBaseTypes.h"
#include "Tokenizer.h"
#include "KKStr.h"
#include "TokenBuffer.h"
using namespace KKB;



Tokenizer* Tokenizer::OpenFile (const KKStr&  _fileName)
{
  bool fileOpened = false;
  auto  tokenizer = new Tokenizer (_fileName, fileOpened);
  if  (!fileOpened)
  {
    delete tokenizer;
    tokenizer = NULL;
  }
  return tokenizer;
}



Tokenizer::Tokenizer (TokenBufferPtr  _in):
  atEndOfFile           (false),
  in                    (_in),
  secondCharAtEndOfFile (false),
  operatorChars         (NULL),
  tokenList             (true),
  weOwnTokenBuffer      (false)
{
  Initialize ();
}



Tokenizer::Tokenizer (const KKStr&  _str):
  atEndOfFile           (false),
  in                    (NULL),
  secondCharAtEndOfFile (false),
  operatorChars         (NULL),
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
  operatorChars         (NULL),
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
  delete  operatorChars;
  operatorChars = NULL;
}



void  Tokenizer::Initialize ()
{
  DefineOperatorChars (",+-*/^=%[]{}()<>");

  GetNextChar ();
  GetNextChar ();

  kkuint32 tokenListLen = 10;

  while  (tokenList.QueueSize () < tokenListLen)
  {
    ReadInNextLogicalToken ();
  }
}  /* Initialize */



void  Tokenizer::DefineOperatorChars (char const * _operatorChars)
{
  delete  operatorChars ;
  operatorChars = KKB::STRDUP (_operatorChars);
}



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



void  Tokenizer::PushTokenOnFront (KKStrPtr  t)
{
  tokenList.PushOnFront (t);
}



KKStrConstPtr  Tokenizer::Peek (kkuint32 idx)
{
  while  ((tokenList.QueueSize () < (idx + 1))  &&  !atEndOfFile)
    ReadInNextLogicalToken ();

  if  (idx >= tokenList.size ())
    return NULL;

  return  tokenList.IdxToPtr (idx);
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
  return  (strchr ("\n\r\t", c) != NULL);
}



bool  Tokenizer::OperatorChar (char c)  const
{
  return  (strchr (operatorChars, c) != NULL);
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
    // We are at the start of a string
    nextRawToken = ProcessStringToken (firstChar);
  }

  else if  (OperatorChar (firstChar))
  {
    nextRawToken = ProcessOperatorToken ();
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

  KKStr  str (20U);

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



KKStrPtr  Tokenizer::ProcessOperatorToken ()
{
  KKStrPtr  field = new KKStr (4U);
  field->Append (firstChar);

  if  ((firstChar == '+')  &&  (secondChar == '+'))
  {
    field->Append (secondChar);
    GetNextChar ();
  }

  else if  ((firstChar == '-')  &&  (secondChar == '-'))
  {
    field->Append (secondChar);
    GetNextChar ();
  }

  else if  (firstChar == '=')
  {
    if  (strchr ("=<>+-*/^", secondChar) != NULL)
    {
      field->Append (secondChar);
      GetNextChar ();
    }
  }

  else if  (strchr ("+-*/^<>", firstChar) != NULL)
  {
    if  (secondChar == '=')
    {
      field->Append (secondChar);
      GetNextChar ();
    }
  }

  return  field;
}  /* ProcessFieldToken */



KKStrPtr  Tokenizer::ProcessFieldToken ()
{
  // We have a token that we don't recognize.  We will create a token 
  // of type tokNULL and place all characters up till the next whitespace 
  // or delimiter character.
  KKStrPtr  field = new KKStr (10U);
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



KKStrConstPtr  Tokenizer::operator[](kkuint32 idx)
{
  return Peek (idx);
}  /* operator[] */
