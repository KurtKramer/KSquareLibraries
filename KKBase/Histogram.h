/* Histogram.h -- Class that represents a single Histogram.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef  _HISTOGRAM_
#define  _HISTOGRAM_

#include "KKBaseTypes.h"
#include "KKStr.h"


/**
 *@class  KKB::Histogram
 *@brief  Used to manage the construction of a Histogram.
 */
namespace KKB
{
  #ifndef _RASTER_
  class  Raster;
  typedef  Raster*  RasterPtr;
  #endif


  class  Histogram
  {
  public:
    Histogram (float  _minValue,
               kkint32  _numOfBuckets,
               float  _bucketSize,
               bool   _wrapArround
              );

    ~Histogram ();

    kkint32       AreaInRange (kkint32  minBucket,
                               kkint32  maxBucket
                              )  const;

    float         AreaInRangePercent (kkint32  minBucket,
                                      kkint32  maxBucket
                                     )  const;

    float         AverageOfMaxBucket ()  const;

    float         AverageOfMaxBucketInRange (kkint32  firstBucket,
                                             kkint32  lastBucket
                                            )  const;


    float         AverageOfMaxBucketExcludingRange (kkint32  minBucket,
                                                    kkint32  maxBucket
                                                   )  const;

    float         AverageOfMinBucketInRange (kkint32  minBucket,
                                             kkint32  maxBucket
                                            )  const;

    float         Bucket (kkint32  bucket)  const;

    float         BucketSize ()  const  {return bucketSize;}

    void          CalculatePeaks (kkint32  threshold);

    float         CountOfMaxBucket ()  const;

    RasterPtr     CreateGraph ()  const;

    RasterPtr     CreateGraph (kkint32  barSize)  const;

    Histogram*    Equalized ();

    kkint32*        EqualizedMapTable ()  {return  equalizedMapTable;}

    void          GetStats (float&   min,
                            float&   max,
                            float&   mean,
                            float&   variance
                           );

    kkint32       GetPeakBucket (kkint32 peakNum);

    kkint32       GetPeakByHighestOrder (kkint32 peakNum);

    float         GetPeakAvgByHighestOrder (kkint32 peakNum);

    void          Increment (float  val);

    bool          IsBucketAPeak (kkint32  bucket,
                                 kkint32  tolerance
                                )  const;

    kkint32       MaxBucketIdx ()  const;

    float         MinValue () const  {return minValue;}

    kkint32       NumOfBuckets ()  const  {return  numOfBuckets;}

    void          PrintTable (ostream&  o);

    void          Save (KKStr  fileName)  const;

    void          SaveGraphImage (const KKStr&  fileName)  const;

    void          SaveGraphImage (const KKStr&  fileName,
                                  kkint32       barSize
                                 )  const;

    Histogram*    Smooth (kkint32 smoothWidth);

    float         TotalCount () const {return  totalCount;}

  private:
    float       bucketSize;
    float*      buckets;
    float*      bucketTotals;

    kkint32*    equalizedMapTable;

    float       minValue;

    kkint32     numOfBuckets;

    vector<kkint32> peaks; 

    float       range;

    float       totalCount;   // Total of all Buckets
    float       totalVal;

    bool        wrapArround;
  };  /* Histogram */

  typedef  Histogram*  HistogramPtr;

} /* namespace KKB; */

#endif

