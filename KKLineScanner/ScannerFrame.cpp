#include "FirstIncludes.h"
#include <stdio.h>
#include "MemoryDebug.h"
//using  namespace  std;


#include "KKBaseTypes.h"
using namespace  KKB;

#include "ScannerClock.h"
#include "ScannerFrame.h"
using namespace  KKLSC;


ScannerFrame::ScannerFrame (ScannerClockPtr _clock,
                            kkint32         _scanLinesPerFrame,
                            kkint32         _pixelsPerScanLine
                           ):
  
  clock         (_clock),
  frameNum      (-1),
  height        (_scanLinesPerFrame),
  scanLines     (NULL),
  scanLineFirst (-1),
  scanLineLast  (-1),
  time          (0),
  width         (_pixelsPerScanLine)

{
  clock = _clock;
  scanLines = AllocateFrameArray ();
}


ScannerFrame::~ScannerFrame ()
{
  if  (scanLines)
  {
    for  (kkint32  x = 0;  x < height;  ++x)
    {
      delete  scanLines[x];
      scanLines[x] = NULL;
    }
  }

  delete  scanLines;
  scanLines = NULL;
}



uchar**  ScannerFrame::AllocateFrameArray ()
{
  uchar** a = new uchar*[height];
  for  (kkint32 r = 0;  r < height;  r++)
    a[r] = new uchar[width];
  return a;
}



uchar**  ScannerFrame::ScanLines ()
{
  time = clock->Time ();
  return scanLines;
} 



void  ScannerFrame::ScanLines (uchar** _scanLines)
{
  time = clock->Time ();
  scanLines = _scanLines;
}
