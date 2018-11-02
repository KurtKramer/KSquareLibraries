#pragma once
#include "KKTest.h"

namespace KKBaseTest
{
  class KKStrTest: public KKTest
  {
  public:
    KKStrTest ();
    virtual ~KKStrTest ();

    virtual const char*  TestName () const {return "KKStr";}

    virtual bool  RunTests ();

  private:
    void  AssertAreEqual (const char* expected,  const KKStr& found,  const KKStr& testName);
  
    bool  ExtractQuotedStr ();
  };
}
