#if  !defined(_BASETEST_)
#define _BASETEST_

#include "KKStr.h"

namespace KKTest
{
  class TestManager;
  typedef TestManager*  TestManagerPtr;

  class BaseTest
  {
  public:
    BaseTest (TestManagerPtr  _testManager);

    virtual ~BaseTest ();

    virtual void RunTest () = 0;

  protected:
    virtual KKB::KKStr ClassName () const;

    virtual  void  ReportResult (bool success, const KKB::KKStr& testName, const KKB::KKStr&  msg);

  private:
    TestManagerPtr  testManager;
  };
}

#endif
