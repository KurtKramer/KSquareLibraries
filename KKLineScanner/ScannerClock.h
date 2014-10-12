#if  !defined(_SCANNERCLOCK_)
#define  _SCANNERCLOCK_


#include  "KKBaseTypes.h"
using namespace  KKB;

namespace KKLSC 
{

  /**
   *@class  ScannerClock
   *@brief Used by Scanner file routines to keep track of most current buffers.
   *@details 
   *@code
   *****************************************************************************
   ** Used by SscannerFileBuffered to keep track of age if Buffered Frames     *
   ** Every time a buffered frame is accessed it will update its TimeStamp     *
   ** from a global instance of 'UmiScannerClock'                              *
   *****************************************************************************
   *@endcode
  */
  class  ScannerClock
  {
  public:
    typedef  ScannerClock*  ScannerClockPtr;

    ScannerClock ();

    /** @brief  Returns the current value of 'time' them increments by 1. */
    int32  Time ();

    void  Time (int32 _time);

  private:
    int32  time;
  };  /* ScannerClock */

  typedef  ScannerClock::ScannerClockPtr  ScannerClockPtr;

}  /* KKLSC */


#endif



