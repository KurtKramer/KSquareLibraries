#if !defined(_SCANNERFILESIPPER3)
#define  _SCANNERFILESIPPER3

#include  "KKBaseTypes.h"
using namespace KKB;

namespace  KKLSC
{

  /**
   *@class ScannerFileSipper3  ScannerFileSipper3.h
   *@brief  Used to construct Sipper3 data record stream from scan lines.
   *@details  This class is meant to construct a stream of Sipper3 Records for
   * a single Scan line.  You add all the pixels from a single scan line by 
   * calling 'AddPixel' for each pixel in the scan line.  When you are done 
   * Adding all the pixels in a single scan line you call 'Write'  to write
   * all the Sipper3 buffer records to disk.  The input range of pixel values
   * is '0 '- thru '255' where '0' is the background.  'AddPixel' will
   * convert into 3 bit grayscale by dividing by 32.
   */
  class  ScannerFileSipper3
  {
  public:
    ScannerFileSipper3 ();

    ~ScannerFileSipper3 ();

    kkint64  BytesWritten      ()  const  {return bytesWritten;}
    int32    BytesLastScanLine ()  const  {return bytesLastScanLine;}
    int32    ScanLinesWritten  ()  const  {return scanLinesWritten;}

    void  WriteWholeScanLine (ostream&  o,
                              uchar*    line,
                              int32     len
                             );

  private:
    //  The two variables 'rawPixelsBuffUsed' and 'zerosInARow' specify whether Back Ground Run Length data or
    //  raw data is currently be added.  Only one of them can be greater than 0.
    kkint64   bytesWritten;
    int32     bytesLastScanLine;
    int32     scanLinesWritten;
  };  /* ScannerFileSipper3 */

  typedef  ScannerFileSipper3*  ScannerFileSipper3Ptr;
}



#endif
