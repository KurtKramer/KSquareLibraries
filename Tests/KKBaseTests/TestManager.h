#if  !defined(_TESTMANAGER_)
#define _TESTMANAGER_

#include "KKStr.h"

namespace KKTest
{
  class TestManager
  {
  public:
    TestManager ();
    virtual ~TestManager ();

    virtual void  ReportResult (bool success, const KKB::KKStr& testName, const KKB::KKStr&  msg);
  };
}

#endif

