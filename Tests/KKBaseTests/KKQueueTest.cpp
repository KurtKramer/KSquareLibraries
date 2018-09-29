#include "FirstIncludes.h"
#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <math.h>
#include "MemoryDebug.h"
using namespace std;

#include "KKBaseTypes.h"
using namespace KKB;

#include "KKQueueTest.h"



KKQueueTest::KKQueueTest ()
{
}



KKQueueTest::~KKQueueTest ()
{
}


bool  KKQueueTest::TestFindTheKthElement()
{
  auto comp = [](TestClass* x, TestClass* y) -> bool { return x->x < y->x; };
  auto testData = BuildTestList(20);
  auto indexOf3rdElement = testData->FindTheKthElement(3, comp);
  return true;
}


bool  KKQueueTest::RunTest ()
{
  TestFindTheKthElement ();
  return true;
}



KKQueueTest::TestClassList*  KKQueueTest::BuildTestList(kkuint32 size) const
{
  auto list = new TestClassList (true);
  for  (kkuint32 x = 0;  x < size;  ++x)
    list->PushOnBack (new TestClass (size - x));
  list->RandomizeOrder ();
  return list;
}