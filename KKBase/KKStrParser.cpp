/* KKStrParser.cpp -- Class used to parse string into tokens.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */

#include "FirstIncludes.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <string>
#include <vector>

#include "MemoryDebug.h"
using namespace std;

#include "KKBaseTypes.h"
#include "DateTime.h"
#include "KKStr.h"
using namespace KKB;


#include "KKStrParser.h"



KKStrParser::KKStrParser (const KKStrParser&  _strParser):
    len            ((kkuint32)strlen (_strParser.str)),
    nextPos        (_strParser.nextPos),
    str            (_strParser.str),
    trimWhiteSpace (_strParser.trimWhiteSpace),
    whiteSpace     (NULL)
{
  if  (_strParser.whiteSpace)
    whiteSpace = STRDUP (_strParser.whiteSpace);
}



KKStrParser::KKStrParser (const KKStr&  _str):
    len            (_str.Len ()),
    nextPos        (0),
    str            (_str.Str ()),
    trimWhiteSpace (false),
    whiteSpace     (NULL)
{
}


KKStrParser::KKStrParser (const char*  _str):
    len            ((kkuint16)strlen (_str)),
    nextPos        (0),
    str            (_str),
    trimWhiteSpace (false),
    whiteSpace     (NULL)
{
}




KKStrParser::~KKStrParser ()
{
  delete  whiteSpace;
}




void  KKStrParser::TrimWhiteSpace (const char*  _whiteSpace)
{
  delete  whiteSpace;  
  whiteSpace = NULL;

  trimWhiteSpace = true;
  if  (_whiteSpace)
    whiteSpace = STRDUP (_whiteSpace);
  else
    whiteSpace = STRDUP (" ");

  SkipWhiteSpace (whiteSpace);
}



void  KKStrParser::SkipWhiteSpace (const char*  whiteSpace)
{
  while  ((nextPos < len)  &&  (strchr (whiteSpace, str[nextPos]) != NULL))
    nextPos++;
}  /* SkipWhiteSpce */



VectorKKStr  KKStrParser::Split (const char* delStr)
{
  VectorKKStr  tokens;
  while  (MoreTokens ())
   tokens.push_back (this->GetNextToken (delStr));
  return  tokens;
}





KKStr  KKStrParser::GetNextToken (const char* delStr)
{
  if  (trimWhiteSpace)
  {
    while  ((nextPos < len)  &&  (strchr (whiteSpace, str[nextPos]) != NULL))
      nextPos++;
  }

  if (nextPos >= len)
    return KKStr::EmptyStr ();

  kkuint32  startPos = nextPos;
  kkuint32  endPos   = startPos;

  char ch = str[endPos];
  if  ((ch == '\'')  ||  (ch == '"'))
  {
    KKStr  token (30);
    // Token is a string
    char  quoteChar = ch;

    // We have a quoted String need to skip to end of quote.

    ++endPos;  // Skipped past initial quote character.
    while  (endPos < len)
    {
      ch = str[endPos];
      if  (ch == quoteChar)
        break;

      if  ((ch == '\\')  &&  (endPos < (len - 1)))
      {
        ++endPos;
        char  ec = str[endPos];
        switch  (ec)
        {
        case '\\':  ch = '\\';  break;
        case  '"':  ch = '"';  break;
        case  'r':  ch = '\r';  break;
        case  'n':  ch = '\n';  break;
        case  't':  ch = '\t';  break;
        }
      }

      token.Append (ch);
      ++endPos;
    }

    if  (endPos >= len)
    {
      nextPos = len;
    }
    else
    {
      nextPos = endPos + 1;  // Setting to character after the ending quote.

      // Now that we are at the end of the quoted String we need set the next character pointer to the next delimiter character.
      while  (nextPos < len)
      {
        ch = str[nextPos];
        if  (strchr (delStr, ch) != NULL)
          break;
        ++endPos;
      }
    }  
    return  token;
  }
  else
  {
    // scan until end of string or next delimiter
    kkuint32  delimeterIdx = 0;
    bool      delimeterFound = false;
    while  (endPos < len)
    {
      ch = str[endPos];
      if  (strchr (delStr, ch) != NULL)
      {
        delimeterFound = true;
        delimeterIdx = endPos;
        break;
      }
      ++endPos;
    }

    endPos--;  // Move endPos back to last character in token.

    if  (trimWhiteSpace)
    {
      while  ((endPos >= startPos)  &&  (strchr (whiteSpace, str[endPos]) != NULL))
        endPos--;
    }


    if  (delimeterFound)
      nextPos = delimeterIdx + 1;

    else if  (endPos >= len)
      nextPos = len;

    else
      nextPos = endPos + 1;

    if  (trimWhiteSpace)
    {
      while  ((nextPos < len)  &&  (strchr (whiteSpace, str[nextPos]) != NULL))
        ++nextPos;
    }

    return KKStr (str, startPos, endPos);
  }
}  /* GetNextToken */




/**
 *@brief  Returns what the next token that 'GetNextToken' will without updating the position in the string buffer.
 *@details  Allows you to see what the token would be without updating the KKStrParser instance.
 *@param[in]  delStr List of delimiting characters.
 *@returns  Next Token to be returned by 'GetNextToken'.
 */
