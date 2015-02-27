/* KKStrParser.h -- Class used to parse string into tokens.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#ifndef  _KKSTRPARSER_
#define  _KKSTRPARSER_

#include  "DateTime.h"
#include  "Str.h"


namespace KKU 
{
  /**
   *@class  KKStrParser
   *@brief  Class that manages the extraction of tokens from a String without being destructive to the original string.
   */
  class KKStrParser
  {
  public:
    KKStrParser (const KKStrParser&  _strParser);
    KKStrParser (const char*   _str);
    KKStrParser (const KKStr&  _str);
    ~KKStrParser ();

    KKStr    GetNextToken       (const char* delStr = "\n\t\r ");

    char     GetNextTokenChar   (const char* delStr = "\n\t\r ");

    KKU::DateTime  GetNextTokenDateTime (const char* delStr = "\n\t\r ");

    int32    GetNextTokenInt    (const char* delStr = "\n\t\r ");

    long     GetNextTokenLong   (const char* delStr = "\n\t\r ");

    double   GetNextTokenDouble (const char* delStr = "\n\t\r ");

    float    GetNextTokenFloat  (const char* delStr = "\n\t\r ");

    uint32   GetNextTokenUint   (const char* delStr = "\n\t\r ");

    bool     GetNextTokenBool   (const char* delStr = "\n\t\r ");

    KKStr    GetRestOfLine      ();
 
    KKStr    GetRestOfStr       ();

    bool     MoreTokens ()  const  {return  (nextPos < len);}

    void     Reset              ();

    void     SkipWhiteSpace     (const char*  whiteSpace = " ");

    VectorKKStr  Split (const char* delStr = "\n\t\r ");

    const char*  Str ()  const  {return str;}

    KKStr    SubStrPart (uint32  firstChar,
                         uint32  lastChar
                        )  const;

    void     TrimWhiteSpace     (const char*  _whiteSpace = " ");  /**< @brief After this call all leading and trailing whitespace will be trimmed from tokens.  */

  private:
    uint32       len;
    uint32       nextPos;
    const char*  str;
    bool         trimWhiteSpace;
    char*        whiteSpace;
  };
}  /* KKU*/


#endif
