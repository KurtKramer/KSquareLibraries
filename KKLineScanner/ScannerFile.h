#if  !defined(_SCANNERFILE_)
#define  _SCANNERFILE_

#include <vector>


//**************************************************************************
//*                              ScannerFile                               *
//*                                                                        *
//*                                                                        *
//* Base class to be used by different ScanLine buffer formats.            *
//*                                                                        *
//* <p>Copyright:   Copyright (c) 2011</p>                                 *
//* <p>Author:      Kurt Kramer</p>                                        * 
//*                                                                        *
//*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *
//*                                                                        *
//*                                                                        *
//**************************************************************************

#include "GoalKeeper.h"
#include "KKBaseTypes.h"
#include "KKStr.h"
#include "RunLog.h"
using namespace KKB;

#define  MAXLINELEN  4096

#include "StartStopPoint.h"
#include "ScannerHeaderFields.h"


namespace  KKLSC
{

#if  !defined(_SCANNERFILEENTRY_)
  class  ScannerFileEntry;
  typedef  class  ScannerFileEntry*  ScannerFileEntryPtr;
#endif


  /** 
   * @class  ScannerFile  ScannerFile.h  base class to be used for all the different Scanner File Formats.
   */
  class  ScannerFile
  {
  public:
    typedef  ScannerFile*  ScannerFilePtr;

    enum  class  Format
                   {sfSimple,
                    sf2BitEncoded,
                    sf3BitEncoded,
                    sf4BitEncoded,
                    sfZlib3BitEncoded,
                    sfUnKnown
                   };

    typedef  enum  {ioRead,  ioWrite}  IOMode;


    /**  Constructor for opening file for reading */
    ScannerFile (const KKStr&  _fileName,
                 RunLog&       _log
                );

    /**  Constructor for opening file for Writing */
    ScannerFile (const KKStr&  _fileName,
                 kkuint32      _pixelsPerScanLine,
                 kkuint32      _frameHeight,
                 RunLog&       _log
                );

    virtual  ~ScannerFile ();

    virtual  kkMemSize  MemoryConsumedEstimated ()  const;

    virtual  Format  FileFormat ()  const  {return Format::sfUnKnown;}

    KKStr   FileFormatStr      ()  const;

    virtual  void  Close ();

    virtual  void  Flush ();

    static
      const  uchar*  ConpensationTable (Format  format);
  
    bool                    BuildFrameOffsetsRunning  ()  const {return  frameOffsetsBuildRunning;}
    kkint64                 ByteOffsetScanLineZero    ()  const {return  byteOffsetScanLineZero;}  /**< Byte offset of 1st scan line after header field.                */
    bool                    Eof                       ()  const {return  eof;}
    const KKStr&            FileName                  ()  const {return  fileName;}
    kkint64                 FileSizeInBytes           ()  const {return  fileSizeInBytes;}         /**<  When opening a existing file represents size of file in Bytes. */
    bool                    FlatFieldEnabled          ()  const {return  flatFieldEnabled;}
    kkuint32                FlowMeterCounter          ()  const {return  flowMeterCounter;}
    kkuint32                FlowMeterCounterScanLine  ()  const {return  flowMeterCounterScanLine;}
    kkint32                 FrameHeight               ()  const {return  frameHeight;}
    uchar*                  FrameBuffer               ()  const {return  frameBuffer;}
    bool                    FrameOffsetsLoaded        ()  const {return  frameOffsetsLoaded;}
    kkint64                 FrameBufferFileOffsetLast ()  const {return  frameBufferFileOffsetLast;}
    kkint64                 FrameBufferFileOffsetNext ()  const {return  frameBufferFileOffsetNext;}
    ScannerHeaderFieldsPtr  HeaderFields              ()  const {return  headerFields;}
    kkint32                 LargestKnowmFrameNum      ()  const {return  ((kkint32)frameOffsets.size () - 1);}
    kkint32                 LargestKnownScanLine      ()  const {return  largestKnownScanLine;}
    kkint32                 LastScanLine              ()  const {return  lastScanLine;}            /**<  Last Scan-line read or written.                               */
    kkint32                 NextScanLine              ()  const {return  nextScanLine;}            /**<  Next scan-line to be read.                                    */
    bool                    Opened                    ()  const {return  opened;}
    kkuint32                PixelsPerScanLine         ()  const {return  pixelsPerScanLine;}
    float                   ScanRate                  ()  const {return  scanRate;}

    virtual
    void  ScanRate          (float  _scanRate);
 
  

    /********************************************************************/
    /*  next several methods support Start-Stop-Point maintenance.      */
    /********************************************************************/

