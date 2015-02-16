/* KKStrParser.cpp -- Class used to parse string into tokens.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
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
#include "BasicTypes.h"


using namespace std;
using namespace KKU;


//#define  KKDEBUG

#include  "DateTime.h"
#include  "Str.h"


#include "KKStrParser.h"



KKStrParser::KKStrParser (const KKStrParser&  _strParser):
    len            (strlen (_strParser.str)),
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
len            ((KKStr::KKStrInt)strlen (_str)),
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



/**
 @brief   After this call all leading and trailing whitespace will be trimmed from tokens.
 */
void  KKStrParser::TrimWhiteSpace (const char*  _whiteSpace)
{
  delete  whiteSpace;  
  whiteSpace = NULL;

  trimWhiteSpace = true;
  if  (_whiteSpace)
    whiteSpace = STRDUP (_whiteSpace);
  else
    whiteSpace = STRDUP (" ");
}



void  KKStrParser::SkipWhiteSpace (const char*  whiteSpace)
{
  while  ((nextPos < len)  &&  (strchr (whiteSpace, str[nextPos]) != NULL))
  {
    nextPos++;
  }
}  /* SkipWhiteSpce */



VectorKKStr  KKStrParser::Split (const char* delStr)
{
  VectorKKStr  tokens;
  while  (MoreTokens ())
   tokens.push_back (this->GetNextToken (delStr));
  return  tokens;
}




/**
 *@brief  Extract next Token from string, each delimiter will separate tokens.
 *@details Removes next Token from string. The token will be terminated by end of 
 * string or the first occurrence of a delimiter character. If no more tokens left
 * will return a Empty KKStr.
 *@param[in]  delStr List of delimiting characters.
 *@return  Extracted Token.
 */
KKStr  KKStrParser::GetNextToken (const char* delStr)
{
  if  (trimWhiteSpace)
  {
    while  ((nextPos < len)  &&  (strchr (whiteSpace, str[nextPos]) != NULL))
      nextPos++;
  }

  if (nextPos >= len)
    return KKStr::EmptyStr ();

  uint32  startPos = nextPos;
  uint32  endPos   = startPos;

  // scan until end of string or next delimiter
  while  ((endPos < len)  &&  (strchr (delStr, str[endPos]) == NULL))
    endPos++;

  // At this point endPos is either on the next delimiter or past end of str.
  if  (endPos >= len)
    nextPos = len;
  else
    nextPos = endPos + 1;

  endPos--;  // Move endPos back to last character in token.

  if  (trimWhiteSpace)
  {
    while  ((endPos >= startPos)  &&  (strchr (whiteSpace, str[endPos]) != NULL))
      endPos--;
  }

  return KKStr (str, startPos, endPos);
}  /* ExtractToken */



char  KKStrParser::GetNextTokenChar (const char* delStr)
{
  KKStr  nextToken = GetNextToken (delStr);
  char nextTokenChar = nextToken[0];
  return  nextTokenChar;
}


KKU::DateTime  KKStrParser::GetNextTokenDateTime (const char* delStr)
{
  return  KKU::DateTime (GetNextToken (delStr));
}



int32  KKStrParser::GetNextTokenInt (const char* delStr)
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



KKU::uint32  KKStrParser::GetNextTokenUint (const char* delStr)
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




KKStr  KKStrParser::SubStrPart (uint32  firstChar,
                                uint32  lastChar
                               )  const
{
  if  (lastChar < firstChar)
    return KKStr::EmptyStr ();

  uint32  subStrLen = (1 + lastChar - firstChar);
  KKStr  result (subStrLen + 1);

  if  (lastChar >= len)
    lastChar = len - 1;

  uint32  idx = 0;
  for  (idx = firstChar;  idx <= lastChar;  idx++)
    result.Append (str[idx]);

  return  result;
}
