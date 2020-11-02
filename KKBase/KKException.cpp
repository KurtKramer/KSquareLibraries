/* KKException.cpp -- Base class to be used for exception handling.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */


#include "FirstIncludes.h"

WarningsLowered()

#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "MemoryDebug.h"

WarningsRestored()

using namespace std;

#include "KKException.h"
#include "KKStr.h"



using namespace KKB;



KKException::KKException ():
  std::exception (),
  exceptionStr (NULL)
{
}



KKException::KKException (const KKException&  _exception):
  std::exception (),
  exceptionStr (new KKStr(*_exception.exceptionStr))
{
}



KKException::KKException (const char*  _exceptionStr):
  std::exception (),
  exceptionStr (new KKStr (_exceptionStr))
{
}



KKException::KKException (const KKStr&  _exceptionStr):
  std::exception (),
  exceptionStr (new KKStr (_exceptionStr))
{
}



KKException::KKException (const KKStr&           _exceptionStr,
                          const std::exception&  _innerException
                         ):
  std::exception (_innerException),
  exceptionStr (new KKStr ())
{
  *exceptionStr << _exceptionStr << endl
                << _innerException.what ();
}



KKException::KKException (const char*            _exceptionStr,
                          const std::exception&  _innerException
                         ):
  std::exception (_innerException),
  exceptionStr (new KKStr (_exceptionStr))
{
}



KKException::KKException (const char*         _exceptionStr,
                          const KKException&  _innerException
                         ):
  std::exception (_innerException),
  exceptionStr (new KKStr(_exceptionStr))
{
}



KKException::KKException (const KKStr&        _exceptionStr,
                          const KKException&  _innerException
                         ):
  std::exception (_innerException),
  exceptionStr (new KKStr (_exceptionStr))
{
}



KKException::KKException (const char*   _fileName,
                          kkuint32      _lineNum,
                          const KKStr&  _exceptionStr
                         ):
    std::exception (),
    exceptionStr (new KKStr ())
{
  *exceptionStr << "Exception " << _fileName << ":" << _lineNum << " " <<  _exceptionStr;
}
       


KKException::~KKException ()  throw ()
{
  delete exceptionStr;
  exceptionStr = NULL;
}



const KKStr&  KKException::ToString ()  const
{
  return  *exceptionStr;
}



const char*  KKException::what () const throw ()
{
  return  exceptionStr->Str ();
}