    /**@brief  Adds a Start-Point to the 'StartStopPoints'  list.  */
    void  AddStartPoint (kkint32  _scanLineNum);

    /**@brief  Adds a Stop-Point to the 'StartStopPoints'  list.  */
    void  AddStopPoint (kkint32  _scanLineNum);

    void  StartStopPointDelete (kkint32 _scanLineNum);

    StartStopPointPtr  StartStopPointNearestEntry (kkint32 _scanLineNum);

    StartStopPointPtr  StartStopPointPrevEntry (kkint32 _scanLineNum);

    StartStopPointPtr  StartStopPointSuccEntry (kkint32 _scanLineNum);

    const StartStopPointList&   StartStopPoints ()  const  {return startStopPoints;}



    /**
     *@brief  Returns an array indication the record rate in bytes/sec for specified time-intervals.
     *@details  Each element in the returned array will give the average number of bytes recorded for the time
     * the corresponding time interval.  Array element 0 starts at the beginning of the Scanner file and covers
     * the number of scan lines required to account for 'intervalSecs' seconds.
     */
    VectorFloatPtr  RecordRateByTimeIntervals (int intervalSecs);


    void  AddHeaderField (const KKStr&  _fieldName,
                          const KKStr&  _fieldValue
                         );

    void  AddHeaderFields (const ScannerHeaderFieldsPtr  _headerFields);

    const KKStr&  GetValue (const KKStr&  fieldName);

    float  GetValueFloat (const KKStr&  fieldName);




    /**
     *@brief  Will update the 'frameOffsets' table by scanning the file from the last known entry until the end of file.
     *@details  It would be best to call this method using a separate thread.  The method will utilize synchronization code
     *  to prevent interference with the other access methods such as 'GetNextLine', 'FrameRead', etc.  The idea is that
     *  it will not interfere with file positioning.
     *@param[in]  cancelFlag  This Boolean variable will be monitored by the method; if it turns true it will terminate 
     *                        and return immediately.
     */
    void  BuildFrameOffsets (const volatile bool&  cancelFlag);


    /**
     *@brief  Call this method to Load the FrameOffsets and StartStop points from the index file.
     *@details  This method is also called from 'BuildFrameOffsets',
     */
    void  LoadIndexFile (bool&  successful);


    /**
     *@brief Read into frame buffer 'frameNum' and reposition so that next scan-line will be the 1st line in the frame.
     *@details  The purpose of this method is to allow you to get whole frames at a time. Use the access method 'FrameBuffer' 
     * to get a pointer to the contents of the frame retrieved.  The next call to 'GetNextLine' will return the first scan
     * line in frame 'frameNum'.
     */
    void  FrameRead (kkuint32  frameNum,
                     bool&     found
                    );


    virtual 
    void  GetNextLine (uchar*     lineBuff,
                       kkuint32   lineBuffSize,
                       kkuint32&  lineSize,
                       kkuint32   colCount[],
                       kkuint32&  pixelsInRow
                      );
  
    void  InitiateWritting ();

    void  Reset ();
  
    void  SkipNextLine ();


    /**
     *@brief  Repositions the file such that the next call to 'GetNextLine' returns the 'scanLine' scan-line.
     *@details This method depends on the table'frameOffsets'; it will compute the frame number from the scan line
     * and use the appropriate entry in 'frameOffsets' to start reading from the beginning of the frame that contains
     * 'scanLine'.  If you select a 'scanLine' that is beyond the known number of scan lines in the scanner file
     * (see largestKnownScanLine) the eof flag will be set to 'true'.
     *@param[in]  scanLine  Scan line to skip to so that the next call to 'GetNextLine' retrieves it.
     */
    void  SkipToScanLine (kkint32  scanLine);
  

    virtual
    void   WriteScanLine (const uchar*  buffer,
                          kkuint32      bufferLen
                         );

    virtual
    void   WriteTextBlock (const uchar*  txtBlock,
                           kkuint32      txtBlockLen
                          ) = 0;

 
    /**
     *@brief  Writes a 32 bit number into the Scanner File Stream at current location.  
     *@param[in]  idNum  nNumber that identifies Instrument data,  ex: 0 is reserved for Flow Meter Count.
     *@param[in]  scanLineNum  Scan-line that 'dataWord' occurred at.
     *@param[in]  dataWord 32 bit number being written.
     */
    virtual
    void   WriteInstrumentDataWord (uchar             idNum,
                                    kkuint32          scanLineNum,
                                    WordFormat32Bits  dataWord
                                   );


    static
    Format  GuessFormatOfFile (const KKStr&  _fileName,
                               RunLog&       _log
                              );
  
    static
    ScannerFilePtr  CreateScannerFile (KKStr    _fileName,
                                       RunLog&  _log
                                      );

