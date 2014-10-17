#if  !defined(_SCANNERFILE4BITENCODED_)
#define  _SCANNERFILE4BITENCODED_

#include  "ScannerFile.h"

namespace  KKLSC
{
  /**
   *@class ScannerFile4BitEncoded  
   *@brief Implements a 4 bit Encoded format.
   *@details This is a very simple implementation of run-length compression
   *         only background pixels (0) will be compressed by groups of 4.
   *         All pixels will be translated from 8 bit to 3 bit.
   *@code
   *  1) Records are or varying length.
   *  2) Each record starts with a 4 bit Op-Code.
   *  3) Records contain either Text or Imagery
   *  4) Imagery records can be eith Run-Length (Same Pixel repeating) or Raw-String(A sequence of pixels).
   *
   *   Summary of Op-Codes:
   *   --------------------------------------------------------------------
   *   0: End-Of-Line:  Indicates that end of Scan-Line reached.
   *   1: Text-Block:
   *   2: InsrumentData
   *   3: Un-Used
   *   4: Run-Length (2 Pixels).
   *   5: Run-Length (3 Pixels).
   *   6: Run-Length (4 Pixels).
   *   7: Run-Length (5 Pixels).
   *   8: Run-Length (6 Pixels).
   *   9: Run-Length (7 Pixels).
   *  10: Run-Length (1 thru 256 Pixels).
   *  11: Raw-String (1 Pixel Length).
   *  12: Raw-String (Even length 2 thru 32).
   *  13: Raw-String (Odd Length 1 thru 513).
   *  14: Un-Used.
   *  15: Un-Used.
   *
   *
   *  Bits
   *  =====
   *  0 - 3:  OP-Code  (Values 0 thru 15)
   *
   *
   *----------------------------------------------------------
   *  Op-Code = 0   end of line
   *  4 - 7:  Ignore
   *
   *
   *----------------------------------------------------------
   *  Op-Code 1:   Test Block.
   *  4 -  4:  End of text block.
   *  5 -  7:  TextBlock Len HighEnd.
   *  8 - 15:  TextBlock Len LowEnd
   *    Text-Block-Len = HighEnd * 256 + LowEnd.
   *  Followed by 'Text-Block-Len' bytes.
   *
   *
   *----------------------------------------------------------
   *  Op-Code 2:  InstrumentData
   *  4 -  7:  InstrumentId
   *  8 - 39:  Scan-Line Number.
   * 40 - 71:  Instrument Data.
   *
   *
   *----------------------------------------------------------
   *  Op-Code = 4 thru 9:  Short-Run-Len
   *                       RunLen = OpCode - 2
   *  4 -  7: Pixel Value being repeated.
   *
   *
   *----------------------------------------------------------
   * Op-Code = 10:   Run-length of range "1" thru "256"
   *                 Since there are no run-lengths of '0';  '0' will decode to Run-Lengrth = 1;
   *  4 -  7: Pixel Value being repeated.
   *  8 - 15: Runlen     0=1, 1=2, 2=3,... 254=255, 255=256
   *
   *
   *----------------------------------------------------------
   * Op-Code = 11:   One Pixel Raw String. 
   *  4 -  7:     PixelValue.
   *
   *
   *----------------------------------------------------------
   * Op-Code = 12   Raw Pixel String of even length ranging from 2 to 32 
   *                because each byte can fit two pixels it is best that Raw String lengths
   *                always be multiples of 2.
   *
   *  4 -  7:     Len;   Num-Raw-Pixels = (Len + 1) * 2
   *              Len  0=(2 raw Pixels), 1=(4 raw pixels), 2=(6 raw pixels),,, 15=(32 raw pixles)
   *              Number of following bytes = (Len + 1).
   *
   *
   *----------------------------------------------------------
   * Op-Code = 13  Odd number of raw pixels up to length of 513 pixels.
   *               We use the other 4 bits in the 1st byte to represent the high order bits of the length
   *               and the 1st 4 bits ibn the second byte for the low order;  that leaves room for one 
   *               pixels in the second byte.  This is why odd length stings are supported.
   *
   *   8 -  7:  LenHigh             String Length = 1 + 2 * (LenHigh * 16 + LenLow);
   *   8 - 11:  LenLow
   *  12 - 15:  First Raw Pixel.    Will be followed by ((LenHigh * 16 + LenLow) bytes with two pixels 
   *                                in each one.
   *
   *           Translation Tables
   *  ======================================
   *  4     8                              4
   *  Bit   Bit           8-Bit  Range   Bit 
   *  ===   ===           ============   ===
   *  0       0           (  0 -  15):     0
   *  1      17           ( 16 -  31):     1
   *  2      34           ( 32 -  47):     2
   *  3      51           ( 48 -  63):     3
   *  4      68           ( 64 -  79):     4
   *  5      85           ( 80 -  95):     5
   *  6   	102           ( 96 - 111):     6
   *  7   	119           (112 - 127):     7
   *  8   	136           (128 - 143):     8
   *  9   	153           (144 - 159):     9
   *  10    170           (160 - 175):    10
   *  11    187           (176 - 191):    11
   *  12    204           (192 - 207):    12
   *  13    221           (208 - 223):    13
   *  14    238           (224 - 239):    14
   *  15    255           (240 - 255):    15
   *
   *@endcode
   */
  class ScannerFile4BitEncoded: public ScannerFile
  {
  public:
    typedef  ScannerFile4BitEncoded*  ScannerFile4BitEncodedPtr;

