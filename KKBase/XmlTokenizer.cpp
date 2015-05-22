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

  atEndOfFile      (false),
  in               (_in),
  tokenList        (),
  weOwnTokenBuffer (false),
  logger1          ("C:\\Temp\\XmlTokenizer-1.txt"),
  logger2          ("C:\\Temp\\XmlTokenizer-2.txt")
{
  Initialize ();
}



XmlTokenizer::XmlTokenizer (const KKStr&  _str):

  atEndOfFile       (false),
  in                (NULL),
  tokenList         (),
  weOwnTokenBuffer  (false),
  logger1           ("C:\\Temp\\XmlTokenizer-1.txt"),
  logger2           ("C:\\Temp\\XmlTokenizer-2.txt")
{
  in = new TokenBufferStr (_str);
  weOwnTokenBuffer = true;
  Initialize ();
}



XmlTokenizer::XmlTokenizer (const KKStr&  _fileName,
                            bool&         _fileOpened
                           ):

  atEndOfFile       (false),
  in                (NULL),
  tokenList         (),
  weOwnTokenBuffer  (false),
  logger1           ("C:\\Temp\\XmlTokenizer-1.txt"),
  logger2           ("C:\\Temp\\XmlTokenizer-2.txt")
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

  tokenListLen = 10;

  while  ((tokenList.size () < tokenListLen)  &&  (!atEndOfFile))
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
  while  ((tokenList.size () < 1)  && (!atEndOfFile))
    ReadInNextLogicalToken ();

  if  (tokenList.size () < 1)
  {
    logger2 << "GetNextToken    return NULL" << endl;
    logger2.flush ();
    return NULL;
  }

  kkuint32 s = tokenList.size ();

  KKStrPtr t = tokenList.front ();
  tokenList.pop_front ();

  logger2 << "GetNextToken size[" << s << "] :" << (t ? (*t) : "NULL") << endl;
  logger2.flush ();

  if  (*t == "<ModelOldSVM>")
    logger2 << "GetNextToken   At Break Point." << endl;

  return  t;
}  /* GetNextToken */



/**
 @brief  Will return a list of tokens up to and including the first occurrence if 'delToken'.
 */
KKStrListPtr  XmlTokenizer::GetNextTokens (const KKStr& delToken)
{
  if  (delToken.Empty ())
    return NULL;

  if  (atEndOfFile  &&  (tokenList.size () < 1))
    return NULL;

  KKStrPtr  t = GetNextToken ();
  if  (t == NULL)
    return NULL;

  KKStrListPtr  tokens = new KKStrList (true);
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
  tokenList.push_front (t);
}




KKStrConstPtr  XmlTokenizer::Peek (kkuint32 idx)
{
  while  ((tokenList.size () < (idx + 1))  &&  !atEndOfFile)
    ReadInNextLogicalToken ();

  if  (idx >= tokenList.size ())
  {
    logger2 << "Peek  idx[" << idx << "]   returning NULL" << endl;
    return NULL;
  }

  logger2 << "Peek  idx[" << idx << "]  :" << *(tokenList[idx]) << endl;

  return  tokenList[idx];
}  /* Peek */



bool  XmlTokenizer::EndOfFile ()
{
//  if  (tokenList.QueueSize () == 0)
//    return true;
  while  ((tokenList.size () < 1)  &&  (!atEndOfFile))
    ReadInNextLogicalToken ();

  return  (tokenList.size () < 1);
}  /* EndOfFile */



char  XmlTokenizer::GetNextChar ()
{
  if  (atEndOfFile)
  {
    firstChar = 0;
  }
  else if  (in->EndOfFile ())
  {
    atEndOfFile = true;
    firstChar = 0;
    logger1 << endl << "GetNextChar  atEndOfFile = true;" << endl;
  }
  else
  {
    firstChar = in->GetNextChar ();
    if  (in->EndOfFile ())
    {
      atEndOfFile = true;
      firstChar = 0;
      logger1 << endl << "GetNextChar  atEndOfFile = true;" << endl;
    }
    else
    {
      logger1 << firstChar;
      if  (firstChar == '\r')
      {
        if  (in->PeekNextChar () == '\n')
          firstChar = in->GetNextChar ();
      }
    }
  }

  logger1.flush ();

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
    tokenList.push_back (t);
  }

  logger2 << "ReadInNextLogicalToken size[" << tokenList.size () << "]  :" << (t ? (*t) : "NULL RETURNED") << endl;
  logger2.flush ();
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
}  /* GetNextTokenRaw */




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
        if  (firstChar == '\\')
        {
          GetNextChar ();
          switch  (firstChar)
          {
          case   't':  firstChar = '\t';  break;
          case   'n':  firstChar = '\n';  break;
          case   'r':  firstChar = '\r';  break;
          case   '0':  firstChar = '\0';  break;
          case  '\\':  firstChar = '\\';  break;
          case   '"':  firstChar = '"';   break;
          }
        }
        token->Append (firstChar);
        GetNextChar ();
      }

      if  (firstChar == endingQuoteChar)
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

    // If there is a trailing carriage-return line-feed or just line-feed;  we want to skip past them.
    if  (firstChar == '\r')  GetNextChar ();
    if  (firstChar == '\n')  GetNextChar ();
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
  {
    atEndOfFile = true;
    return;
  }

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
  KKStrPtr  token = new KKStr(512);

  while  ((!atEndOfFile)  &&  (firstChar != '<')  &&  (firstChar != '\n'))
  {
    if  (firstChar == '&')
      ProcessAmpersand ();
    token->Append (firstChar);
    GetNextChar ();
  }

  //token->TrimRight (" \r\n");

  if  ((firstChar == '\n')  &&  (!atEndOfFile))
    GetNextChar ();

  // At this point we are either at end-of-file, end-of-line,  or the next character is "<" start of a tag field.
  return  token;
}  /* ProcessTagToken */







KKStrConstPtr  XmlTokenizer::operator[](kkuint32 idx)
{
  return Peek (idx);
}  /* operator[] */
