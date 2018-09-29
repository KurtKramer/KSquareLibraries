#pragma once
#include "KKQueue.h"
using namespace KKB;

#include "KKTest.h"
class KKQueueTest : public KKTest
{
  class TestClass
  {
  public:
    TestClass(int _x): x(_x) {}
    int x;
  };

  typedef  KKQueue<TestClass>  TestClassList;


public:
  KKQueueTest ();
  virtual ~KKQueueTest ();

  bool  RunTest () override;


private:
  TestClassList*  BuildTestList(kkuint32 size) const;
  bool  TestFindTheKthElement();
};

