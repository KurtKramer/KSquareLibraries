#pragma once
#include "KKTest.h"
namespace KKBaseTest
{
  class OptionTest : public KKTest
  {
  public:
    OptionTest ();
    virtual ~OptionTest ();

    virtual const char*  TestName () const { return "Option"; }

    virtual bool  RunTests ();

  private:
    void  AssertAreEqual (const char* expected, const KKStr& found, const KKStr& testName);

  };
}