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

    int32  FollowContour (float*  countourFreq,
                          float   fourierDescriptors[15],
                          int32   totalPixels,
                          bool&   successful
                         );

    int32  FollowContour2 (float*  countourFreq,
                           bool&   successful
                          );

    int32  CreateFourierDescriptorBySampling (int32   numOfBuckets,
                                              float*  countourFreq,
                                              bool&   successful
                                             );

    void  HistogramDistanceFromAPointOfEdge (float     pointRow,
                                             float     pointCol,
                                             int32     numOfBuckets,
                                             int32*    buckets,
                                             float&    minDistance,
                                             float&    maxDistance,
                                             int32&    numOfEdgePixels
                                            );


  private:

    void  GetFirstPixel (int32&  row,
                         int32&  col
                        );

    void  GetNextPixel (int32&  row,
                        int32&  col
                       );

    int32  PixelCountIn9PixelNeighborhood (int32  row, 
                                           int32  col
                                          );

    uchar  PixelValue (int32 row,
                       int32 col
                      );

    int32    curCol;
    int32    curRow;
    int32    fromDir;
    int32    height;
    int32    lastDir;
    RunLog&  log;
    Raster&  raster;
    uchar**  rows;
    int32    width;
  }; /* ContourFollower*/




  /*
  class  PixelAddr
  {
  public:  
    PixelAddr (int32  _row,
               int32  _col
              ):
          row (_row),
          col (_col)
      {}

    int32  Row ()  {return row;}
    int32  Col ()  {return col;}


  private:
    int32  row;
    int32  col;
  };

  */

} /* namespace KKB; */

#endif
