#if  !defined(_SCANNERFILE2BITENCODED_)
#define  _SCANNERFILE2BITENCODED_

#include "RunLog.h"
#include "KKStr.h"
using namespace KKB;

#include  "ScannerFile.h"

namespace  KKLSC
{
  /**
   *@class ScannerFile2BitEncoded  
   *@brief Implements a 2 bit Encoded format.
   *@details This is a very simple implementation of run-length compression.
   *         All pixels will be translated from 8 bit to 2 bit.
   *@code
   *  All data will be stored in two byte records with the exception of text.
   *  each 2 byte record will start with a 3 bit OpCode and a one bit eol flag.
   *  the rest of the 12 bits will be organized depending on the opcode field.
   *
   *  Bits
   *  =====
   *  0 - 2:  OP-Code
   *
   *  3 - 3:  EOL Flag;  End of ScanLine '0' = Not end of line,   '1' = End of line.
   *
   * Op-Code
   *   0: Text Block
   *      Bits 4 - 15  Length of text block in 4 byte groups.  So number in this field 
   *                   is multiplied by 4 to get the total number of bytes.  This allows
   *                   for up to (4096 * 4) bytes.  
   *
   *   1: 3 Raw Bytes
   *       Pix0  Pix1   Pix2
   *      (4-7) (8-11) (12-15)
   *
   *   2: Number of White Spaces
   *      Bits 4 - 15  Number of 4 pixels that are background (0).  The total length of
   *                   this field is this number times 4 allowing for a run-length of up
   *                   to (4 * 4096) spaces divisible by 4.
   *
   *@endcode
   */
  class ScannerFile2BitEncoded: public ScannerFile
  {
  public:
    typedef  ScannerFile2BitEncoded*  ScannerFile2BitEncodedPtr;

    ScannerFile2BitEncoded (const KKStr&  _fileName,
                            RunLog&       _log
                           );

    ScannerFile2BitEncoded (const KKStr&  _fileName,
                            kkuint32      _pixelsPerScanLine,
                            kkuint32      _frameHeight,
                            RunLog&       _log
                           );

    virtual
    ~ScannerFile2BitEncoded ();

    static const uchar*  CompensationTable ();

    virtual  ScannerFileFormat  FileFormat ()  const  {return sf4BitEncoded;}

    virtual
    void  ScanRate (float  _scanRate);


    virtual
    void   WriteTextBlock (const uchar*  txtBlock,
                           kkuint32      txtBlockLen
                          );

  protected:
    /**  @brief  Read on one Scanner File Frame. */
    kkuint32 ReadBufferFrame ();

    virtual
      kkint64  SkipToNextFrame ();

        /**  @brief Write the contents of 'frameBuffer' to he end of the scanner file. */
    virtual  void  WriteBufferFrame ();



  private:
    void  AllocateEncodedBuff ();

    static  void  ExitCleanUp ();

    void  GetNextScanLine (uchar*   lineBuff,
                           kkuint32 lineBuffSize
                          );

    void   WriteNextScanLine (const uchar*  buffer,
                              kkuint32      bufferLen
                             );

    /*  Methods and variables needed for both reading and writing scanner files. */
    static uchar*  convTable2BitTo8Bit;  /**< Lookup table to translate from 2 bit to 8bit pixels.                          */
    static uchar*  convTable8BitTo2Bit;  /**< Lookup table to translate from 8 bit to 2bit pixels.                          */
    static uchar*  compensationTable;    /**< Lookup table 256 long used to translate source pixels from a camera to the *
                                          * same values as would be returned if they are written and then reread by this driver.
                                          */

    static  void  BuildConversionTables ();

    struct  OpRecEndOfScanLine;           //  0
    struct  OpRecTextBlock;               //  1
    struct  OpRecTextBlock_2;             //  1 (Part 2)
    struct  OpRecRunLenPVx;               //  4 thru 7
    struct  OpRecRunLen10Bit;             //  8
    struct  OpRecRunLen10Bit_2;           //  8 (Part 2)
    struct  OpRecRawPixelOne;             // 10 
    struct  OpRecRawPixelsTwo;            // 11
    struct  OpRecRawPixelsVarLen4Bit;     // 12
    struct  OpRecRawPixelsVarLen12Bit;    // 13
    struct  OpRecRawPixelsVarLen12Bit_2;  // 13 (Part 2)
    struct  RawPixelRec;

    union   OpRec;
    typedef OpRec*  OpRecPtr;


    /** Methods and variables that are required for reading a scanner file. */
    void  ProcessTextBlock (const OpRec&  rec);
    void  AllocateRawPixelRecBuffer (kkuint32 size);
    void  ProcessRawPixelRecs (kkuint16  numRawPixels,
                               uchar*    lineBuff,
                               kkuint32  lineBuffSize,
                               kkuint32& bufferLineLen
                              );

    RawPixelRec*  rawPixelRecBuffer;
    kkuint32      rawPixelRecBufferLen;
    kkuint32      rawPixelRecBufferSize;

    /** Methods and variables that are required for writing a scanner file. */

    void  AddCurRunLenToOutputBuffer ();
    
    void  AddCurRawStrToOutputBuffer ();

    void  AddRawStrPixelsToEncodedBuffer (kkuint16&  nextCp, 
                                          kkuint16   len
                                         );

    void  AllocateRawStr (kkuint16  size);

    void  ReSizeEncodedBuff (kkuint32  newSize);

    typedef  enum  {csNull, csRunLen, csRaw}  CompStatus;

    OpRecPtr    encodedBuff;           /**< This is where compressed data will be stored before writing to scanner file.  */
    kkuint32    encodedBuffLen;        /**< Number of bytes used so far.                                                  */
    OpRecPtr    encodedBuffNext;       /**< Pointer to next position in encodedBuff to write to.                          */
    kkuint32    encodedBuffSize;       /**< Size of 'encodedBuff' allocated.                                              */

    uchar*      rawStr;
    kkuint16    rawStrLen;
    kkuint16    rawStrSize;


    kkint32     runLen;
    uchar       runLenChar;
    CompStatus  curCompStatus;
  };  /* ScannerFile2BitEncoded */
}

#endif

