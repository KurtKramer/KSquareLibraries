/* TokenBuffer.h -- Class to Manage Token Parsing
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef _TOKENBUFFER_
#define _TOKENBUFFER_

#include "KKStr.h"

namespace  KKB
{
  class  TokenBuffer
  {
  public:
    TokenBuffer ();
    virtual  
      ~TokenBuffer ();

    virtual char  GetNextChar   () = 0;
    virtual bool  EndOfFile     () = 0;
    virtual char  PeekNextChar  () = 0;
    virtual void  UnGetNextChar () = 0;
    virtual bool  Valid         () = 0;

  private:
  };  /* TokenBufer */


  typedef  TokenBuffer*  TokenBufferPtr;




  class  TokenBufferStr: public TokenBuffer
  {
  public:
    TokenBufferStr (const KKStr&  _buff);
  
    virtual
      ~TokenBufferStr ();

    virtual bool  EndOfFile     () override;
    virtual char  GetNextChar   () override;
    virtual char  PeekNextChar  () override;
    virtual void  UnGetNextChar () override;
    virtual bool  Valid         () override;

  private:
    KKStr     buff;
    kkuint32  nextCharPos;
  };



  class  TokenBufferStream: public TokenBuffer
  {
  public:
    TokenBufferStream (std::istream*  _in);

    TokenBufferStream (const KKStr&  _fileName);

    virtual
      ~TokenBufferStream ();

    virtual bool  EndOfFile     () override;
    virtual char  GetNextChar   () override;
    virtual void  UnGetNextChar () override;
    virtual char  PeekNextChar  () override;
    virtual bool  Valid         () override;

  private:
    bool            endOfFile;
    KKStr           fileName;
    std::ifstream*  fileStream;
    std::istream*   in;
  };  /* TokenBufferStream */

  
  typedef  TokenBufferStream*  TokenBufferStreamPtr;

}  /* KKB */


#endif