    static  
    ScannerFilePtr  CreateScannerFileForOutput (const KKStr&  _fileName,
                                                Format        _format,
                                                kkuint32      _pixelsPerScanLine,
                                                kkuint32      _frameHeight,
                                                RunLog&       _log
                                               );
  
    static  
    ScannerFilePtr  CreateScannerFileForOutput (const KKStr&   _fileName,
                                                const KKStr&   _formatStr,
                                                kkuint32       _pixelsPerScanLine,
                                                kkuint32       _frameHeight,
                                                RunLog&        _log
                                               );
  

    static
    const KKStr&  ScannerFileFormatToStr (Format  fileFormat);
    
    static
    Format  ScannerFileFormatFromStr (const KKStr&  fileFormatStr);


    /**
     *@brief Retrieves statistics for a specified Scanner File.
     *@details Will read the header information for the specified scanner file to retrieve
     *         parameters.
     *@param[in]   _scannerFileName    Name of scanner file that you want to retrieve parameters for.
     *@param[out]  _headerFields       Copy of header fields from Scanner File;  caller will own them and 
     *                                 be responsible for deleting them;  if != NULL upon call previous 
     *                                 instance will be deleted.
     *@param[out]  _scannerFileFormat  Format of scanner file;  ex: sf3BitEncoded.
     *@param[out]  _frameHeight        Frame height of source camera.
     *@param[out]  _frameWidth         Width in pixels of scanner file.
     *@param[out]  _scanRate           Scan lines/sec that imagery was acquired at.
     *@param[out]  _successful         Indicates if successful in retrieving parameters.
     *@param[in,out]  _log             Log file.
     */
    static
    void   GetScannerFileParameters (const KKStr&             _scannerFileName,
                                     ScannerHeaderFieldsPtr&  _headerFields,
                                     Format&                  _scannerFileFormat,
                                     kkint32&                 _frameHeight,
                                     kkint32&                 _frameWidth,
                                     float&                   _scanRate,
                                     bool&                    _successful,
                                     RunLog&                  _log
                                    );
                                     


  protected:
    void  AllocateFrameBuffer ();

    void  ExtractHeaderField (const KKStr&  fieldName,
                              const KKStr&  fieldValue
                             );
    void  Open (const KKStr&  _fileName);


    /**  
     *@brief  Read in one Scanner File Frame return number of actual scan-lines read. 
     *@details  Unless end of file is reached this method will read in 'framHeight' scan-lines.
     */
    virtual
      kkuint32  ReadBufferFrame () = 0;

  
    void   ReadHeader ();


    /**
     *@brief  Text messages that are embedded in a scanner file can be reported here.
     *@details  If a derived class/format of 'ScannerFile' contains Text messages, that class
     * would call this method with the embedded text message.  Possible uses of this would be
     * instrumentation data such as that produced by CTD.
     */
    void  ReportTextMsg (const char*  textBuff, 
                         kkint32      numTextBytes
                        );

    void  ReportInstrumentDataWord (uchar             idNum,
                                    kkuint32          scanLineNum,
                                    WordFormat32Bits  dataWord
                                   );


    kkint32  FSeek (kkint64  filePos);

    /**  
     *@brief Write the contents of 'frameBuffer' to he end of the scanner file.
     *@details  Will write the entire contents of 'frameBuffer' to the end of the scanner file.
     */
    virtual
      void  WriteBufferFrame () = 0;


    /**
     *@brief  This method is called before any scanner data is added to the file.  It will
     *        write the header information for the file
     */
    void  WriteHeader ();


    static
    void  ReadHeaderOneLine (FILE*   f,
                             bool&   endOfText,
                             KKStr&  line
                            );

    void  SkipBytesForward (kkuint32  numBytes);

    static
    const KKStr  fileFormatOptions[];

    void     CreateGoalie ();
    kkint64  GetFrameOffset (kkuint32  frameNum);
    void     DetermineFrameOffsetForFrame (kkuint32  frameNum);

    void     UpdateFrameOffset (kkuint32 frameNum,
                                kkuint32 scanLineNum,
                                kkint64  byteOffset
                               );

    void  SaveIndexFile (std::vector<kkint64>&  frameOffsets);

    void  AddStartStopEntryToIndexFile (kkint32                        scanLineNum,
                                        StartStopPoint::StartStopType  type,
                                        bool                           deleteEntry
                                       );


