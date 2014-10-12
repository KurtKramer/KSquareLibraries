#if  !defined(_SCANNERFRAME_)
#define  _SCANNERFRAME_


#include  "ScannerClock.h"


namespace KKLSC 
{
  /**
   *@brief Used to buffer one Frame of Scanner Data for use by 'ScannerFileBuffered'.
   *@details Each access to 'ScanLines' will cause the 'time' variable to increment by one.  This will
   * allow the 'ScannerFleBufered' class know how old its Buffered frames are.  With this knowledge it
   * will be able to keep the more currently accessed frames in memory.
   */
  class  ScannerFrame
  {
  public:
    typedef  ScannerFrame*  ScannerFramePtr;


    /**
     *@brief  Construct a frame with the dimensions (Height = _scanLinesPerFrame) and (Width = _pixelsPerScanLine).
     *@param[in] _clock  A ScannerClock instance that is shared by all instances of 'ScannerFrame' that are used by the same instance of 'ScannerFleBuffered'.
     *@param[in] _scanLinesPerFrame  Height of the frame.
     *@param[in] _pixelsPerScanLine  Width of the frame.
     */
    ScannerFrame (ScannerClockPtr _clock,
                  int32           _scanLinesPerFrame,
                  int32           _pixelsPerScanLine
                 );

    ~ScannerFrame ();

    int32    FrameNum      () const  {return frameNum;}
    uchar**  ScanLines     ();                                  /**< Will return scan-lines and increment 'time' by one. */
    int32    ScanLineFirst () const  {return scanLineFirst;}
    int32    ScanLineLast  () const  {return scanLineLast;}
    int32    Time          () const  {return time;}             /**< The higher 'time' the more recently this frame has been accessed. */


    void  FrameNum      (int32   _frameNum)      {frameNum      = _frameNum;}
    void  ScanLines     (uchar** _scanLines);
    void  ScanLineFirst (int32   _scanLineFirst) {scanLineFirst = _scanLineFirst;}
    void  ScanLineLast  (int32   _scanLineLast)  {scanLineLast  = _scanLineLast;}
    void  Time          (int32   _time)          {time          = _time;}


  private:
    uchar**  AllocateFrameArray ();

    ScannerClockPtr   clock;
    int32             frameNum;
    int32             height;
    uchar**           scanLines;
    int32             scanLineFirst;
    int32             scanLineLast;
    int32             time;
    int32             width;
  };  /* ScannerFrame */

  typedef  ScannerFrame::ScannerFramePtr  ScannerFramePtr;
}

#endif
