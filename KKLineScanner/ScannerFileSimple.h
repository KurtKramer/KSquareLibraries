#if  !defined(_SCANNERFILESIMPLE_)
#define  _SCANNERFILESIMPLE_


namespace  KKLSC
{
  /**
   *@class KKLSC::ScannerFileSimple  ScannerFileSimple.h A simple format where each scan line is stored without any encoding or compression.
   */

  class  ScannerFileSimple:  public ScannerFile
  {
  public:
    /**  Constructor for opening file for reading */
    ScannerFileSimple (const KKStr&  _fileName,
                       RunLog&       _log
                      );


    /**  Constructor for opening file for Writing */
    ScannerFileSimple (const KKStr&  _fileName,
                       kkuint32      _channelCount,
                       kkuint32      _pixelsPerScanLine,
                       kkuint32      _frameHeight,
                       RunLog&       _log
                      );

    virtual
    ~ScannerFileSimple ();

    virtual  Format  FileFormat ()  const  {return Format::sfSimple;}

    virtual 
    void   WriteTextBlock (const uchar*  txtBlock,
                           kkuint32      txtBlockLen
                          );

    static
    const uchar*  CompensationTable ();


  protected:
    virtual
      kkuint32  ReadBufferFrame ();

    virtual
      kkint64   SkipToNextFrame ();


    virtual
      void  WriteBufferFrame ();

  };
}


#endif

