/* Histogram.h -- Class that represents a single Histogram.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#ifndef  _HISTOGRAM_
#define  _HISTOGRAM_

#include  "BasicTypes.h"
#include  "Str.h"


/**
 *@class  KKU::Histogram
 *@brief  Used to manage the construction of a Histogram.
 */
namespace KKU
{
  #ifndef _RASTER_
  class  Raster;
  typedef  Raster*  RasterPtr;
  #endif


  class  Histogram
  {
  public:
    Histogram (float  _minValue,
               int32  _numOfBuckets,
               float  _bucketSize,
               bool   _wrapArround
              );

    ~Histogram ();

    int32         AreaInRange (int32  minBucket,
                               int32  maxBucket
                              )  const;

    float         AreaInRangePercent (int32  minBucket,
                                      int32  maxBucket
                                     )  const;

    float         AverageOfMaxBucket ()  const;

    float         AverageOfMaxBucketInRange (int32  firstBucket,
                                             int32  lastBucket
                                            )  const;


    float         AverageOfMaxBucketExcludingRange (int32  minBucket,
                                                    int32  maxBucket
                                                   )  const;

    float         AverageOfMinBucketInRange (int32  minBucket,
                                             int32  maxBucket
                                            )  const;

    float         Bucket (int32  bucket)  const;

    float         BucketSize ()  const  {return bucketSize;}

    void          CalculatePeaks (int32  threshold);

    float         CountOfMaxBucket ()  const;

    RasterPtr     CreateGraph ()  const;

    RasterPtr     CreateGraph (int32  barSize)  const;

    Histogram*    Equalized ();

    int32*        EqualizedMapTable ()  {return  equalizedMapTable;}

    void          GetStats (float&   min,
                            float&   max,
                            float&   mean,
                            float&   variance
                           );

    int32         GetPeakBucket (int32 peakNum);

    int32         GetPeakByHighestOrder (int32 peakNum);

    float         GetPeakAvgByHighestOrder (int32 peakNum);

    void          Increment (float  val);

    bool          IsBucketAPeak (int32  bucket,
                                 int32  tolerance
                                )  const;

    int32         MaxBucketIdx ()  const;

    float         MinValue () const  {return minValue;}

    int32         NumOfBuckets ()  const  {return  numOfBuckets;}

    void          PrintTable (ostream&  o);

    void          Save (KKStr  fileName)  const;

    void          SaveGraphImage (const KKStr&  fileName)  const;

    void          SaveGraphImage (const KKStr&  fileName,
                                  int32         barSize
                                 )  const;

    Histogram*    Smooth (int32 smoothWidth);

    float         TotalCount () const {return  totalCount;}

  private:
    float       bucketSize;
    float*      buckets;
    float*      bucketTotals;

    int32*      equalizedMapTable;

    float       minValue;

    int32       numOfBuckets;

    vector<int32> peaks; 

    float       range;

    float       totalCount;   // Total of all Buckets
    float       totalVal;

    bool        wrapArround;
  };  /* Histogram */

  typedef  Histogram*  HistogramPtr;

} /* namespace KKU; */

#endif

