#if  !defined(_SCANNERFILEZLIBENCODED_)
#define  _SCANNERFILEZLIBENCODED_

#include  "ScannerFile.h"

namespace  KKLSC
{
  /**
   *@class ScannerFileZLib3BitEncoded  
   *@brief Implements Zlib compression on 3bit pixel data.
   *@details Each frame of 3bit data will be compressed using the ZLIB library.
   *@code
   *  Following the header fields at the beginning of the file there will be blocks of data
   *  where the first byte will contain a op-code that describes how to decode the following
   *  bytes.  
   *  All data will be stored in two byte records with the exception of text.
   *  each 2 byte record will start with a 3 bit OpCode and a one bit eol flag.
   *  the rest of the 12 bits will be organized depending on the opcode field.
   *  
   *  Byte  1           : Op-Code
   *  Bytes 2      thru 1 + x:      y = Length of rest of block.  High order followed by low order.
   *  Bytes 2 + x  thru 1 + x + y:  Data, to decoded as specified by op-code will be y bytes long
   * 
   *  Op-Code:
   *  1:  Text block up to 255 bytes long,          x = 1, will be followed by "1" byte specifying text block length.
   *  2:  Text block up to 65,535 bytes long,       x = 2, will be followed by "2" bytes (High order, low order) specifying
   *      text block length.
   *
   *  Op-Coes 5, 6, 7 assume that frame size recorded is the same as defined in the header.
   *  5:  Zlib compressed data up to 65,535 bytes,  x = 2, will be followed by "2" bytes (High order, low order) specifying 
   *      compressed data length.
   *  6:  ZLib compressed data up to 16,777,215,    x = 3, Will be followed by "3" bytes that specify length of compressed data.
   *  7:  ZLib compressed data up to 4,294,967,295, x = 4, Will be followed by "4" bytes that specify length of compressed data.
   *
   *
   *@endcode
   */
  class ScannerFileZLib3BitEncoded: public ScannerFile
  {
  public:
    typedef  ScannerFileZLib3BitEncoded*  ScannerFile3BitEncodedPtr;

    /**  Constructor for opening file for reading */
    ScannerFileZLib3BitEncoded (const KKStr&  _fileName,
                                RunLog&       _log
                               );

    /**  Constructor for opening file for Writing */
    ScannerFileZLib3BitEncoded (const KKStr&  _fileName,
                                kkuint32      _pixelsPerScanLine,
                                kkuint32      _frameHeight,
                                RunLog&       _log
                               );

    virtual
    ~ScannerFileZLib3BitEncoded ();


    virtual  ScannerFileFormat  FileFormat ()  const  {return sfZlib3BitEncoded;}

    virtual
    void  ScanRate          (float  _scanRate);

    virtual
    void   WriteTextBlock (const uchar*  txtBlock,
                           kkuint32      txtBlockLen
                          );

  private:
    void  AllocateBuffers ();

    void  ExpandBuffer (uchar*&  buffer,
                        kkuint32&  bufferSize,
                        kkuint32 bufferNewSize
                       );

    void  ExpandBufferNoCopy (uchar*&  buffer,
                              kkuint32&  bufferSize,
                              kkuint32 bufferNewSize
                             );

    virtual
      kkint64  SkipToNextFrame ();
    
    virtual
      kkuint32  ReadBufferFrame ();
    
    virtual
      void  WriteBufferFrame ();


    typedef uchar   uint8;

    struct  OpCodeRec1;
    struct  OpCodeRec2;
    struct  OpCodeRec5;
    struct  OpCodeRec6;
    struct  OpCodeRec7;
    struct  TwoByteRec;
    struct  ThreeByteRec;
    struct  FourByteRec;

    uchar*  compBuffer;
    kkuint32  compBufferLen;
    kkuint32  compBufferSize;
    
  };  /* ScannerFileZLib3BitEncoded */
}

#endif

