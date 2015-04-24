/* XmlTokenizer.h -- Class to Manage Token Parsing
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */

#ifndef _XMLTOKENIZER_
#define _XMLTOKENIZER_
/**
 *@class  KKB::XmlTokenizer
 *@brief  Class is meant to break down a stream into a set of logical tokens.
 *@author Kurt Kramer
 *@details  This class was originally created while taking Non Linear Systems. It breaks up a source 
 *          KKStr or text file into logical tokens. You can create your own source of characters by
 *          creating a Class derived from KKB::TokenBuffer.
 */


#include <vector>
#include "TokenBuffer.h"

namespace  KKB
{
  class  XmlTokenizer
  {
  public:
    XmlTokenizer (TokenBufferPtr _in);

    XmlTokenizer (const KKStr&  _str);

    XmlTokenizer (const KKStr&  _fileName,
                  bool&         _fileOpened
                 );

    ~XmlTokenizer ();


    bool  EndOfFile ();

    KKStrPtr   GetNextToken ();

    /** 
     *@brief  Returns a list of tokens up to and including the first occurrence of 'delToken'.
     *@details Caller will take ownership of the returned tokens, and be responsible for
     *         deleting them.
     */
    KKStrListPtr   GetNextTokens (const KKStr& delToken);  

    KKStrConstPtr  Peek (kkuint32 idx);

    void  PushTokenOnFront (KKStrPtr  t);

    KKStrConstPtr  operator[](kkuint32 idx); /**< Returns pointers to following Tokens in the stream where idx==0 indicates the next token. */


  private:
    KKStrPtr   GetNextTokenRaw ();
    char       GetNextChar ();

    void       Initialize ();
    KKStrPtr   ProcessTagToken ();
    KKStrPtr   ProcessBodyToken ();
    void       ProcessAmpersand ();


    void       ReadInNextLogicalToken ();
    bool       WhiteSpaceChar (char c)  const;

    char       LookUpEntity (const KKStr&  entityName)  const;

    bool             atEndOfFile;
    TokenBufferPtr   in;
    bool             secondCharAtEndOfFile;

    kkint32          tokenListLen;
    KKStrList        tokenList;      /**< @brief Will contain a fixed list of future tokens to read.
                                      * As end of stream is approached will fill with end of file
                                      * Tokens as a flag.
                                      */

    bool             weOwnTokenBuffer; /**< @brief Set to true indicates that we need to call the destructor on the TokenBuffer 'in' that we are processing. */

    char             firstChar;
    char             secondChar;

    map<KKStr,char>  entityMap;       /**< @brief Used to maintain a list of valid entities and their respective replacement characters. THese are
                                       * the name of the xml escape characters,  ex: "quot" = '"',  "lt" = '<'. These are the escape sequences that
                                       * start with a ampersand(&) and end with a semicolon.
                                       */

  };  /* XmlTokenizer */


  typedef  XmlTokenizer*  XmlTokenizerPtr;
}

#endif
