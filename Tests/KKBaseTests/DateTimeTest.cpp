#include "FirstIncludes.h"
#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <math.h>
#include "MemoryDebug.h"
using namespace std;

#include "DateTime.h"
using namespace KKB;

#include "DateTimeTest.h"

namespace KKBaseTest
{

  DateTimeTest::DateTimeTest ()
  {
  }



  DateTimeTest::~DateTimeTest ()
  {
  }



  bool  DateTimeTest::RunTests ()
  {
    KKB::DateType a ((kkint16)2016, (uchar)2, (uchar)28);
    KKB::DateType b = a + 1;

    Assert (b == DateType (2016, 2, 29), "DateTimeTest", "Adding 1 day to 2016-02-28 gets 2016-02-29 because it is leap year");

    auto deltaDays = DateType (2016, 3, 1) - DateType (2016, 2, 28);
    Assert (deltaDays == 2, "DateTimeTest", "2016/3/1 - 2016/2/28 == 2 because of leap year.");

    DateType c ("2018-2-27");
    Assert (DateType ("2018-2-27") == DateType (2018, 2, 27), "DateTimeTest", "Construct DateType from string(2018-2-27)");
    Assert (DateType ("2018-feb-27") == DateType (2018, 2, 27), "DateTimeTest", "Construct DateType from string(2018-feb-27)");
    Assert (DateType ("2018-October-27") == DateType (2018, 10, 27), "DateTimeTest", "Construct DateType from string(2018-October-27)");
    
    return true;
  }
}