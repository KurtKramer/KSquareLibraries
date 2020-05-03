#pragma once
#include "KKTest.h"
#include "KKHeap.h"

namespace  KKBaseTest
{
  class KKHeapTest : public KKTest
  {
    class TestCase
    {
    public:
      TestCase (int _x) : x (_x) {}
      int x;
    };

    typedef  KKHeap<TestCase>  TestHeap;

  public:
    KKHeapTest ();

    virtual ~KKHeapTest ();

    virtual const char*  TestName () const { return "KKQueue"; }

    bool  RunTests () override;

  private:
    TestHeap  BuildHeap();


  };
}
