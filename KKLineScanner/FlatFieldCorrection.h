/* FlatFieldCorrection.h -- 
 * Copyright (C) 2011-2013  Kurt Kramer
 * For conditions of distribution and use, see copyright notice in LarcosCounterUnManaged.txt
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
   * such that 255 is background and 0 is forground.  The data athat will result from 'ApplyFlatFieldCorrection'
   * will have 0 as forground and 255 as background.  
   */
  class  FlatFieldCorrection
  {
  public:
    FlatFieldCorrection (kkint32       _numSampleLines,
                         kkint32       _lineWidth,
                         const uchar*  _compensationTable
                        );

    ~FlatFieldCorrection ();

    bool    Enabled             () const {return enabled;}
    kkint32 LineWidth           () const {return lineWidth;}
    kkint32 NumSampleLines      () const {return numSampleLines;}
    kkint32 NumSampleLinesAdded () const {return numSampleLinesAdded;}

    void  CompensationTable (const uchar*  _compensationTable);

    void  Enabled (bool _enabled)  {enabled = _enabled;}


    /**
     *@brief  Provide sample of one scan line as from the camera;  where 0 = forground and 255 = background.
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
    VectorUcharPtr  CameraHighPointsFromLastNSampleLines (kkint32 n)  const;

    const uchar**  LookUpTable (uchar**&  lookUpTable);

  private:
    void  ReComputeLookUpForColumn (kkint32 col);

    const uchar*  compensationTable;          /**< From ScannerFile::ConpensationTable(); used to compensate for the effects of ScannerFile compression. */

    bool      enabled;                /**< When set to 'true'  will apply flat field correction otherwise ignore.     */
    uchar*    highPoint;              /**< Highest pixel value in history for the respective column.                  */
    kkint32*  highPointLastSeen;      /**< The number of Samplings since high point was last seen.                    */
    uchar**   history;                /**< 2D array (_numSampleLines x _lineWidth); each row represents a scan-line.  */
    kkint32   lastHistoryIdxAdded;    /**< Index of last history line to be added by 'AddSampleLine'                  */
    kkint32   lineWidth;
    uchar**   lookUpTable;            /**< 2D array (lineWidth x 256) lookup table for each scan-line pixel location. *
                                      * Each column will be the look up table for the respective scan line pixel.    *
                                      * Every time a new high point s seen for a pixel location that column will get *
                                      * recomputed.                                                                  *
                                      */
    kkint32   numSampleLines;         /* Number of history scan lines that are to be kept.                            */
    kkint32   numSampleLinesAdded;    /* Total number of sample lines kept.                                           */
    kkint32*  totalLine;
  };

  typedef  FlatFieldCorrection*  FlatFieldCorrectionPtr;

}  /* FlatFieldCorrection */

#endif
