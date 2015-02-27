/* KKException.cpp -- Base class to be used for exception handling.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef  _KKEXCEPTION_
#define  _KKEXCEPTION_
//************************************************************************************
//*                                                                                  *
//*  Developed By:  Kurt A. Kramer                                                   *
//*                                                                                  *
//*  Date:          Early 90's                                                       *
//*                                                                                  *
//************************************************************************************
//*  KKStr class and string manipulation routines.                                  *
//************************************************************************************

#include  <ostream>
#include  <string>

#ifdef  WIN32
#else
#define  __cdecl  
#endif

#include "BasicTypes.h"
#include "KKQueue.h"
#include "Str.h"

#define  EnterChar   13
#define  EscapeChar  27


namespace KKU
{

  class  KKException: public std::exception
  {
  public:
    KKException ();
    
    KKException (const KKException&  _exception);
    
    KKException (const KKStr&   _exceptionStr);

    KKException (const KKStr&           _exceptionStr,
                 const std::exception&  _innerException
                );

    KKException (const char*  _exceptionStr);

    //KKException (const char*  &_exceptionStr);
    
    KKException (const char*            _exceptionStr,
                 const std::exception&  _innerException
                );

    virtual  ~KKException ()  throw ();

    virtual  const KKStr&  ToString ()  const;

    virtual const char*  what () const throw ();


  private:
    KKStr  exceptionStr;
  };  /* KKException */

}  /* namespace KKB; */

#endif
