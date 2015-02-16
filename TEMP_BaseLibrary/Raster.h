/* Raster.h -- Class that one raster image.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#if  !defined(_RASTER_)
#define _RASTER_

#include <map>

#include "BasicTypes.h"
#include "KKQueue.h"
#include "Str.h"
#include "PixelValue.h"
#include "Point.h"


namespace KKU
{
  /**
   *@file  Raster.h
   *@author  Kurt Kramer
   *@details
   *@code
   ***************************************************************************
   **                                Raster                                  *
   **                                                                        *
   **  Supports image Morphological operations such as Dilation, Opening,   *
   **  Closing and other operations.                                        *
   ***************************************************************************
   *@endcode
   *@sa Blob
   *@sa ContourFollower
   *@sa ConvexHull
   *@sa PixelValue
   *@sa Point
   *@sa Sobel
   */


  #ifndef  _BLOB_
  class  Blob;
  typedef  Blob*  BlobPtr;
  class  BlobList;
  typedef  BlobList*  BlobListPtr;
  #endif

  #ifndef  _MATRIX_
  class Matrix;
  typedef  Matrix*  MatrixPtr;
  #endif

  #if  !defined(_KKU_GOALKEEPER_)
	class  GoalKeeper;
	typedef  GoalKeeper*  GoalKeeperPtr;
  #endif


  typedef  enum  
  {
    CROSS3   = 0,
    CROSS5   = 1,
    SQUARE3  = 2,
    SQUARE5  = 3,
    SQUARE7  = 4,
    SQUARE9  = 5,
    SQUARE11 = 6
  }  MaskTypes;


  typedef  enum  {RedChannel, GreenChannel, BlueChannel}  ColorChannels;


  class  RasterList;
  typedef  RasterList*  RasterListPtr;

  class  BmpImage;
  typedef  BmpImage*  BmpImagePtr;


  #ifndef  _HISTOGRAM_
  class Histogram;
  typedef  Histogram*  HistogramPtr;
  #endif

  #define DefaultBackgroundPixelTH  31


  /**
   *@class Raster 
   *@brief  A class that is used by to represent a single image in memory.  
   *@details  It supports morphological operations and other tasks. It can handle either
   *          Grayscale or Color.  By default it will use Grayscale unless specified 
   *          otherwise. It will allocate one continuous block of memory for each
   *          channel (RGB). If the image is only grayscale the Green Channel (G) will
   *          be used leaving the Red and Blue channels set to NULL. You can access
   *          Individual pixels through access methods that will ensure memory integrity
   *          or you can also access the pixel data directly in memory.
   *@see Blob
   *@see ContourFollower
   *@see ConvexHull
   *@see PixelValue
   *@see Point
   *@see PointList
   *@see RasterList()
   *@see Sobel
   */
  class  Raster
  {
  public:
    typedef  Raster*  RasterPtr;

    Raster ();

    Raster (const Raster&  _raster);  /**< @brief Copy Constructor */

    /**
     *@brief Constructs a blank GrayScale image with given dimensions; all pixels will be initialized to 0.  
     *@details  When working with Grayscale images pixel value of '0' = Background and '255'= foreground.
     *          The green channel will be used to represent the grayscale value.  When these raster images
     *          are saved to a image file such as a BMP file the pixel value of 0 will point to the color
     *          value of (255, 255, 255) and  pixel value 255 will point to the color value of (0, 0, 0).
     *          This way when displaying the image background will appear as white.
     */
    Raster (int32  _height,
            int32  _width
           );


    /**
     *@brief  Constructs a blank image with given dimensions.  
     *@details  The third parameter determines whether it will be a color or grayscale image, If a Color
     *          image then all three color channel will be set to = 255 which stands for white. If
     *          Grayscale the green channel will be set to 0.
     */
    Raster (int32 _height,
            int32 _width,
            bool  _color
           );

    /**
     *@brief  Constructs a Raster from a BMP image loaded from disk.
     *@details If BMP Image is a grayscale value pixel values will be reversed. See description of
     *        grayscale constructor.
     */
    Raster (const BmpImage&  _bmpImage);


    /**
     *@brief Constructs a new Raster using a subset of the specified Raster as its source. The
     *      dimensions of the resultant raster will be '_height', and '_width'
     */
    Raster (const Raster& _raster,  /**<  Source Raster                             */
            int32         _row,     /**<  Starting Row in '_raster' to copy from.             */
            int32         _col,     /**<  Starting Col in '_raster' to copy from.             */
            int32         _height,  /**<  Height of resultant raster. Will start from '_row'  */
            int32         _width    /**<  Width of resultant raster.                          */
           );

    /**
     * Constructs a Raster that will be as large as the specified '_mask'  at the point
     * specified by '_row' and '_col'.
     * \see MaskTypes
     */
    Raster (const Raster&   _raster,
            MaskTypes       _mask,    /*!< \see MaskTyps  Generated 'Raster' with be the of the mask.   */
            int32           _row,
            int32           _col
           );

    /**
     *@brief  Constructs a Raster image from by reading an existing image File such as a BMP file.
     *@details  Will read from the specified file (fileName) the existing image. If the load fails then
     *          the contents of this object will be undefined.
     *@param[in]  fileName  Name of Image file to read.
     *@param[out] validFile  If image successfully loaded will be set to 'True' otherwise 'False'.
     */
    Raster (const KKStr&  fileName,    /*!<  \param  fileName  name of image file to load/                    */
            bool&         validFile    /*!<  \param  validFile will return true if image successfully loaded. */
           );


    /**
     *@brief  Construct a raster object that will utilize a image already in memory.
     *@details  This instance will not own the raster data. It will only point to it.
     */
    Raster (int32    _height,
            int32    _width,
            uchar*   _grayScaleData,
            uchar**  _grayScaleRows
           );

    /**
     *@brief  Construct a Grayscale Raster object using provided raw data,
     */
    Raster (int32         _height,
            int32         _width,
            const uchar*  _grayScaleData
           );

    /**
     *@brief  Construct a Color Raster object using provided raw data,
     */
    Raster (int32         _height,
            int32         _width,
            const uchar*  _redChannel,
            const uchar*  _greenChannel,
            const uchar*  _blueChannel
           );


    virtual
    ~Raster ();


    /** 
     *@brief  Sets an existing instance to specific Raster data of a grayscale image. 
     *@details  This instance of 'raster' will take ownership of '_grayScaleData' and '_grayScaleRows'.
     */
    void  Initialize (int32    _height,
                      int32    _width,
                      uchar*   _grayScaleData,
                      uchar**  _grayScaleRows,
                      bool     _takeOwnership
                     );

    void  Initialize (int32    _height,
                      int32    _width,
                      uchar*   _redArea,
                      uchar**  _red,
                      uchar*   _greenArea,
                      uchar**  _green,
                      uchar*   _blueArea,
                      uchar**  _blue,
                      bool     _takeOwnership
                     );

    float    CentroidCol ();  /**< \return  returns  the centroid's column  */
    float    CentroidRow ();  /**< \return  returns  the centroid's row     */

    bool     Color ()        const  {return  color;}

    const
    KKStr&   FileName ()     const  {return fileName;}

    int32    ForegroundPixelCount () const {return foregroundPixelCount;}

    int32    Height    ()    const  {return  height;}
    uchar    MaxPixVal ()    const  {return  maxPixVal;}
    uchar**  Rows      ()    const  {return  green;}
    int32    TotPixels ()    const  {return  totPixels;}
    int32    Width     ()    const  {return  width;}

    uchar**  Red   ()        const  {return  red;}       /**< returns a pointer to two dimensional array for 'Red' color channel. */
    uchar**  Green ()        const  {return  green;}     /**< returns a pointer to two dimensional array for 'Green' color channel. */
    uchar**  Blue  ()        const  {return  blue;}      /**< returns a pointer to two dimensional array for 'Blue' color channel. */

    uchar*   RedArea   ()    const  {return  redArea;}
    uchar*   GreenArea ()    const  {return  greenArea;}
    uchar*   BlueArea  ()    const  {return  blueArea;}

    float*   FourierMagArea ()  const {return fourierMagArea;}

    uchar    BackgroundPixelTH    () const {return backgroundPixelTH;}
    uchar    BackgroundPixelValue () const {return backgroundPixelValue;}
    uchar    ForegroundPixelValue () const {return foregroundPixelValue;}

    int32    TotalBackgroundPixels () const;

    void     BackgroundPixelTH    (uchar          _backgroundPixelTH)    {backgroundPixelTH    = _backgroundPixelTH;}
    void     BackgroundPixelValue (uchar          _backgroundPixelValue) {backgroundPixelValue = _backgroundPixelValue;}
    void     ForegroundPixelCount (int32          _foregroundPixelCount) {foregroundPixelCount = _foregroundPixelCount;}
    void     ForegroundPixelValue (uchar          _foregroundPixelValue) {foregroundPixelValue = _foregroundPixelValue;}
    void     FileName             (const KKStr&   _fileName)             {fileName             = _fileName;}
    void     MaxPixVal            (uchar          _maxPixVal)            {maxPixVal            = _maxPixVal;}
    void     WeOwnRasterData      (bool           _weOwnRasterData)      {weOwnRasterData      = _weOwnRasterData;}

    uchar         GetPixelValue (int32 row,  int32 col)  const;


    void          GetPixelValue (int32   row,
                                 int32   col,
                                 uchar&  r,
                                 uchar&  g,
                                 uchar&  b
                                )  const;


    void          GetPixelValue (int32       row,
                                 int32       col,
                                 PixelValue& p
                                )  const;


    uchar         GetPixelValue (ColorChannels  channel,
                                 int32          row,
                                 int32          col
                                )  const;


    void          SetPixelValue (const Point&       point,
                                 const PixelValue&  pixVal
                                );


    void          SetPixelValue (int32  row,
                                 int32  col,
                                 uchar  pixVal
                                );


    void          SetPixelValue (int32              row,
                                 int32              col,
                                 const PixelValue&  pixVal
                                );


    void          SetPixelValue (int32  row,
                                 int32  col,
                                 uchar  r,
                                 uchar  g,
                                 uchar  b
                                );


    void          SetPixelValue (ColorChannels  channel,
                                 int32          row,
                                 int32          col,
                                 uchar          pixVal
                                );


    /**
     *@brief returns true if there are any forground pixels within 'edgeWidth' pixels of the top, bottom, left, or right edges of the image.
     */
    bool          AreThereEdgePixels (int32 edgeWidth);


    /**
     *@brief Returns a image that is the result of a BandPass using Fourier Transforms.
     *@details A 2D Fourier transform is performed. The range specified is from 0.0 to 1.0 where range is
     * determined from the center of the image to the farthest corner where the center is 0.0 and the farthest
     * corner is 1.0. Pixels in the resultant 2D Transform that are "NOT" in the specified range are set to
     * 0.0.  A reverse transform is then performed and the resultant image is returned.
     *@param[in] lowerFreqBound  Lower range of frequencies to retain; between 0.0 and 1.0.
     *@param[in] upperFreqBound  Upper range of frequencies to retain; between 0.0 and 1.0.
     *@return The result image.
     */
    RasterPtr     BandPass (float  lowerFreqBound,    /**< Number's between 0.0 and 1.0  */
                            float  upperFreqBound     /**< Represent fraction.           */
                           );


    RasterPtr     BinarizeByThreshold (uchar  min,
                                       uchar  max
                                      );

    /**
     *@brief  Return the ID of the blob that the specified pixel location belongs to.
     *@details If a connected component (ExtractBlobs) was performed on this image then the pixels that belong
     *         to blobs were assigned a blob ID. These ID's are retained with the original image in 'blobIds'.
     *@param[in] row Row in image.
     *@param[in] col Column in image.
     *@returns BlobID of pixel location or -1 of does not belong to a blob.
     *@see ExtractBlobs
     */
    int32         BlobId (int32  row,  int32  col)  const;


    /**
     *@brief  Builds a 2d Gaussian kernel
     *@details Determines the size of the gaussian kernel based off the specified sigma parameter. Returns a
     *         2D matrix representing the kernel which will have 'len' x 'len' dimensions. The caller will be
     *         responsible for deleting the kernel.
     *@param[in]  sigma   parameter used to control the width of the gaussian kernel
     *@Returns A 2 dimension matrix representing the gaussian kernel.
     */
    static
    MatrixPtr     BuildGaussian2dKernel (float  sigma);


    int32         CalcArea ();

    
    /**
     *@brief Calculates the occurrence of different intensity levels.
     *@details The pixel values 0-255 are split into 8 ranges.  (0-31), (32-63),  (64-95), (96-127), (128-159),
     *         (160-191), (192-223), (224-255).  The background range (0-31) are not counted.
     *@param[out]  area Total number of foreground pixels in the image.
     *@param[out]  intensityHistBuckets  An array of 8 buckets where each bucket represents an intensity range.
     */
    void          CalcAreaAndIntensityHistogram (int32&  area,
                                                 uint32  intensityHistBuckets[8]
                                                );

    /**
     *@brief  Calculates a Intensity Histogram including Background pixels in the image.
     *@details All background pixels that are inside the image will also be included in the counts. This is done
     *         by building a mask on the original image then performing a FillHole operation. This mask is then
     *         used to select pixels for inclusion in the histogram.
     */
    void          CalcAreaAndIntensityHistogramWhite (int32&  area,
                                                      uint32  intensityHistBuckets[8]
                                                     );
    


    /**
     *@brief  Calculates both Intensity Histograms, one not including internal background pixels and one with
     *        plus size and weighted size.
     *@details
     *        This method incorporates the functionality of several methods at once. The idea being that while
     *        we are iterating through the raster image we might as well get all the data we can so as to save
     *        total overall processing time.
     *@code
     * Histogram Ranges:
     *   0:   0  - 31    4: 128 - 159
     *   1:  31  - 63    5: 192 - 223
     *   2:  64  - 95    6: 192 - 223
     *   3:  96 - 127    7: 224 - 255
     *@endcode
     *
     *@param[out]  area          Number of foreground pixels.
     *@param[out]  weightedSize  Area that takes intensity into account. The largest pixel will have a value of 1.0.
     *@param[out]  intensityHistBuckets A 8 element array containing a histogram by intensity range.
     *@param[out]  areaWithWhiteSpace  Area including any whitespace enclosed inside the image.
     *@param[out]  intensityHistBucketsWhiteSpace  A 8 element array containing a histogram by intensity range,
     *             with enclosed whitespace pixels included.
     */
    void          CalcAreaAndIntensityFeatures (int32&   area,
                                                float&   weighedSize,
                                                uint32   intensityHistBuckets[8],
                                                int32&   areaWithWhiteSpace,
                                                uint32   intensityHistBucketsWhiteSpace[8]
                                               );


    /**
     *@brief  Calculates both Intensity Histograms, one not including internal background pixels and one with
     *         plus size and weighted size.
     *@details 
     *        This method incorporates the functionality of several methods at once. The idea being that while
     *        we are iterating through the raster image we might as well get all the data we can so as to save
     *        total overall processing time.
     *@code
     * Histogram Ranges:
     *   0:   0  - 31    4: 128 - 159
     *   1:  31  - 63    5: 192 - 223
     *   2:  64  - 95    6: 192 - 223
     *   3:  96 - 127    7: 224 - 255
     *@endcode
     *
     *@param[out]  area          Number of foreground pixels.
     *@param[out]  weightedSize  Area that reflects intensity; the largest pixel will have a value of 1.0.
     *@param[out]  intensityHistBuckets A 8 element array containing a histogram by intensity range.
     */
    void          CalcAreaAndIntensityFeatures (int32&  area,
                                                float&  weighedSize,
                                                uint32  intensityHistBuckets[8]
                                               );


    void          CalcCentroid (int32&    size,
                                int32&    weight,
                                float&  rowCenter,  
                                float&  colCenter,
                                float&  rowCenterWeighted,
                                float&  colCenterWeighted
                               );


    void          CalcOrientationAndEigerRatio (float&  eigenRatio,
                                                float&  orientationAngle
                                               );

    float         CalcWeightedArea ()  const;


    float         CenMoment (int32  p, 
                             int32  q, 
                             float  ew, 
                             float  eh
                            )  const;

    void          CentralMoments (float  features[9])  const;

    float         CenMomentWeighted (int32   p, 
                                     int32   q, 
                                     float   ew, 
                                     float   eh
                                    ) const;

    void          CentralMomentsWeighted (float  features[9])  const;

    void          Closing ();

    void          Closing (MaskTypes  mask);

    void          ConnectedComponent (uchar connectedComponentDist);
    void          ConnectedComponentOrig ();
    void          ConnectedComponent8Conected ();

    RasterPtr     CreateDialatedRaster ()  const;
    void          Dialation ();


    RasterPtr     CreateDialatedRaster (MaskTypes  mask)  const;
    void          Dialation (MaskTypes  mask);

    RasterPtr     CreateErodedImage (MaskTypes  mask)  const;


    RasterPtr     CreateGrayScale ()  const;

    /**
     *@brief Creates a grayscale image using a KLT Transform with the goal of weighting in favor the color
     * channels with greatest amount of variance.
     *@details The idea is to weight each color channel by the amount of variance. This is accomplished by
     *  producing a covariance matrix of the three color channels and then taking the Eigen vector with the
     *  largest eigen value and using its components to derive weights for each channel for the conversion 
     *  from RGB to grayscale.
     */
    RasterPtr     CreateGrayScaleKLT ()  const;

    /**
     *@brief  Same as 'CreateGrayScaleKLT' except it will only take into account 
     *        pixels specified by the 'mask' image.
     *@param[in]  mask  Raster object where pixels that are greater than 'backgroundPixelTH' are to be considered.
     */
    RasterPtr     CreateGrayScaleKLTOnMaskedArea (const Raster&  mask)  const;

    static
    RasterPtr     CreatePaddedRaster (BmpImage&  image,
                                      int32      padding
                                     );

    RasterPtr     CreateSmoothImage (int32  maskSize = 3)  const;
    
    RasterPtr     CreateSmoothedMediumImage (int32 maskSize)  const;

    RasterPtr     CreateGaussianSmoothedImage (float sigma)  const;

    /**
     *@brief Produces a color image using the 'greenArea' channel, assuming that each unique value will be
     *       assigned a unique color.
     *@details
     *        Assuming that each value in the grayscale channel(GreenArea) will be assigned  a different color
     *        useful for image created by  "SegmentorOTSU::SegmentImage".
     *@see Raster::CreateGrayScaleKLT
     */
    RasterPtr     CreateColorImageFromLabels ();


    /**
     *@brief Returns image where each blob is labeled with a different color.
     *@details
     *        Only useful if 'ExtractBlobs' was performed on this instance. Eight different colors are used and
     *        they are selected by the modules of the blobId(blobId % 8). Assignments are 0:Red, 1:Green,
     *        2:Blue, 3:Yellow, 4:Orange, 5:Magenta, 6:Purple, 7:Teal.
     */
    RasterPtr     CreateColorWithBlobsLabeldByColor (BlobListPtr  blobs);  /**< Only useful if 'ExtractBlobs' was performed on this instance, the returned image will be color with each blob labeled a different color. */


    /**
     *@brief Returns a copy of 'origImage' where only the blobs specified in 'blobs' are copied over.
     *@details 
     *@code
     *  Example:
     *      origImage = image that we want to segment and get list of discrete blobs from.
     *      RasterPtr  segmentedImage = origImage->SegmentImage ();
     *      BlobListPtr  blobs = segmentedImage->ExtractBlobs (1);
     *      RasterPtr  imageWithBlobOnly = segmentedImage->CreateFromOrginalImageWithSpecifidBlobsOnly (blobs);
     *@endcode
     *@param[in]  origImage  Image that this instance was derived for, must have same dimensions.
     *@param[in]  blobs      Blobs that you want copied over from.
     *@returns Image consisting of specified blobs only.
     */
    RasterPtr     CreateFromOrginalImageWithSpecifidBlobsOnly (RasterPtr    origImage,
                                                               BlobListPtr  blobs
                                                              );

    /** @brief Draw a circle who's center is at 'point' and radius in pixels is 'radius'using color 'color'. */
    void          DrawCircle (const Point&       point,
                              int32              radius,
                              const PixelValue&  color
                             );


    void          DrawCircle (float              centerRow, 
                              float              centerCol, 
                              float              radius,
                              const PixelValue&  pixelValue
                             );


    void          DrawCircle (float              centerRow, 
                              float              centerCol, 
                              float              radius,
                              float              startAngle,   /**< Start and End angles should be given in radians */
                              float              endAngle,     /**< Where the angles are with respect to the compass */
                              const PixelValue&  pixelValue
                             );


    void          DrawDot (const Point&       point, 
                           const PixelValue&  color,
                           int32              size
                          );


    void          DrawGrid (float              pixelsPerMinor,
                            uint32             minorsPerMajor,
                            const PixelValue&  hashColor,
                            const PixelValue&  gridColor
                           );


    void          DrawLine (int32 bpRow,  int32 bpCol,
                            int32 epRow,  int32 epCol
                           );


    void          DrawLine (int32 bpRow,    int32 bpCol,
                            int32 epRow,    int32 epCol,
                            uchar pixelVal
                           );


    void          DrawLine (const Point&  beginPoint,
                            const Point&  endPoint,
                            uchar         pixelVal
                           );

    void          DrawLine (const Point&       beginPoint,
                            const Point&       endPoint,
                            const PixelValue&  pixelVal
                           );


    void          DrawLine (int32 bpRow,    int32 bpCol,
                            int32 epRow,    int32 epCol,
                            uchar  r,
                            uchar  g,
                            uchar  b
                           );


    void          DrawLine (int32 bpRow,    int32 bpCol,
                            int32 epRow,    int32 epCol,
                            uchar  r,
                            uchar  g,
                            uchar  b,
                            float  alpha
                           );


    void          DrawLine (int32  bpRow,    int32 bpCol,
                            int32  epRow,    int32 epCol,
                            PixelValue  pixelVal
                           );

    void          DrawLine (int32  bpRow,    int32 bpCol,
                            int32  epRow,    int32 epCol,
                            PixelValue  pixelVal,
                            float       alpha
                           );


    void          DrawConnectedPointList (Point              offset,
                                          const PointList&   borderPixs,
                                          const PixelValue&  pixelValue,
                                          const PixelValue&  linePixelValue
                                         );

    void          DrawPointList (const PointList&   borderPixs,
                                 const PixelValue&  pixelValue
                                );

    void          DrawPointList (Point              offset,
                                 const PointList&   borderPixs,
                                 const PixelValue&  pixelValue
                                );


    void          DrawPointList (const PointList&  borderPixs,
                                 uchar             redVal,
                                 uchar             greenVal,
                                 uchar             blueVal
                                );


    void          DrawPointList (Point             offset,
                                 const PointList&  borderPixs,
                                 uchar             redVal,
                                 uchar             greenVal,
                                 uchar             blueVal
                                );

    /** @brief reduces image to edge pixels only.  */
    void          Edge ();

    /** @brief removes spurs from image. */
    void          ErodeSpurs ();

    void          Erosion ();

    void          Erosion (MaskTypes  mask);
    void          ErosionChanged (MaskTypes  mask, int32 row, int32 col);
    void          ErosionChanged1 (MaskTypes  mask, int32 row, int32 col);
    void          ErosionBoundary (MaskTypes  mask, int32 blobrowstart, int32 blobrowend, int32 blobcolstart, int32 blobcolend);

    /**
     *@brief  Extracts a specified blob from this image; useful to extract individual detected blobs.
     *@details  The 'ExtractBlobs' method needs to have been performed on this instance first.  You
     * would use this method after calling 'ExtractBlobs'. The extracted image will be of the same 
     * dimensions as the original image except it will extract the pixels that belong to the specified
     * blob only.
     *@code
     *   // Example of processing extracted blobs
     *   void  ProcessIndividulConectedComponents (RasterPtr  image)
     *   {
     *     BlobListPtr blobs = image->ExtractBlobs (3);
     *     BlobList::iterator  idx;
     *     for  (idx = blobs->begin ();  idx != end ();  ++idx)
     *     {
     *       RasterPtr  individuleBlob = image->ExtractABlob (*idx);
     *       DoSomethingWithIndividuleBlob (individuleBlob);
     *       delete  individuleBlob;
     *       individuleBlob = NULL;
     *     )
     *     delete  blobs;
     *     blobs = NULL;
     *   }
     *@endcode
     */
    RasterPtr     ExtractABlob (const BlobPtr  blob)  const;


    /**
     *@brief  Extracts a specified blob from this image into a tightly bounded image.
     *@details
     *        Similar to 'ExtractABlob' except that the returned image will have the dimension necessary
     *        to contain the specified blob with the specified number of padded row and columns.
     */
    RasterPtr     ExtractABlobTightly (const BlobPtr  blob,
                                       int32          padding
                                       ) const;


    /**
     *@brief Will extract a list of connected components from this instance.
     *@details
     *        Will perform a connected component analysis and label each individual blob. A list of blob
     *        descriptors will be returned. These blob descriptors can then be used to access individual
     *        blobs. See 'ExtractABlob' for an example on how to use this method. The 'ForegroundPixel'
     *        method is used to determine if a given pixel is foreground or background.
     *
     *@param[in]  dist  The distance in pixels that two different pixel locations have to be for them to
     *            be considered connected. "dist = 1" would indicate that two pixels have to be directly
     *            connected.
     *@returns A list of Blob descriptor instances.
     *@see  ExtractABlob, ExtractABlobTightly, Blob
     */
    BlobListPtr   ExtractBlobs (int32  dist);

    
    /**
     *@brief Will return a grayscale image consisting of the specified color channel only.
     */
    RasterPtr     ExtractChannel (ColorChannels  channel);

    RasterPtr     FastFourier ()  const;
    RasterPtr     FastFourierKK ()  const;

    void          FillHole ();

    /**
     *@brief  Will paint the specified blob with the specified color
     *@details
     *@code
     * Example Use:
     *    BlobListPtr  blobs = srcImage->ExctractBlobs (1);
     *    RasterPtr  labeledColorImage = new Raster (srcImage->Height (), srcImage->Width (), true);
     *    BlobList::iterator  idx;
     *    for  (idx = blobs->begin ();  idx != blobs->end ();  ++idx)
     *    {
     *      BlobPtr  blob = *idx;
     *      labeledColorImage->FillBlob (srcImage, blob, PixelValue::Red);
     *    }
     *@endcode
     *@param[in] origImage  The image where the blob was extracted from.
     *@param[in] blob  The specific blob that you want to fill in/ paint.
     *@param[in] Color that is to be filled in.
     */
    void          FillBlob (RasterPtr   origImage,
                            BlobPtr     blob,
                            PixelValue  color
                           );

    void          FillRectangle (int32              tlRow,
                                 int32              tlCol,
                                 int32              brRow,
                                 int32              brCol,
                                 const PixelValue&  fillColor
                                );

    void          FindBoundingBox (int32&  tlRow,
                                   int32&  tlCol,
                                   int32&  brRow,
                                   int32&  brCol
                                  )  const;

    void          FourierExtractFeatures (float  fourierFeatures[5])  const;

    bool          ForegroundPixel (int32  row,  
                                   int32  col
                                  )  const;


    uchar**       GetSubSet (uchar** _src,
                             int32   _row,
                             int32   _col,
                             int32   _height,
                             int32   _width
                            )  const;

    RasterPtr     HalfSize ();

    HistogramPtr  Histogram (ColorChannels  channel)  const;

    RasterPtr     HistogramEqualizedImage ()  const;

    RasterPtr     HistogramEqualizedImage (HistogramPtr  equalizedHistogram) const;

    HistogramPtr  HistogramGrayscale ()  const;

    RasterPtr     HistogramImage (ColorChannels  channel)  const;

    RasterPtr     HistogramGrayscaleImage ()  const;

    int32         MemoryConsumedEstimated ()  const;

    void          Opening ();

    void          Opening (MaskTypes mask);

    RasterPtr     Padded (int32 padding);  // Creates a Padded raster object.

    RasterPtr     ReversedImage ();

    void          ReverseImage ();   // Reverse the image Foreground and Background.
    RasterPtr     ReduceByEvenMultiple (int32  multiple)  const;

    RasterPtr     ReduceByFactor (float factor)  const;  //  0 < factor <= 1.0  ex: 0.5 = Make raster half size

    /**
     *@brief  Locates most complete blob; that is the one with the largest (Height x Width); and removes all
     * other images from the blob.
     */
    void          ReduceToMostCompleteBlob (uchar connectedComponentDist);

    RasterPtr     Rotate (float  turnAngle);

    RasterPtr     SegmentImage (bool  save = false);

    RasterPtr     SobelEdgeDetector ();

    RasterListPtr SplitImageIntoEqualParts (int32 numColSplits,
                                            int32 numRowSplits
                                           )  const;

    RasterPtr     SwapQuadrants ()  const;

    /**
     *@brief Thresholds image in HSI space.
     *@details
     *        Returns an image with only the pixels that are within a specified distance in HSI space to the
     *        supplied HSI parameters.  All pixels that are not within the specified distance will be set to
     *        'flagValue'.
     *@param[in] thresholdH  Hue in radians(0.0 <-> 2Pie).
     *@param[in] thresholdS  Saturation (0.0 <-> 1.0).
     *@param[in] thresholdI  Intensity (0.0 <-> 1.0).
     *@param[in] distance    Threshold Distance (0.0 <-> 1.0) that a pixel must be within in HSI space to be included.
     *@param[in] flagValue   PixelValue to set for pixels that are NOT within 'distance' of threshold.
     *@returns A image where pixels that are within the threshold will retain their original pixel values and 
     * the ones that are not will be set to 'flagValue'.
     */
    RasterPtr     ThresholdInHSI (float              thresholdH,
                                  float              thresholdS, 
                                  float              thresholdI, 
                                  float              distance,
                                  const PixelValue&  flagValue
                                 );

    RasterPtr     ThinContour ();

    RasterPtr     TightlyBounded (uint32 borderPixels)  const;

    RasterPtr     ToColor ()  const;

    /**
     *@brief  Sets all pixels that are in the Background Range ov values to BackgroundPixelValue.
     */
    void          WhiteOutBackground ();


    /**
     *@brief Compresses the image in Raster using zlib library and returns a pointer to compressed data.
     *@details Will first write Rater data to a buffer that will be compressed by the Compressor class using the zlib library.
     *@code
     *        Buffer Contents:
     *           0 - 3: Height:  high order to low order
     *           4 - 7:    Width:   high order to low order
     *           8 - 8:    Color    0 = Grayscale,  1 = Color
     *           9 -  :   Green Channel  (Height * Width bytes)
     *           xxxxx:   Red  Channel, if color image.
     *           xxxxx:   Blue Channel, if color image.
     *@endcode
     *@param[out]  compressedBuffLen Length of the compressed buffer returned.
     *@return pointer to compressed data.
     */
    uchar*        ToCompressor (uint32&  compressedBuffLen)  const;


    virtual
      RasterPtr  AllocateARasterInstance (int32  height,
                                          int32  width,
                                          bool   color
                                         )  const;

    virtual
      RasterPtr  AllocateARasterInstance (const Raster& r)  const;

    virtual
      RasterPtr  AllocateARasterInstance (const Raster& _raster,  /**<  Source Raster                             */
                                          int32         _row,     /**<  Starting Row in '_raster' to copy from.             */
                                          int32         _col,     /**<  Starting Col in '_raster' to copy from.             */
                                          int32         _height,  /**<  Height of resultant raster. Will start from '_row'  */
                                          int32         _width    /**<  Width of resultant raster.                          */
                                         )  const;

  private:
    void   AllocateBlobIds ();

    void   AllocateImageArea ();

    void   AllocateFourierMagnitudeTable ();

    void   CleanUpMemory ();

    inline
    bool   BackgroundPixel (uchar  pixel)  const;


    inline
    bool   BackgroundPixel (int32  row,
                            int32  col
                           )  const;


    // Used by the Gaussian Smoothing algorithm.
    void   BuildGaussian2dKernel (float     sigma,
                                  int32&    len,
                                  float**&  kernel
                                 )  const;


    inline
    void   CalcDialatedValue (int32   row,
                              int32   col,
                              int32&  totVal,
                              uchar&  numNeighbors
                             )  const;


    inline
    bool   CompletlyFilled3By3 (int32  row, 
                                int32  col
                               )  const;


    void  DeleteExistingBlobIds ();


    void   FillHoleGrow (int32  _row, 
                         int32  _col
                        );

    bool     Fit (MaskTypes  mask,
                  int32     row, 
                  int32     col
                 )  const;

    
    bool     ForegroundPixel (uchar  pixel)  const;


    uchar    Hit (MaskTypes  mask,
                  int32      row, 
                  int32      col
                 )  const;

    float    Moment (int32 p, 
                     int32 q
                    )  const;


    float   MomentWeighted (int32 p, 
                            int32 q
                           )  const;


    inline
    int32    NearestNeighborUpperLeft (int32  centRow,
                                       int32  centCol,
                                       int32  dist
                                      );

    inline
    int32    NearestNeighborUpperRight (int32  centRow,
                                        int32  centCol,
                                        int32  dist
                                       );


    void     SmoothImageChannel (uchar**  src,
                                 uchar**  dest,
                                 int32    maskSize
                                )  const;

    void     SmoothUsingKernel (Matrix&  kernel,
                                uchar**  src,
                                uchar**  dest
                               )  const;



  protected:
    uchar    backgroundPixelValue;
    uchar    backgroundPixelTH;
    int32**  blobIds;              // Used when searching for connected components
    float    centroidCol;
    float    centroidRow;
    bool     color;
    KKStr    fileName;
    int32    foregroundPixelCount;
    uchar    foregroundPixelValue;
    float**  fourierMag;           /**< Only used if image is result of a Fourier Transform   */
    float*   fourierMagArea;       /**< Only used if image is result of a Fourier Transform   */
    int32    height;
    uchar    maxPixVal;
    int32    totPixels;
    bool     weOwnRasterData;
    int32    width;

    uchar*   redArea;              // Each color channel is allocated as a single block
    uchar*   greenArea;            // for 2 dimensional access use corresponding 2d variables
    uchar*   blueArea;             // red for redAreas, green for grenArea.  If GrayScale
                                   // image then only green channel is used.

    // The next three variables provide row indexing into there respective color channels. For performance
    // and simplicity purposes I allocate each channel in a continuous block of memory but to allow for
    // simple accessing by 'row' and 'col' I create the following 3 variables. Depending on what you
    // are trying to do you could use the appropriate variable.
    uchar**  red;                  // Provides row indexes into 'redArea'.
    uchar**  green;                // Provides row indexes into 'greenArea'.
    uchar**  blue;                 // Provides row indexes into 'blueArea'.


    //  The following code is being added to support the tracking down of memory leaks in Raster.
    static std::map<RasterPtr, RasterPtr>  allocatedRasterInstances;
    static volatile GoalKeeperPtr  goalKeeper;
    static volatile bool           rasterInitialized;
    static void  Initialize ();
    static void  FinalCleanUp ();
    static void  AddRasterInstance (const RasterPtr  r);
    static void  RemoveRasterInstance (const RasterPtr  r);
  public:
    static void  PrintOutListOfAllocatedrasterInstances ();
  };  /* Raster */


  typedef  Raster::RasterPtr  RasterPtr;


  typedef  struct  
  {
    int32  row;
    int32  col;
  }  MovDir;




  class  RasterList:  public  KKQueue<Raster>
  {
  public:
    RasterList (bool  owner, 
                int32 growthRate = 10
               ):
        KKQueue<Raster> (owner, growthRate)
        {}

  private:
    RasterList (const RasterList&  rasterList):
        KKQueue<Raster> (rasterList)
        {}

  public:
    RasterList (const RasterList&  rasterList,
                bool               _owner
               ):
        KKQueue<Raster> (rasterList, _owner)
        {}
        
        
   RasterPtr  CreateSmoothedFrame ();
  };


  typedef  RasterList*  RasterListPtr;

}  /* namespace KKU; */
#endif
