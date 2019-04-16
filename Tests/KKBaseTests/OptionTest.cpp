#include "FirstIncludes.h"
#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <math.h>
#include "MemoryDebug.h"
using namespace std;

#include "Option.h"
using namespace KKB;

#include "OptionTest.h"

namespace KKBaseTest
{
  OptionTest::OptionTest ()
  {
  }


  OptionTest::~OptionTest ()
  {
  }


  void  OptionTest::AssertAreEqual (const char* expected, const KKStr& found, const KKStr&  testName)
  {
    bool equal = strcmp (expected, found.Str ()) == 0;
    KKStr msg;
    msg << "Expected: " << expected << "\t" << "Found: " << found.Str ();
    Assert (equal, testName, msg);
  }

  bool  OptionTest::RunTests ()
  {
    OptionUInt32 x = 5;
    OptionUInt32 y = {};
    if (y < 0)
      cerr << "It Worked" << std::endl;

    if (y)
      cout << "y is true when it is none!" << endl;

    if (!y)
      cout << "y is false when it is none." << endl;

    if (y > 3)
      cout << "y > 3 when it is not defined!" << endl;

    auto l = y + 2;
    cout << "l = " << l << endl;

    if (y >= 0)
      cout << "y >= 0" << endl;

    if (x == 5)
      cout << "x == 5" << endl;

    if  (x < 0)
      cout << "x < 0  true when it is not" << endl;

    auto z = x + 2;
    cout << "x = " << z << endl;

    return true;
  }

}
