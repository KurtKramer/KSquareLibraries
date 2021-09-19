#include "FirstIncludes.h"
#include <stdio.h>
#include <iostream>
#include "MemoryDebug.h"


#include  "KKBaseTypes.h"
using namespace  KKB;

#include "ScannerClock.h"
using namespace  KKLSC;


ScannerClock::ScannerClock ():
   time (0)
{
}


kkint32  ScannerClock::Time ()
{
  kkint32 x = time;
  time++;
  return  x;
}



void   ScannerClock::Time (kkint32 _time)
{
  time = _time;
}