    /**
     *@brief Skip to start of next frame returning back byte offset of that frame.
     *@details  This will be called by the 'UpdateFrameOffset' and 'DetermineFrameOffsetForFrame'  methods used 
     *  to build the 'frameOffsets' table.  It is important that the implementation of this method NOT update
     *  the frameBuffer fields; such as 'frameBuffer', 'frameBufferNextLine', etc .....
     */
    virtual
      kkint64   SkipToNextFrame () = 0;

    
    kkint64                 byteOffsetScanLineZero;  /**< Byte offset of 1st scan line after the header fields. */
    bool                    eof;
    FILE*                   file;
    KKStr                   fileName;
    kkint64                 fileSizeInBytes;
    bool                    flatFieldEnabled;        /**< Indicates if Flat-Field-Correction was enabled when file recorded.  */
    kkuint32                frameHeight;             /**< Represents the number of scam lines per frame; as stored in the Scanner File. */
    bool                    headerDataWritten;       /**< Sets to true after all Header data has been written.                */
    ScannerHeaderFieldsPtr  headerFields;
    IOMode                  ioMode;
    kkint32                 largestKnownScanLine;
    kkint32                 lastScanLine;            /**< The last scan-line read.                              */
    RunLog&                 log;
    kkint32                 nextScanLine;            /**< The next scan-line that will be returned by 'GetNextLine'. */
    bool                    opened;
    kkuint32                pixelsPerScanLine;
    float                   scanRate;                /**< Scan-Lines Per Second.  */


    
    kkuint32  flowMeterCounter;          /**< Updated while reading Scanner Files; whenever InstrumentID == 0 is read
                                          * this field will be updated along with 'flowMeterCounterScanLineNum'.
                                          */

    kkuint32  flowMeterCounterScanLine;  /**< Represents the scan line that 'flowMeterCounter' was last updated for
                                          * while reading a Scanner File.
                                          */
 
    // 'frameBuffer' fields:  The following fields are to be used for buffering ScnnerFile frames, both reading and writing.

    uchar*    frameBuffer;               /**< Raw scanner data will be stored here;  for both reading and writing on scanner  *
                                          * file frame data. The frame of the scanner file is not the same as a frame from    *
                                          * the camera.                                                                       *
                                          */

    kkint64   frameBufferFileOffsetLast; /**< The byte-offset that the last frameBuffer read or written to starts at.         */

    kkint64   frameBufferFileOffsetNext; /**< The byte-offset of the next frameBuffer to be read or written                   */

    bool      frameBufferLastReadEof;    /**< Indicates that the last call to 'ReadBufferFrame' encountered    *
                                          *   feof (file) which means the next call to 'ReadBufferFrame' should return eof.   *
                                          */

    kkuint32  frameBufferLen;            /**< Represents the number of bytes being used in 'frameBuffer';  when data is being *
                                          * read from a Scanner file you would start reading from this point in the buffer and *
                                          * while data is be written it will be added to 'frameBuffer' and this variable will *
                                          * be incremented to reflect the new amount occupied.                                *
                                          */
 
    kkuint32  frameBufferNextLine;       /**< Next scan-line in 'frameBuffer' to either read or write; used to compute the    *
                                          *   byte offset into the buffer.                                                    *
                                          */

    kkuint32  frameBufferNumScanLines;   /**< The Number of scan-lines that the last call to 'ReadBufferFrame' read.          */

    kkuint32  frameBufferSize;           /**< The number of bytes that were allocated to 'frameBuffer';  'frameBufferLen'     *
                                          * should never exceed this value.                                                   *
                                          */

    kkuint32  frameNumCurLoaded;         /**< Indicates the frame that is currently loaded in 'frameBuffer'.                  */


    std::vector<kkint64>  frameOffsets;  /**<  Will maintain a list of byte offsets;  each entry will be the byte offset for  *
                                          * its corresponding frame                                                           *
                                          */
    bool      frameOffsetsBuildRunning;

    bool      frameOffsetsLoaded;        /**< Indicates if the entire scannerFile has been scanned and all entries in         *
                                          * frameOffsets are updated.   Can only be set to 'true' upon successful completion  *
                                          * of 'BuildFrameOffsets'.                                                           *
                                          */

    GoalKeeperPtr        goalie;         /**<  Used to control access to the buildFrameIndex table.                           */

    std::ofstream*       indexFile;      /**< When writing a Scanner file will also write out a ByteOffset index file to aid
                                          * future access to this file.
                                          */
    KKStr                indexFileName;

    ScannerFileEntryPtr  scannerFileEntry;


    StartStopPointList   startStopPoints;   /**< Points where he user does or does-not want to process(Count) are tacked in this
                                             * data structure.  Each entry is flagged as a Start or Stop point.  These entries
                                             * are saved in the IndexFile along with the frame offsets table.
                                             */

  };  /* ScannerFile */

  typedef ScannerFile::ScannerFilePtr  ScannerFilePtr;


}  /* KKLSC */


#endif
