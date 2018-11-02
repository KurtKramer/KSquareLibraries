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

#include "KKStrTest.h"

namespace KKBaseTest
{

  KKStrTest::KKStrTest ()
  {
  }



  KKStrTest::~KKStrTest ()
  {
  }



  bool  KKStrTest::RunTests ()
  {
    ExtractQuotedStr ();
    return true;
  }



  void  KKStrTest::AssertAreEqual (const char* expected,  const KKStr& found,  const KKStr&  testName)
  {
    bool equal = strcmp(expected, found.Str ()) == 0;
    KKStr msg;
    msg << "Expected: " << expected << "\t" << "Found: " << found.Str ();
    Assert(equal, testName, msg);
  }



  bool  KKStrTest::ExtractQuotedStr ()
  {
    KKStr s = "TokenOne, Token;Two,'Token Three',\"\\\"Token Four\\\"\",Token\\tFive";

    auto t1 = s.ExtractQuotedStr (",", true);
    AssertAreEqual ("TokenOne", t1, "ExtractQuotedStr");

    auto t2 = s.ExtractQuotedStr (",", true);
    AssertAreEqual (" Token;Two", t2, "ExtractQuotedStr");

    auto t3 = s.ExtractQuotedStr (", ", true);
    AssertAreEqual ("Token Three", t3, "ExtractQuotedStr using single(') quote");

    auto t4 = s.ExtractQuotedStr (",", true);
    AssertAreEqual ("\"Token Four\"", t4, "ExtractQuotedStr Decode Escape Characters");

    auto t5 = s.ExtractQuotedStr (",", false);
    AssertAreEqual ("\"Token\\tFive\"", t5, "ExtractQuotedStr Don't Decode Escape Characters");

    return true;
  }
}