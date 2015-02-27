/* TokenBuffer.h -- Class to Manage Token Parsing
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#ifndef _TOKENBUFFER_
#define _TOKENBUFFER_

#include  "Str.h"

namespace  KKU
{
  class  TokenBuffer
  {
  public:
    TokenBuffer ();
    virtual  
      ~TokenBuffer ();

    virtual 
      bool  Valid () = 0;

    virtual char  GetNextChar () = 0;
    virtual bool  EndOfFile () = 0;

  private:
  };  /* TokenBufer */


  typedef  TokenBuffer*  TokenBufferPtr;




  class  TokenBufferStr: public TokenBuffer
  {
  public:
    TokenBufferStr (const KKStr&  _buff);
  
    virtual
      ~TokenBufferStr ();

    virtual 
      bool  Valid ();

    virtual char  GetNextChar ();
    virtual bool  EndOfFile ();

  private:
    KKStr  buff;

    int32  nextCharPos;
  };


  class  TokenBufferStream: public TokenBuffer
  {
  public:
    TokenBufferStream (istream*  _in);

    TokenBufferStream (const KKStr&  _fileName);

    virtual
      ~TokenBufferStream ();

    virtual 
      bool  Valid ();

    virtual char  GetNextChar ();
    virtual bool  EndOfFile ();

  private:
    bool       endOfFile;
    KKStr      fileName;
    ifstream*  fileStream;
    istream*   in;
  };  /* TokenBufferStream */

  
  typedef  TokenBufferStream*  TokenBufferStreamPtr;

}  /* KKU */


#endif
