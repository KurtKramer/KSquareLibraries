/* XmlTokenizer.cpp -- Class to Manage Token Parsing
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


#include "XmlTokenizer.h"
#include "KKStr.h"
#include "TokenBuffer.h"
using namespace KKB;



XmlTokenizer::XmlTokenizer (TokenBufferPtr  _in):

  atEndOfFile           (false),
  in                    (_in),
  secondCharAtEndOfFile (false),
  tokenList             (true),
  weOwnTokenBuffer      (false)
{
  Initialize ();
}



XmlTokenizer::XmlTokenizer (const KKStr&  _str):

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



XmlTokenizer::XmlTokenizer (const KKStr&  _fileName,
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




XmlTokenizer::~XmlTokenizer ()
{
  if  (weOwnTokenBuffer)
  {
    delete  in;
    in = NULL;
  }
}





void  XmlTokenizer::Initialize ()
{
  GetNextChar ();
  GetNextChar ();

  tokenListLen = 10;

  while  (tokenList.QueueSize () < tokenListLen)
  {
    ReadInNextLogicalToken ();
  }
}  /* Initialize */




KKStrPtr  XmlTokenizer::GetNextToken ()
{
  while  (tokenList.QueueSize () < 1)
    ReadInNextLogicalToken ();

  KKStrPtr t = tokenList.PopFromFront ();
  return  t;
}  /* GetNextToken */



/**
 @brief  Will return a list of tokens up to and including the first occurrence if 'delToken'.
 */
KKStrListPtr  XmlTokenizer::GetNextTokens (const KKStr& delToken)
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



void  XmlTokenizer::PushTokenOnFront (KKStrPtr  t)
{
  tokenList.PushOnFront (t);
}




KKStrConstPtr  XmlTokenizer::Peek (kkuint32 idx)
{
  while  ((tokenList.QueueSize () < (kkint32)(idx + 1))  &&  !atEndOfFile)
    ReadInNextLogicalToken ();

  if  (idx >= tokenList.size ())
    return NULL;

  return  tokenList.IdxToPtr ((kkint32)idx);
}  /* Peek */



bool  XmlTokenizer::EndOfFile ()
{
//  if  (tokenList.QueueSize () == 0)
//    return true;
  while  ((tokenList.QueueSize () < 1)  &&  (!atEndOfFile))
    ReadInNextLogicalToken ();

  return  (tokenList.QueueSize () < 1);
}  /* EndOfFile */



char  XmlTokenizer::GetNextChar ()
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

  if  ((firstChar == '\r')  &&  (secondChar == '\n'))
  {
    GetNextChar ();
  }

  return  firstChar;
}  /* GetNextChar */




void  XmlTokenizer::ReadInNextLogicalToken ()
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



bool  XmlTokenizer::WhiteSpaceChar (char c)  const
{
  if  (strchr (" ", c) == NULL)
    return false;
  else
    return true;
}  /* WhiteSpaceChar */




KKStrPtr  XmlTokenizer::GetNextTokenRaw ()
{
  if  (atEndOfFile)
    return NULL;

  // Lets skip whitespace
  while  (WhiteSpaceChar (firstChar)  &&  (!atEndOfFile))
    GetNextChar ();
  if  (atEndOfFile)
  {
    return  NULL;
  }
 
  KKStrPtr  nextRawToken = NULL;

  if  (firstChar == '<')
  {
    // We are start of tag token
    nextRawToken = ProcessTagToken ();
  }

  else
  {
    nextRawToken = ProcessBodyToken ();
  }


  return  nextRawToken;
}  /* Get Next Token */






KKStrPtr  XmlTokenizer::ProcessStringToken (char strDelChar)
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


KKStrPtr  XmlTokenizer::ProcessTagToken ()
{
  KKStrPtr  token = new KKStr(100);
  token->Append (firstChar);
  GetNextChar ();

  while  ((!atEndOfFile)  &&  (firstChar != '>'))
  {
    if  ((firstChar == '"')   ||  (firstChar == '\''))
    {
      // We are starting a quote;  will scan characters literately until we reach end of quote */
      char  endingQuoteChar = firstChar;
      token->Append (firstChar);
      GetNextChar ();

      while  ((!atEndOfFile)  &&  (firstChar != endingQuoteChar))
      {
        if  (firstChar == '\'')
        {
          GetNextChar ();
          token->Append (firstChar);
        }
        GetNextChar ();
      }

      if  (firstChar != endingQuoteChar)
      {
        token->Append (firstChar);
        GetNextChar ();
      }
    }
    else
    {
      token->Append (firstChar);
      GetNextChar ();
    }
  }

  if  (!atEndOfFile)
  {
    token->Append (firstChar);
    GetNextChar ();
  }

  return  token;
}  /* ProcessTagToken */




KKStrPtr  XmlTokenizer::ProcessBodyToken ()
{
  KKStrPtr  token = new KKStr(100);

  while  ((!atEndOfFile)  &&  (firstChar != '<'))
  {
    if  ((firstChar == '"')   ||  (firstChar == '\''))
    {
      // We are starting a quote;  will scan characters literately until we reach end of quote */
      char  endingQuoteChar = firstChar;
      token->Append (firstChar);
      GetNextChar ();

      while  ((!atEndOfFile)  &&  (firstChar != endingQuoteChar))
      {
        if  (firstChar == '\'')
        {
          GetNextChar ();
          token->Append (firstChar);
        }
        GetNextChar ();
      }

      if  (firstChar != endingQuoteChar)
      {
        token->Append (firstChar);
        GetNextChar ();
      }
    }
    else
    {
      token->Append (firstChar);
      GetNextChar ();
    }
  }

  // At this point we are either at the end of the file or the next character is "<" tart of a tag field.
  return  token;
}  /* ProcessTagToken */







KKStrConstPtr  XmlTokenizer::operator[](kkuint32 idx)
{
  return Peek (idx);
}  /* operator[] */
