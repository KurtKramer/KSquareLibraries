/* ContourFollower.h -- Used to find the contour of image in a Raster object.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef _CONTOURFOLLOWER_
#define _CONTOURFOLLOWER_

#include  <complex>
#include  "KKBaseTypes.h"
#include  "RunLog.h"

namespace KKB
{
  #ifndef  _POINT_
  class  Point;
  typedef  Point*  PointPtr;
  class  PointList;
  typedef  PointList*  PointListPtr;
  #endif



  #ifndef _RASTER_
  class  Raster;
  typedef  Raster*  RasterPtr;
  #endif


  typedef  std::complex<double>  ComplexDouble;


  class   ContourFollower
  {
  public:  
    ContourFollower (Raster&  _raster,
                     RunLog&  _log
                    );

    std::vector<ComplexDouble>  CreateFourierFromPointList (const PointList&  points);

    PointListPtr  CreatePointListFromFourier (std::vector<ComplexDouble>  fourier,
                                              PointList&                  origPointList
                                             );

    PointListPtr  GenerateContourList ();

    kkint32  FollowContour (float*  countourFreq,
                            float   fourierDescriptors[15],
                            kkint32 totalPixels,
                            bool&   successful
                           );

    kkint32  FollowContour2 (float*  countourFreq,
                             bool&   successful
                            );

    kkint32  CreateFourierDescriptorBySampling (kkint32 numOfBuckets,
                                                float*  countourFreq,
                                                bool&   successful
                                             );

    void  HistogramDistanceFromAPointOfEdge (float     pointRow,
                                             float     pointCol,
                                             kkint32   numOfBuckets,
                                             kkint32*    buckets,
                                             float&    minDistance,
                                             float&    maxDistance,
                                             kkint32&  numOfEdgePixels
                                            );


  private:

    void  GetFirstPixel (kkint32&  row,
                         kkint32&  col
                        );

    void  GetNextPixel (kkint32&  row,
                        kkint32&  col
                       );

    kkint32  PixelCountIn9PixelNeighborhood (kkint32  row, 
                                             kkint32  col
                                            );

    uchar  PixelValue (kkint32 row,
                       kkint32 col
                      );

    kkint32  curCol;
    kkint32  curRow;
    kkint32  fromDir;
    kkint32  height;
    kkint32  lastDir;
    RunLog&  log;
    Raster&  raster;
    uchar**  rows;
    kkint32  width;
  }; /* ContourFollower*/

} /* namespace KKU; */

#endif
