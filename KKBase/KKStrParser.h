/* KKStrParser.h -- Class used to parse string into tokens.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef  _KKSTRPARSER_
#define  _KKSTRPARSER_

#include "DateTime.h"
#include "KKStr.h"


namespace KKB 
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
    KKStrParser (KKStr&&  _str);

    ~KKStrParser ();

    char     GetLastChar        ();


    /**
     *@brief  Extract next Token from string, tokens will be separated by delimiter characters. 
     *@details Removes next Token from string. The token will be terminated by end of string or the first 
     * occurrence of a delimiter character. If no more tokens left will return a Empty KKStr. If you want 
     * to remove leading and trailing whitespace characters you need to call the "TrimWhiteSpace" method. 
     *
     * Quoted Strings will be treated differently. If the first character in the token is a quote(") or 
     * apostrophe(') character then the token will include all characters until the matching quote 
     * character. The quote characters will be NOT be included in the token. The next character pointer will 
     * be set to the following delimiter character. The quote characters must match. That is if the first 
     * quote character was (')  then the terminating quote character also be ('). The special escape sequences
     * ("\t", "\n", "\r", "\\", '\"', and "\'" will be translated  into (tab), (line-feed), (carriage-return), 
     * (back-slash), (quote), and (apostrophe).
     *
     *@param[in]  delStr List of delimiter characters.
     *@return  Extracted Token.
     */
    KKStr    GetNextToken       (const char* delStr = "\n\t\r ");

    char     GetNextChar        ();

    char     GetNextTokenChar   (const char* delStr = "\n\t\r ");

    KKB::DateTime  GetNextTokenDateTime (const char* delStr = "\n\t\r ");

    kkint32  GetNextTokenInt    (const char* delStr = "\n\t\r ");

    long     GetNextTokenLong   (const char* delStr = "\n\t\r ");

    double   GetNextTokenDouble (const char* delStr = "\n\t\r ");

    float    GetNextTokenFloat  (const char* delStr = "\n\t\r ");

    kkuint32 GetNextTokenUint   (const char* delStr = "\n\t\r ");

    bool     GetNextTokenBool   (const char* delStr = "\n\t\r ");

    KKStr    GetRestOfLine      ();
 
    KKStr    GetRestOfStr       ();

    char     LastDelimiter      ()  const  {return  lastDelimiter;}

    bool     MoreTokens         ()  const  {return  (nextPos < len);}

    char     PeekLastChar       ()  const;

    char     PeekNextChar       ()  const;


    /**
     *@brief Will use the same rules as "GetNextToken"  to retrieve the next token n the string but will not
     * advance the next character pointer.
     */
    KKStr    PeekNextToken      (const char* delStr = "\n\t\r ")  const;

    void     Reset              ();

    /**
     *@brief  Advances the next-character pointer to the next NOT white space character.
     */
    void     SkipWhiteSpace     (const char*  whiteSpace = " ");

    VectorKKStr  Split (const char* delStr = "\n\t\r ");

    const char*  Str ()  const  {return str;}

    KKStr    SubStrPart (kkuint32  firstChar,
                         kkuint32  lastChar
                        )  const;

    /**
     *@brief  After this call all leading and trailing whitespace will be trimmed from tokens.
     *@details The next character pointer will be advanced to the next NON whitespace character.
     */
    void     TrimWhiteSpace     (const char*  _whiteSpace = " ");

  private:
    char         lastDelimiter;  /**< The last delimiter character encountered when calling "GetNextToken". */
    kkuint32     len;
    kkuint32     nextPos;
    const char*  str;
    bool         trimWhiteSpace;
    bool         weOwnStr;
    char*        whiteSpace;
  };
}  /* KKB*/


#endif
