/* Tokenizer.h -- Class to Manage Token Parsing
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */

#ifndef _TOKENIZER_
#define _TOKENIZER_
/**
 *@class  KKB::Tokenizer
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
  class  Tokenizer
  {
  public:
    static
      Tokenizer* OpenFile (const KKStr&  _fileName);

    Tokenizer (TokenBufferPtr _in);

    Tokenizer (const KKStr&  _str);

    Tokenizer (const KKStr&  _fileName,
               bool&         _fileOpened
              );

    ~Tokenizer ();
    
    void  DefineOperatorChars (char const * _operatorChars);

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
    bool       DelimiterChar (char c)  const;
    bool       OperatorChar  (char c)  const;
    KKStrPtr   GetNextTokenRaw ();
    char       GetNextChar ();

    void       Initialize ();
    KKStrPtr   ProcessOperatorToken ();
    KKStrPtr   ProcessStringToken  (char strDelChar);
    KKStrPtr   ProcessFieldToken  ();
    void       ReadInNextLogicalToken ();
    bool       WhiteSpaceChar (char c)  const;

    bool             atEndOfFile;
    TokenBufferPtr   in;
    bool             secondCharAtEndOfFile;

    char const *     operatorChars;

    KKStrList        tokenList;      /**< @brief Will contain a fixed list of future tokens to read.
                                      * As end of stream is approached will fill with end of file
                                      * Tokens as a flag.
                                      */

    bool             weOwnTokenBuffer; /**< @brief Set to true indicates that we need to call the destructor on the TokenBuffer 'in' that we are processing. */

    char             firstChar;
    char             secondChar;
  };  /* Tokenizer */


  typedef  Tokenizer*  TokenizerPtr;
}

#endif
