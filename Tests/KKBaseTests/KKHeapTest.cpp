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

#include "KKHeapTest.h"


namespace  KKBaseTest
{
  KKHeapTest::KKHeapTest ()
  {
  }



  KKHeapTest::~KKHeapTest ()
  {
  }



  bool  KKHeapTest::RunTests ()
  {
    TestHeap  h = BuildHeap();
    h.Push (TestCase (1));
    h.Push (TestCase (-1));
    h.Push (TestCase (10));
    h.Push (TestCase (-9));
    h.Push (TestCase (0));
    h.Push (TestCase (-11));
    h.Push (TestCase (5));

    auto n = h.Pop();
    cout << n.x << std::endl;

    return true;
  }



  KKHeapTest::TestHeap  KKHeapTest::BuildHeap ()
  {
    TestHeap  h([](const TestCase& left, const TestCase& right) -> bool {return left.x < right.x;});
    return h;
  }
}
