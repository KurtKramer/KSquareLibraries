#include "FirstIncludes.h"
#include <cstring>
#include <ctype.h>
#include <exception>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <string.h>
#include "MemoryDebug.h"
using namespace std;

#include "KKException.h"
#include "KKStr.h"
#include "Option.h"
using namespace KKB;

namespace KKB
{
  void  ValidateValidUint32 (kkint64 newValue)
  {
    KKCheck(newValue >= 0, "OptionUInt32  result: " << newValue << " is nagative!")
    KKCheck(newValue <= std::numeric_limits<uint32_t>::max (), "OptionUInt32  result: " << newValue << "  exceeds capacity of uint32!")
  }
}
