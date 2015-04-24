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
  entityMap.insert (pair<KKStr,char> ("quot",'"'));
  entityMap.insert (pair<KKStr,char> ("amp", '&'));
  entityMap.insert (pair<KKStr,char> ("apos",'\''));
  entityMap.insert (pair<KKStr,char> ("lt",  '<'));
  entityMap.insert (pair<KKStr,char> ("gt",  '>'));
  entityMap.insert (pair<KKStr,char> ("tab", '\t'));
  entityMap.insert (pair<KKStr,char> ("lf",  '\n'));
  entityMap.insert (pair<KKStr,char> ("cr",  '\r'));

  GetNextChar ();
  GetNextChar ();

  tokenListLen = 10;

  while  (tokenList.QueueSize () < tokenListLen)
  {
    ReadInNextLogicalToken ();
  }
}  /* Initialize */




char  XmlTokenizer::LookUpEntity (const KKStr&  entityName)  const
{
  map<KKStr,char>::const_iterator  idx;
  idx = entityMap.find (entityName);
  if  (idx == entityMap.end ())
    return 0;
  else
    return idx->second;
}




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
    firstChar = 0;
  }
  else
  {
    firstChar = in->GetNextChar ();
    if  (firstChar == '\r')
    {
      if  (in->PeekNextChar () == '\n')
        firstChar = in->GetNextChar ();
    }
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




/**
 *@brief  Processes a XML entity such as "&lt;";  when you encounter a ampersand (&) in the stream you 
 * call this method; it will scan until it reaches the matching semi colon(';') character. The word
 * located between the '&' and ';' will be used to look up the appropriate replacement character 
 * in 'entityMap'.
 */
void  XmlTokenizer::ProcessAmpersand ()
{
  KKStr  entityName  (10);
  if  (in->EndOfFile ())
    return;

  char ch = in->GetNextChar ();
  while  ((!in->EndOfFile ())  &&  (ch != ';')  &&  (entityName.Len () < 10))
  {
    entityName.Append (ch);
    ch = in->GetNextChar ();
  }

  if  (ch != ';')
  {
    // Name is getting too long; the ampersand is invalid; will return characters as is.
    while  (entityName.Len () > 0)
    {
      char ch = entityName.ExtractLastChar ();
      in->UnGetNextChar ();
    }
  }
  else
  {
    char ch = LookUpEntity (entityName);
    firstChar = ch;
  }
}  /* ProcessAmpersand */





KKStrPtr  XmlTokenizer::ProcessBodyToken ()
{
  KKStrPtr  token = new KKStr(100);

  while  ((!atEndOfFile)  &&  (firstChar != '<'))
  {
    if  (firstChar == '&')
      ProcessAmpersand ();
    token->Append (firstChar);
    GetNextChar ();
  }

  // At this point we are either at the end of the file or the next character is "<" tart of a tag field.
  return  token;
}  /* ProcessTagToken */







KKStrConstPtr  XmlTokenizer::operator[](kkuint32 idx)
{
  return Peek (idx);
}  /* operator[] */