KKStr  KKStrParser::PeekNextToken (const char* delStr)  const
{
  kkuint32  nextPosP = nextPos;

  if  (trimWhiteSpace)
  {
    while  ((nextPos < len)  &&  (strchr (whiteSpace, str[nextPosP]) != NULL))
      nextPosP++;
  }

  if (nextPosP >= len)
    return KKStr::EmptyStr ();

  kkuint32  startPos = nextPosP;
  kkuint32  endPos   = startPos;

  char ch = str[endPos];
  if  ((ch == '\'')  ||  (ch == '"'))
  {
    KKStr  token (30);
    // Token is a string
    char  quoteChar = ch;
    // We have a quoted String need to skip to end of quote.
    ++endPos;
    while  (endPos < len)
    {
      ch = str[endPos];
      if  (ch == quoteChar)
      {
        ++endPos;
        break;
      }

      if  ((ch == '\\')  &&  (endPos < (len - 1)))
      {
        ++endPos;
        char  ec = str[endPos];
        switch  (ec)
        {
        case  '"':  ch =  '"';  break;
        case '\\':  ch = '\\';  break;
        case  'r':  ch = '\r';  break;
        case  'n':  ch = '\n';  break;
        case  't':  ch = '\t';  break;
        }
      }

      token.Append (ch);
      ++endPos;
    }
    return  token;
  }
  else
  {
    // scan until end of string or next delimiter
    while  (endPos < len)
    {
      ch = str[endPos];
      if  (strchr (delStr, ch) != NULL)
      {
        break;
      }
      ++endPos;
    }

    endPos--;  // Move endPos back to last character in token.

    if  (trimWhiteSpace)
    {
      while  ((endPos >= startPos)  &&  (strchr (whiteSpace, str[endPos]) != NULL))
        endPos--;
    }

    return KKStr (str, startPos, endPos);
  }
}  /* PeekNextToken */



char  KKStrParser::GetNextChar ()
{
  char  nextChar = 0;
  if  (nextPos < len)
  {
    nextChar = str[nextPos];
    ++nextPos;
  }
  return  nextChar;
}



char  KKStrParser::GetLastChar ()
{
  char  lastChar = 0;
  if  (nextPos < len)
  {
    --len;
    lastChar = str[len];
  }
  return  lastChar;
}



char  KKStrParser::PeekNextChar ()  const
{
  char  nextChar = 0;
  if  (nextPos < len)
  {
    nextChar = str[nextPos];
  }
  return  nextChar;
}



char  KKStrParser::PeekLastChar ()  const
{
  char  lastChar = 0;
  if  (nextPos < len)
  {
    lastChar = str[len - 1];
  }
  return  lastChar;
}




char  KKStrParser::GetNextTokenChar (const char* delStr)
{
  KKStr  nextToken = GetNextToken (delStr);
  char nextTokenChar = nextToken[0];
  return  nextTokenChar;
}


KKB::DateTime  KKStrParser::GetNextTokenDateTime (const char* delStr)
{
  return  KKB::DateTime (GetNextToken (delStr));
}




kkint32  KKStrParser::GetNextTokenInt (const char* delStr)
{
  return  GetNextToken (delStr).ToInt ();
}



long  KKStrParser::GetNextTokenLong (const char* delStr)
{
  return GetNextToken (delStr).ToLong ();
}



double  KKStrParser::GetNextTokenDouble (const char* delStr)
{
  return  GetNextToken (delStr).ToDouble ();
}


float  KKStrParser::GetNextTokenFloat  (const char* delStr)
{
  return  GetNextToken (delStr).ToFloat ();
}



kkuint32  KKStrParser::GetNextTokenUint (const char* delStr)
{
  return  GetNextToken (delStr).ToUint ();
}



bool  KKStrParser::GetNextTokenBool   (const char* delStr)
{
  return  GetNextToken (delStr).ToBool ();
}



KKStr  KKStrParser::GetRestOfLine ()
{
  if  (trimWhiteSpace)
  {
    while  ((nextPos < len)  &&  (strchr (whiteSpace, str[nextPos]) != NULL))
      nextPos++;
  }

  if (nextPos >= len)
    return KKStr::EmptyStr ();

  KKStr  result (1 + len - nextPos);
  char  lastChar = 0;

  while  (nextPos < len)  
  {
    lastChar = str[nextPos];
    if  (lastChar == '\n')
      break;

    else if  (lastChar == '\r')
      break;

    result.Append (lastChar);
    nextPos++;
  }

  if  (nextPos < len)
  {
    if  (lastChar == '\n')
    {
      if  (str[nextPos] == '\r')
        nextPos++;
    }
    else if  (lastChar == '\r')
    {
      if  (str[nextPos] == '\n')
        nextPos ++;
    }
  }

  if  (trimWhiteSpace)
    result.TrimRight (whiteSpace);

  return  result;
}  /* GetRestOfLine */



KKStr  KKStrParser::GetRestOfStr ()
{
  if  (trimWhiteSpace)
  {
    while  ((nextPos < len)  &&  (strchr (whiteSpace, str[nextPos]) != NULL))
      nextPos++;
  }

  if (nextPos >= len)
    return KKStr::EmptyStr ();

  KKStr  result (1 + len - nextPos);
  while  (nextPos < len)  
  {
    result.Append (str[nextPos]);
    nextPos++;
  }

  if  (trimWhiteSpace)
    result.TrimRight (whiteSpace);

  return  result;
}  /* GetRestOfStr */



void  KKStrParser::Reset ()
{
  nextPos = 0;
}




KKStr  KKStrParser::SubStrPart (kkuint32  firstChar,
                                kkuint32  lastChar
                               )  const
{
  if  (lastChar < firstChar)
    return KKStr::EmptyStr ();

  kkuint32  subStrLen = (1 + lastChar - firstChar);
  KKStr  result (subStrLen + 1);

  if  (lastChar >= len)
    lastChar = len - 1;

  kkuint32  idx = 0;
  for  (idx = firstChar;  idx <= lastChar;  idx++)
    result.Append (str[idx]);

  return  result;
}
