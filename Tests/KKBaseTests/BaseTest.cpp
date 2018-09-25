


#include <iostream>

using namespace std;

#include "BaseTest.h"
#include "TestManager.h"

namespace KKTest
{

  BaseTest::BaseTest (TestManagerPtr  _testManager) :
    testManager (_testManager)
  {
  }



  BaseTest::~BaseTest ()
  {
    testManager = NULL;
  }

  

  KKB::KKStr BaseTest::ClassName () const
  {
    return "Not-Defined";
  }



  void  BaseTest::ReportResult (bool success, const KKB::KKStr& testName, const KKB::KKStr&  msg)
  {
    cout << ClassName () << "\t" << testName << "\t" << (success ? "Pased" : "Failed") << endl;

  }
}
