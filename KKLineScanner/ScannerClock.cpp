#include "FirstIncludes.h"
#include <stdio.h>
#include "MemoryDebug.h"


#include  "KKBaseTypes.h"
using namespace  KKB;

#include "ScannerClock.h"
using namespace  KKLSC;


ScannerClock::ScannerClock ():
   time (0)
{
}


int32  ScannerClock::Time ()
{
  int32 x = time;
  time++;
  return  x;
}



void   ScannerClock::Time (int32 _time)
{
  time = _time;
}