    ScannerFile4BitEncoded (const KKStr&  _fileName,
                            RunLog&       _log
                           );

    ScannerFile4BitEncoded (const KKStr&  _fileName,
                            uint32        _pixelsPerScanLine,
                            uint32        _frameHeight,
                            RunLog&       _log
                           );

    virtual
    ~ScannerFile4BitEncoded ();

    static const uchar*  CompensationTable ();

    virtual  ScannerFileFormat  FileFormat ()  const  {return sf4BitEncoded;}

    virtual
    void  ScanRate (float  _scanRate);

    virtual
    void   WriteTextBlock (const uchar*  txtBlock,
                           uint32        txtBlockLen
                          );

    /**
     *@brief  Writes a 32 bit number into the Scanner File Stream at current location.  
     *@param[in]  idNum  nNumber that identifies Instrument data,  ex: 0 is reserved for Flow Meter Count.
     *@param[in]  dataWord 32 bit number being written.
     */
//    virtual
//    void   WriteInstrumentDataWord (uchar             idNum,
//                                    uint32            scanLineNum,
//                                    WordFormat32Bits  dataWord
//                                   );

  protected:
    /**  @brief  Read in one Scanner File Frame. */
    uint32   ReadBufferFrame ();

    virtual  kkint64  SkipToNextFrame ();

    /**  @brief Write the contents of 'frameBuffer' to he end of the scanner file. */
    virtual  void  WriteBufferFrame ();



  private:
    void  AllocateEncodedBuff ();

    static  void  ExitCleanUp ();

    void  GetNextScanLine (uchar* lineBuff,
                           uint32 lineBuffSize
                          );

    void   WriteNextScanLine (const uchar*  buffer,
                              uint32        bufferLen
                             );

    /*  Methods and variables needed for both reading and writing scanner files. */
    static uchar*  convTable4BitTo8Bit;  /**< Lookup table to translate from 3 bit to 8bit pixels.                          */
    static uchar*  convTable8BitTo4Bit;  /**< Lookup table to translate from 8 bit to 3bit pixels.                          */
    static uchar*  compensationTable;    /**< Lookup table 256 long used to translate source pixels from a camera to the
                                          * same values as would be returned if they are written and then reread by this driver.
                                          */

    static  void  BuildConversionTables ();

    struct  OpRecEndOfScanLine;  /**< OpCode 0         */
    struct  OpRecTextBlock1;     /**< OpCode 1         */
    struct  OpRecTextBlock2;

    struct  OpRecInstrumentDataWord1;  /**<  OpCode 2  */
    struct  OpRecInstrumentDataWord2;  /**<  OpCode 2  */
    struct  OpRecInstrumentDataWord3;  /**<  OpCode 2  */

    struct  OpRecRunLen;         /**< OpCodes 4 thru 9 */
    
    struct  OpRecRun256Len1;     /**< OpCode 8         */
    struct  OpRecRun256Len2;

    struct  OpRecRaw1Pixel;      /**< OpCode 11        */

    struct  OpRecRaw32Pixels;    /**< OpCode 12        */

    struct  OpRecRaw513Pixels1;  /**< OpCode 13        */
    struct  OpRecRaw513Pixels2;

    struct  RawPixelRec;

    union   OpRec;
    typedef OpRec*  OpRecPtr;


    void  PrintSizeInfo ();


    /** Methods and variables that are required for reading a scanner file. */
    void  ProcessTextBlock (const OpRec&  rec);
    void  ProcessInstrumentDataWord (const OpRec&  rec);
    void  AllocateRawPixelRecBuffer (uint32 size);
    void  ProcessRawPixelRecs (kkuint16   numRawPixelRecs,
                               uchar*     lineBuff,
                               uint32     lineBuffSize,
                               uint32&    bufferLineLen
                              );

    RawPixelRec*  rawPixelRecBuffer;
    uint32        rawPixelRecBufferSize;
    uint32        rawPixelRecBufferLen;

    /** Methods and variables that are required for writing a scanner file. */

    void  AddCurRunLenToOutputBuffer ();
    void  AddCurRawStrToOutputBuffer ();
    void  AllocateRawStr (kkuint16  size);
    void  ReSizeEncodedBuff (uint32  newSize);

    typedef  enum  {csNull, csRunLen, csRaw}  CompStatus;

    OpRecPtr  encodedBuff;           /**< This is where compressed data will be stored before writing to scanner file.  */
    uint32    encodedBuffLen;        /**< Number of bytes used so far.                                                  */
    OpRecPtr  encodedBuffNext;       /**< Pointer to next position in encodedBuff to write to.                          */
    uint32    encodedBuffSize;       /**< Size of 'encodedBuff' allocated.                                              */

    uchar*    rawStr;
    kkuint16    rawStrLen;
    kkuint16    rawStrSize;
    //uint16    rawStrNumSameInARow;

    int32  runLen;
    uchar  runLenChar;
    CompStatus  curCompStatus;
  };  /* ScannerFile4BitEncoded */
}

#endif

