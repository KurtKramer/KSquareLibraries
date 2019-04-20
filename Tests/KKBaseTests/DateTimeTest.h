#pragma once
#include "KKTest.h"

namespace KKBaseTest
{
  class DateTimeTest: public KKTest
  {
  public:
    DateTimeTest ();
    virtual ~DateTimeTest ();

    virtual const char*  TestName () const {return "DateTime";}

    virtual bool  RunTests ();

  private:
  };
}
