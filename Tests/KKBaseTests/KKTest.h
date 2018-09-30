#pragma once

#include "KKStr.h"


namespace  KKTest
{
  class KKTest
  {
  public:

    class  TestResult
    {
    public:
      TestResult (bool _passed, const KKStr& _testName): passed(_passed), testName(_testName) {}

      bool  passed;
      KKStr testName;
    };

    typedef  KKQueue<TestResult>  TestResultList;
    
    KKTest ();

    virtual ~KKTest ();

    kkuint32  FailedCount () { return failedCount; }
    kkuint32  PassedCount () { return passedCount; }

    virtual bool RunTests() = 0;

    virtual void Assert(bool passed, const KKB::KKStr& testName);


  private:
    kkuint32        failedCount;
    kkuint32        passedCount;
    TestResultList  results;
  };
}

