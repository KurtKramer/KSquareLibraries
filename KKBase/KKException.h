/* KKException.cpp -- Base class to be used for exception handling.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef  _KKEXCEPTION_
#define  _KKEXCEPTION_

#include <ostream>
#include <string>

#ifdef  WIN32
#else
#if  !defined(__cdecl)
#define  __cdecl
#endif
#endif

#include "KKBaseTypes.h"
#include "KKQueue.h"



#define  EnterChar   13
#define  EscapeChar  27


namespace KKB
{

#if  !defined(_KKStr_Defined_)
  class  KKStr;
  typedef  KKStr*  KKStrPtr;
#endif


  class  KKException: public std::exception
  {
  public:
    KKException ();

    KKException (const char*   _fileName,
                 kkuint32      _lineNum,
                 const KKStr&  _exceptionStr
                );
    
    KKException (const KKException&  _exception);
    
    KKException (const KKStr&  _exceptionStr);

    KKException (const char*  _exceptionStr);

    KKException (const KKStr&           _exceptionStr,
                 const std::exception&  _innerException
                );

    KKException (const char*            _exceptionStr,
                 const std::exception&  _innerException
                );

    KKException (const char*         _exceptionStr,
                 const KKException&  _innerException
                );

    KKException (const KKStr&        _exceptionStr,
                 const KKException&  _innerException
                );

    virtual  ~KKException ()  throw ();

    virtual  const KKStr&  ToString ()  const;

    virtual const char*  what () const throw ();

  private:
    KKStrPtr  exceptionStr;
  };  /* KKException */

#define KKCheck(condition, errMsg)  if  (!(condition))  {    \
  std::stringstream  errMsgStream;                           \
  errMsgStream << errMsg;                                    \
  auto errMsgString = errMsgStream.str ();                   \
  std::cerr << errMsgString << std::endl;                    \
  throw KKB::KKException (__FILE__, __LINE__, errMsgString); \
  }
}  /* namespace KKB; */

#endif
