// KKBaseTests.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "FirstIncludes.h"
#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <math.h>
#include "MemoryDebug.h"
using namespace std;

#include "DateTimeTest.h"
#include "KKQueueTest.h"
#include "KKHeapTest.h"
#include "KKStrTest.h"
#include "KKTest.h"
#include "OptionTest.h"
using namespace KKBaseTest;

  int main()
  {
    KKQueue<KKTest> tests;
    tests.PushOnBack (new KKHeapTest   ());
    tests.PushOnBack (new DateTimeTest ());
    tests.PushOnBack (new OptionTest   ());
    tests.PushOnBack (new KKQueueTest  ());
    tests.PushOnBack (new KKStrTest    ());

    kkuint32 failedCount = 0;

    for (auto test: tests)
    {
      test->RunTests ();
      failedCount += test->FailedCount ();
    }

    return failedCount;
  }

  // Run program: Ctrl + F5 or Debug > Start Without Debugging menu
  // Debug program: F5 or Debug > Start Debugging menu

  // Tips for Getting Started: 
  //   1. Use the Solution Explorer window to add/manage files
  //   2. Use the Team Explorer window to connect to source control
  //   3. Use the Output window to see build output and other messages
  //   4. Use the Error List window to view errors
  //   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
  //   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
