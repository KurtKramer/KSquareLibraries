#include "FirstIncludes.h"
#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <math.h>
#include "MemoryDebug.h"
using namespace std;

#include "KKStr.h"
using namespace KKB;


#include "KKTest.h"

namespace  KKTest
{

  KKTest::KKTest ()
  {
  }



  KKTest::~KKTest ()
  {
  }



  void KKTest::Assert (bool passed, const KKStr& testName)
  {
    cout << "Test: " << testName << "\t" << (passed ? "Passed" : "***FAILED***") << endl;
    results.PushOnBack (new TestResult(passed, testName));
    if  (passed)
      ++passedCount;
    else
      ++failedCount;
  }
}