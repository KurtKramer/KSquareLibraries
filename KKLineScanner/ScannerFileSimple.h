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
                       uint32        _pixelsPerScanLine,
                       uint32        _frameHeight,
                       RunLog&       _log
                      );

    virtual
    ~ScannerFileSimple ();

    virtual  ScannerFileFormat  FileFormat ()  const  {return sfSimple;}

    virtual 
    void   WriteTextBlock (const uchar*  txtBlock,
                           uint32        txtBlockLen
                          );

    static
    const uchar*  CompensationTable ();


  protected:
    virtual
      uint32  ReadBufferFrame ();

    virtual
      int64   SkipToNextFrame ();


    virtual
      void  WriteBufferFrame ();

  };
}


#endif

