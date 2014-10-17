#if  !defined(_SCANNERFILE3BITENCODED_)
#define  _SCANNERFILE3BITENCODED_

#include  "ScannerFile.h"

namespace  KKLSC
{
  /**
   *@class ScannerFile3BitEncoded  
   *@brief Implements a 3 bit Encoded format.
   *@details This is a very simple implementation of run-length compression only background 
   *         pixels (0) and Foreground Pixels(7/255) will be compressed by groups of 4.
   *         All pixels will be translated from 8 bit to 3 bit.
   *<p>
   *  All data will be stored in two byte records with the exception of text.
   *  each 2 byte record will start with a 3 bit OpCode and a one bit eol flag.
   *  the rest of the 12 bits will be organized depending on the opcode field.
   *<p>
   *@code
   *  Bits
   *  ======
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
   *   1: 4 Raw Bytes
   *      Pix0  Pix1  Pix2    Pix
   *      (4-6) (7-9) (10-12) (13-15)
   *
   *   2: Number of White Spaces
   *      Bits 4 - 15  Number of 4 pixels that are background (0).  The total length of
   *                   this field is this number times 4 allowing for a run len of up
   *                   to (4 * 4096) spaces divisible by 4.
   *
   *   3: Number of Black Spaces
   *      Bits 4 - 15  Number of 4 pixels that are foreground (7/255).  The total length of
   *                   this field is this number times 4 allowing for a run len of up
   *                   to (4 * 4096) Foreground pixels (7/255) divisible by 4.
   *
   *   4: Instrument Data
   *      Bits 3 - 6
   *
   * 3-Bit to 8-bit Translation
   *   0:   0
   *   1:  36
   *   2:  73
   *   3: 109
   *   4: 145	
   *   5: 181
   *   6: 218
   *   7: 255
   *
   * 8-Bit to 3-bit Translation
   *    0 ->  31:  0
   *   32 ->  63:  1
   *   64 ->  95:  2
   *   96 -> 127:  3
   *  128 -> 158:  4
   *  159 -> 190:  5
   *  191 -> 222:  6
   *  223 -> 255;  7
   *@endcode
   **/
  class ScannerFile3BitEncoded: public ScannerFile
  {
  public:
    typedef  ScannerFile3BitEncoded*  ScannerFile3BitEncodedPtr;

    ScannerFile3BitEncoded (const KKStr&  _fileName,
                            RunLog&       _log
                           );

    ScannerFile3BitEncoded (const KKStr&  _fileName,
                            uint32        _pixelsPerScanLine,
                            uint32        _frameHeight,
                            RunLog&       _log
                           );

    virtual
    ~ScannerFile3BitEncoded ();

    static
    const uchar*  CompensationTable ();

    virtual  ScannerFileFormat  FileFormat ()  const  {return sf3BitEncoded;}

    virtual
    void  ScanRate          (float  _scanRate);


    virtual
    void   WriteTextBlock (const uchar*  txtBlock,
                           uint32        txtBlockLen
                          );

  protected:
    /**  @brief  Read on one Scanner File Frame. */
    uint32   ReadBufferFrame ();

    virtual  kkint64  SkipToNextFrame ();

    /**  @brief Write the contents of 'frameBuffer' to he end of the scanner file. */
    virtual  void  WriteBufferFrame ();



  private:
    void  AllocateWorkLineAndOutputBuf ();

    static
      void  BuildConversionTables ();

    static
      void  ExitCleanUp ();

    void  GetNextScanLine (uchar* lineBuff,
                           uint32 lineBuffSize
                          );

    void   WriteNextScanLine (const uchar*  buffer,
                              uint32        bufferLen
                             );

    void   WriteNextScanLine2 (const uchar*  buffer,
                               uint32        bufferLen
                              );

    struct  OpRecTextBlock;
    struct  OpRec4RawPixels;
    struct  OpRecSpaces;
    struct  OpRecBlackOuts;
    struct  OpRecRunLen;
    struct  OpRecRaw;
    union   OpRec;
    typedef OpRec*  OpRecPtr;

    inline
    void  Write4Spaces (OpRecPtr&  outputBuffPtr,
                        int32&     outputBuffUsed,
                        int32&     num4SpacesInARow,
                        ushort     eol
                       );

    inline
    void  Write4BlackOuts (OpRecPtr&  outputBuffPtr,
                           int32&     outputBuffUsed,
                           int32&     num4BlackOutsInARow,
                           ushort     eol
                          );

    static
      uchar*    convTable3BitTo8Bit;  /**< Lookup table to translate from 3 bit to 8bit pixels.                          */
    static
      uchar*    convTable8BitTo3Bit;  /**< Lookup table to translate from 8 bit to 3bit pixels.                          */

    static
      uchar*    compensationTable;    /**< Lookup table 256 long used to translate source pixels from a camera to the *
                                       * same values as would be returned if they are written and then reread by this driver.
                                       */

    uchar     fourSpaces[4];          /**< Used to quickly check for 4 blanks in a row.                                  */
    uchar     fourBlackOuts[4];
    OpRecPtr  outputBuff;             /**< This is where compressed data will be stored before writing to scanner file.  */
    uint32    outputBuffLen;          /**< Size of 'outputBuff' allocated.                                               */
    uchar*    workLine;               /**< Where 4bit translated characters will be stored before compression.           */
    uint32    workLineLen;            /**< length of the current 'workLine' buffer.                                      */

    typedef  enum  {csNull, csRunLen, csRaw}  CompStatus;
    CompStatus  curCompStatus;

  };  /* ScannerFile3BitEncoded */
}

#endif

