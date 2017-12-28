#include "FirstIncludes.h"
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "MemoryDebug.h"
using namespace std;


#include "KKBaseTypes.h"
using namespace  KKB;

#include "ScannerFileSipper3.h"
using namespace KKLSC;



ScannerFileSipper3::ScannerFileSipper3 ():
      bytesWritten      (0),
      bytesLastScanLine (0),
      scanLinesWritten  (0)
{
}

      
      
      
ScannerFileSipper3::~ScannerFileSipper3 ()
{

}



void  ScannerFileSipper3::WriteWholeScanLine (ostream& o,
                                              uchar*   line,
                                              kkint32  len
                                            )
{
  bytesLastScanLine = 0;
  kkuint32 lenUint = (kkuint32)len;
  o.write ((char*)&lenUint, sizeof (lenUint));
  bytesWritten += sizeof (lenUint);
  bytesLastScanLine = bytesLastScanLine + (kkint32)sizeof (lenUint);
  o.write ((char*)line, lenUint);
  bytesWritten +=  lenUint;
  bytesLastScanLine +=   lenUint;
  scanLinesWritten++;
}  /* CompressWholeLine */



