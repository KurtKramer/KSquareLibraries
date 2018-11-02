#pragma once
#include "KKQueue.h"
using namespace KKB;

#include "KKTest.h"

namespace  KKBaseTest
{
  class KKQueueTest : public KKTest
  {
    class TestCase
    {
    public:
      TestCase(int _x): x(_x) {}
      int x;
    };

    typedef  KKQueue<TestCase>  TestCaseList;

  public:
    KKQueueTest ();

    virtual ~KKQueueTest ();

    virtual const char*  TestName () const {return "KKQueue";}

    bool  RunTests () override;

  private:
    TestCaseList*  BuildTestList (kkuint32 size) const;
    bool  FindTheKthElement ();
  };
}
