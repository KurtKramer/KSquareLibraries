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
    Option<kkuint32>  i;

    i = 100u;



    OptionUInt32 x = 5;
    OptionUInt32 y = nullopt;
    Assert (y < (kkuint32)0, "OptionUInt32", "None < 0");

    Assert (!(x < (kkuint32)0), "OptionUInt32", "Optional.value(5) not less than 0");

    Assert (!y, "OptionUInt32", "not (y = None)");

    Assert (!(y > (kkuint32)3), "OptionUInt32", "y = None > 3");

    try
    {
      auto zed = y + 2;
      Assert (false, "OptionUInt32", "y = None + 2 Should throw exception!");
    }
    catch (const std::exception& e)
    {
      KKStr eStr = e.what ();
      Assert (true, "OptionUInt32", "y = None + 2 Should throw exception! " + eStr);
    }

    Assert (x == (kkuint32)5, "OptionUInt32", "x = Optional(5) == 5");

    Assert ((x + 2) == (kkuint32)7, "OptionUInt32", "Optional(5) + 2 == 7!");

    return true;
  }
}
