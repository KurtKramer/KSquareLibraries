/* FlatFieldCorrection.h -- 
 * Copyright (C) 2011-2013  Kurt Kramer
 * For conditions of distribution and use, see copyright notice in CounterUnManaged.txt
 */
#if  !defined(_FLATFIELDCORRECTION_)
#define  _FLATFIELDCORRECTION_

#include "KKBaseTypes.h"
#include "GoalKeeper.h"
#include "KKQueue.h"
#include "Raster.h"
#include "RunLog.h"


namespace  KKLSC
{
  /**
   *@details  It is assumed that sample data provided via 'AddSampleLine' will be the data from a camera 
   * such that 255 is background and 0 is foreground.  The data that will result from 'ApplyFlatFieldCorrection'
   * will have 0 as foreground and 255 as background.  
   */
  class  FlatFieldCorrection
  {
  public:
    FlatFieldCorrection (kkuint32      _numSampleLines,
                         kkuint32      _lineWidth,
                         const uchar*  _compensationTable,
                         kkuint32      _startCol,
                         bool          _color
                        );

    virtual ~FlatFieldCorrection ();

    FlatFieldCorrection (const FlatFieldCorrection&) = delete;

    FlatFieldCorrection (FlatFieldCorrection&&) = delete;


    bool     Enabled             () const noexcept {return enabled;}
    kkuint32 LineWidth           () const noexcept {return lineWidth;}
    kkuint32 NumSampleLines      () const noexcept {return numSampleLines;}
    kkuint32 NumSampleLinesAdded () const noexcept {return numSampleLinesAdded;}

    void  Enabled (bool _enabled) noexcept  {enabled = _enabled;}


    /**
     *@brief  Provide sample of one scan line as from the camera;  where 0 = foreground and 255 = background.
     */
    virtual void  AddSampleLine (const uchar*  sampleLine) = 0;

    virtual void  ApplyFlatFieldCorrection (uchar*  scanLine) = 0;

    virtual void  ApplyFlatFieldCorrection (uchar*  srcScanLine,  uchar*  destScanLine) = 0;

    virtual VectorUcharPtr  CameraHighPoints ()  const = 0;

    /**
     *@brief Will return the high point for each pixel from the last 'n' sample lines taken. 
     *@param[in] n The number of Sample lines to locate high point; ex: n = 2 means check the last two sample lines.
     */
    virtual VectorUcharPtr  CameraHighPointsFromLastNSampleLines (kkuint32 n)  const = 0;

    virtual void  CompensationTable (const uchar*  _compensationTable) = 0;

  protected:
    virtual void  ReComputeLookUpForColumn (kkuint32 col) = 0;

    const uchar*  compensationTable;  /**< From ScannerFile::ConpensationTable(); used to compensate for the effects of ScannerFile compression. */

    bool       color;
    bool       enabled;                /**< When set to 'true'  will apply flat field correction otherwise ignore.     */
    kkuint32   lastHistoryIdxAdded;    /**< Index of last history line to be added by 'AddSampleLine'                  */
    kkuint32   lineWidth;
    kkuint32   numSampleLines;         /* Number of history scan lines that are to be kept.                            */
    kkint32    numSampleLinesAdded;    /* Total number of sample lines kept.                                           */
    kkuint32   startCol;               /* First column to process in a scan line; implemented to support counter dat in starting coulmns such as flow rate counter. */
  };

  typedef  FlatFieldCorrection*  FlatFieldCorrectionPtr;

}  /* FlatFieldCorrection */

#endif
