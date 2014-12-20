/* KKException.cpp -- Base class to be used for exception handling.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"
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
using namespace std;


#include "KKException.h"
using namespace KKB;



KKException::KKException ():
  std::exception ()
{
}


KKException::KKException (const KKException&  _exception):
  std::exception (),
  exceptionStr (_exception.exceptionStr)
{
}


KKException::KKException (const char*  _exceptionStr):
  std::exception (),
  exceptionStr (_exceptionStr)
{
}


KKException::KKException (const KKStr&  _exceptionStr):
  std::exception (),
  exceptionStr (_exceptionStr)
{
}


KKException::KKException (const KKStr&           _exceptionStr,
                          const std::exception&  _innerException
                         ):
  std::exception (_innerException),
  exceptionStr ()
{
  exceptionStr << _exceptionStr << endl
               << _innerException.what ();
}



KKException::KKException (const char*            _exceptionStr,
                          const std::exception&  _innerException
                         )
{
  exceptionStr << _exceptionStr << endl
               << _innerException.what ();
}



KKException::~KKException ()  throw ()
{
}


const KKStr&  KKException::ToString ()  const
{
  return  exceptionStr;
}


const char*  KKException::what () const throw ()
{
  return  exceptionStr.Str ();
}



