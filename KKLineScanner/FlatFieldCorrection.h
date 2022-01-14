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
                         kkuint32      _startCol
                        );

    ~FlatFieldCorrection ();

    bool     Enabled             () const {return enabled;}
    kkuint32 LineWidth           () const {return lineWidth;}
    kkuint32 NumSampleLines      () const {return numSampleLines;}
    kkuint32 NumSampleLinesAdded () const {return numSampleLinesAdded;}

    void  CompensationTable (const uchar*  _compensationTable);

    void  Enabled (bool _enabled)  {enabled = _enabled;}


    /**
     *@brief  Provide sample of one scan line as from the camera;  where 0 = foreground and 255 = background.
     */
    void  AddSampleLine (const uchar*  sampleLine);

    void  ApplyFlatFieldCorrection (uchar*  scanLine);

    void  ApplyFlatFieldCorrection (uchar*  srcScanLine,
                                    uchar*  destScanLine
                                   );

    VectorUcharPtr  CameraHighPoints ()  const;

    /**
     *@brief Will return the high point for each pixel from the last 'n' sample lines taken. 
     *@param[in] n The number of Sample lines to locate high point; ex: n = 2 means check the last two sample lines.
     */
    VectorUcharPtr  CameraHighPointsFromLastNSampleLines (kkuint32 n)  const;

  private:
    void  ReComputeLookUpForColumn (kkuint32 col);

    const uchar*  compensationTable;  /**< From ScannerFile::ConpensationTable(); used to compensate for the effects of ScannerFile compression. */

    bool       enabled;                /**< When set to 'true'  will apply flat field correction otherwise ignore.     */
    uchar*     highPoint;              /**< Highest pixel value in history for the respective column.                  */
    kkuint32*  highPointLastSeen;      /**< The number of Samplings since high point was last seen.                    */
    uchar**    history;                /**< 2D array (_numSampleLines x _lineWidth); each row represents a scan-line.  */
    kkuint32   lastHistoryIdxAdded;    /**< Index of last history line to be added by 'AddSampleLine'                  */
    kkuint32   lineWidth;
    uchar**    lookUpTable;            /**< 2D array (lineWidth x 256) lookup table for each scan-line pixel location.
                                       * Each column will be the look uptable for the respective scan line pixel.
                                       * Every time a new high point s seen for a pixel location that column will
                                       * get recomputed.
                                       */
    kkuint32   numSampleLines;         /* Number of history scan lines that are to be kept.                            */
    kkint32    numSampleLinesAdded;    /* Total number of sample lines kept.                                           */
    kkuint32   startCol;               /* First column to process in a scan line; implemented to support counter dat in starting coulmns such as flow rate counter. */
    kkint32*   totalLine;
  };

  typedef  FlatFieldCorrection*  FlatFieldCorrectionPtr;

}  /* FlatFieldCorrection */

#endif
