#include "FirstIncludes.h"
#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <math.h>
#include "MemoryDebug.h"
using namespace std;

#include "KKBaseTypes.h"
#include "KKStr.h"
#include "Option.h"
using namespace KKB;

#include "KKQueueTest.h"


namespace  KKTest
{
  KKQueueTest::KKQueueTest ()
  {
  }



  KKQueueTest::~KKQueueTest ()
  {
  }



  bool  KKQueueTest::FindTheKthElement()
  {
    auto comp = [](TestClass* x, TestClass* y) -> bool { return x->x < y->x; };
    auto testData = BuildTestList(20);

    for (kkint32 zed = 0; zed < 20;  ++zed)
    {
      auto indexOfElement = testData->FindTheKthElementIdx(zed, comp);

      std::cout << zed << "\t" << indexOfElement << "\t" << (*testData)[indexOfElement.value ()].x << std::endl;

      int nthValue = (*testData)[indexOfElement.value ()].x;

      Assert (nthValue == zed, "KKQueueTest::FindTheKthElement  Expected: " + StrFromInt32(zed) + " got: " + StrFromInt32(nthValue));
    }
    return true;
  }



  bool  KKQueueTest::RunTests ()
  {
    FindTheKthElement ();
    return true;
  }



  KKQueueTest::TestClassList*  KKQueueTest::BuildTestList(kkuint32 size) const
  {
    auto list = new TestClassList (true);
    for  (kkuint32 x = 0;  x < size;  ++x)
      list->PushOnBack (new TestClass (size - x - 1));
    list->RandomizeOrder ();
    return list;
  }
}