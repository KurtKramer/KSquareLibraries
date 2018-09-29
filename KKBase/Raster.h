/* Raster.h -- Class that one raster image.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#if  !defined(_RASTER_)
#define _RASTER_

#include <map>


#include "KKBaseTypes.h"
#include "KKQueue.h"
#include "MorphOp.h"
#include "KKStr.h"
#include "PixelValue.h"
#include "Point.h"


namespace KKB
{
  /**
   *@file  Raster.h
   *@author  Kurt Kramer
   *@details
   *@code
   ***************************************************************************
   **                                Raster                                  *
   **                                                                        *
   **  Supports image Morphological operations such as Dilation, Opening,    *
   **  Closing and other operations.                                         *
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
  template<typename T>  class Matrix;
  typedef  Matrix<float>    MatrixF;
  typedef  Matrix<double>   MatrixD;
  typedef  Matrix<float>*   MatrixFPtr;
  typedef  Matrix<double>*  MatrixDPtr;
#endif

  #if  !defined(_GoalKeeper_Defined_)
	class  GoalKeeper;
	typedef  GoalKeeper*  GoalKeeperPtr;
  #endif


  typedef  MorphOp::MaskTypes      MaskTypes;
  typedef  MorphOp::StructureType  StructureType;


  enum class ColorChannels: int
  {
    Red = 0,
    Green = 1,
    Blue = 2
  };

  std::ostream&  operator<< (std::ostream& lhs, ColorChannels rhs);
  KKB::KKStr&    operator<< (KKB::KKStr& lhs,   ColorChannels rhs);

  KKB::KKStr     ColorChannelToKKStr   (ColorChannels  c);
  ColorChannels  ColorChannelFromKKStr (const KKStr&   s);
  
  class  RasterList;
  typedef  RasterList*  RasterListPtr;

  class  BmpImage;
  typedef  BmpImage*  BmpImagePtr;


  #ifndef  _HISTOGRAM_
  class Histogram;
  typedef  Histogram*  HistogramPtr;
  #endif


  /**
   *@class Raster 
   *@brief  A class that is used by to represent a single image in memory.  
   *@details  This class supports morphological operations and other tasks and can handle either 
   *          Gray-scale or Color. The default is Gray-scale unless otherwise specified. Each color channel will be
   *          allocated as one continuous block of memory.  If the image is only gray-scale the Green Channel (G) will 
   *          be used leaving the Red and Blue channels set to NULL. Access to individual pixels through is through 
   *          methods that will ensure memory integrity. If required can also access the pixel data directly in memory.
   *          Each channel can be accessed as a one or two dimensional array. For example the green channel can be 
   *          accesses as either "GreenArea ()" which returns a pointer to a one dimensional array or "Green ()" which
   *          returns a two dimensional array. The imagery is stored in the one dimensional array while the two 
   *          dimensional is a list of pointers to the beginning of each row.
   *@see Blob
   *@see ContourFollower
   *@see ConvexHull
   *@see PixelValue
   *@see Point
   *@see PointList
   *@see RasterList()
   */
  class  Raster
  {
  public:
    typedef  Raster*  RasterPtr;
    typedef  Raster  const  RasterConst;
    typedef  RasterConst*   RasterConstPtr;

    Raster ();

    Raster (const Raster&  _raster);  /**< @brief Copy Constructor */

    /**
     *@brief  Constructs a blank image with given dimensions.  
     *@details  The third parameter determines whether it will be a color or  image,  If a Color
     *          image then all three color channel will be set to = 255 which stands for white.  If
     *          color set to false (grayscale); only green channel will be used and value of '0' = Background 
     *          and '255'= foreground. When saving grayscale to a image file such as a BMP format 
     *          file the pixel value of 0 will point to the color of value of (255, 255, 255) and  
     *          pixel value 255 will point to the color value of (0, 0, 0). This way when displaying 
     *          the image background will appear as white.  See [BMPImage.h].
     */
    Raster (kkint32 _height,
            kkint32 _width,
            bool    _color = false
           );

    /**
     *@brief  Constructs a Raster from a BMP image loaded from disk.
     *@details If BMP Image is a gray-scale value pixel values will be reversed. See description of
     *         constructor.
     */
    Raster (const BmpImage&  _bmpImage);


    /**
     *@brief Constructs a new Raster using a subset of the specified Raster as its source. The
     *      dimensions of the resultant raster will be '_height', and '_width'
     */
    Raster (const Raster& _raster,  /**<  Source Raster                             */
            kkint32       _row,     /**<  Starting Row in '_raster' to copy from.             */
            kkint32       _col,     /**<  Starting Col in '_raster' to copy from.             */
            kkint32       _height,  /**<  Height of resultant raster. Will start from '_row'  */
            kkint32       _width    /**<  Width of resultant raster.                          */
           );

    Raster (const Raster& _raster, const Point& topLeft, const Point& botRight);

    /**
     *@brief Constructs a Raster that will be the same size as the specified '_mask' with the top left specified by '_row' and '_col'.
     *@details The Height and Width of the resultant image will come from the bias of the specified mask.  The Image data will come from
     * the specified raster using '_row' and '_col' to specify the top left column.
     *@param[in]  _raster  Source Raster to extract data from.
     *@param[in]  _mask  Used to derive height and with of resultant image.
     *@param[in]  _row  Starting row where image data is to be extracted from.
     *@param[in]  _col  Starting column where image data is to be extracted from.
     *@see MorphOp::MaskTypes
     */
    Raster (const Raster&  _raster,
            MaskTypes      _mask,
            kkint32        _row,
            kkint32        _col
           );

    /**
     *@brief  Constructs a Raster image from by reading an existing image File such as a BMP file.
     *@details  Will read from the specified file (fileName) the existing image.  If the load fails then 
     *          the contents of this object will be undefined.
     *@param[in]  fileName  Name of Image file to read.
     *@param[out] validFile  If image successfully loaded will be set to 'True' otherwise 'False'.
     */
    Raster (const KKStr&  fileName,    /**<  @param  fileName  name of image file to load/                    */
            bool&         validFile    /**<  @param  validFile will return true if image successfully loaded. */
           );


    /**
     *@brief  Construct a raster object that will utilize a image already in memory.
     *@details  This instance will NOT OWN the raster data; it will only point to it.  That means when this instance 
     * is destroyed the raster data will still be left intact.
     *@param[in]  _height  Height of image.
     *@param[in]  _width   Width of image.
     *@param[in]  _Data   Source gray-scale raster data; needs to be continuous and of length (_height * _width) with data 
     *                             stored row major.
     *@param[in]  _Rows   Two dimensional array where each entry will point into the respective image row data in '_Data'.
     */
    Raster (kkint32    _height,
            kkint32    _width,
            kkuint8*   _Data,
            kkuint8**  _Rows
           );

    /**
     *@brief  Construct a  Raster object using provided raw data.
     *@param[in] _height Image Height.
     *@param[in] _width  Image Width.
     *@param[in] _Data 8 Bit  data, Row Major, that is to be used to populate new instance.
     */
    Raster (kkint32         _height,
            kkint32         _width,
            const kkuint8*  _Data
           );

    /**
     *@brief  Construct a Color Raster object using provided raw data,
     *@param[in] _height Image Height.
     *@param[in] _width  Image Width.
     *@param[in] _redChannel   8 Bit data, Row Major, that is to be used to populate the red channel.
     *@param[in] _greenChannel 8 Bit data, Row Major, that is to be used to populate the green channel.
     *@param[in] _blueChannel  8 Bit data, Row Major, that is to be used to populate the blue channel.
     */
    Raster (kkint32         _height,
            kkint32         _width,
            const kkuint8*  _redChannel,
            const kkuint8*  _greenChannel,
            const kkuint8*  _blueChannel
           );


    virtual
    ~Raster ();



    /**
     *@brief  Lets you resize the raster dimensions; old image data will be lost.
     */
    void  ReSize (kkint32  _height, 
                  kkint32  _width,
                  bool     _color
                 );



    /** 
     *@brief  Sets an existing instance to specific Raster Data of a  image. 
     *@details  This instance of 'Raster' can take ownership of '_Data' and '_Rows' 
     * depending on '_takeOwnership'.
     *@param[in] _height Image Height.
     *@param[in] _width  Image Width.
     *@param[in] _Data The raster data that is to be used by this instance of 'Raster'; it should 
     *           be continuous data, Row Major, of length (_height * _width).
     *@param[in] _Rows  Two dimensional assessors to '_Data'; each entry will point to the
     *           respective row in '_Data' that contains that row.
     *@param[in] _takeOwnership Indicates whether this instance of 'Raster' will own the memory pointed
     *           to by '_Data' and '_Rows'; if set to true will delete them in the
     *           destructor.
     */
    void  Initialize (kkint32    _height,
                      kkint32    _width,
                      kkuint8*   _Data,
                      kkuint8**  _Rows,
                      bool       _takeOwnership
                     );

    /** 
     *@brief  Sets an existing instance to specific Raster Data of a  image. 
     *@details  This instance of 'Raster' can take ownership of '_Data' and '_Rows' 
     * depending on '_takeOwnership'.  The parameters '_redArea', '_greenArea', and '_blueArea' will point 
     * to raster data that represents their respective color channels.  This data will be 'Row-Major' and 
     * of length '(_height * _width) bytes.  For each color channel there will be a corresponding 2d accessors
     * matrix '_red', '_green', and '_blue' where each entry in these 2d arrays will point to their 
     * respective rows in the color channel.  The '_takeOwnership' parameters indicates whether this
     * instance of 'Raster' will own these memory locations.
     * '
     *@param[in] _height Image Height.
     *@param[in] _width  Image Width.
     *@param[in] _redArea The raster data representing the red channel.
     *@param[in] _red  Two dimensional accessor to '_redArea'.
     *@param[in] _greenArea  The raster data representing the green channel.
     *@param[in] _green  Two dimensional accessor to '_greenArea'.
     *@param[in] _blueArea  The raster data representing the blue channel.
     *@param[in] _blue  Two dimensional accessor to '_blueArea'.
     *@param[in] _takeOwnership  Indicates whether this instance of 'Raster' will own the supplied raster data.
     */
    void  Initialize (kkint32    _height,
                      kkint32    _width,
                      kkuint8*   _redArea,
                      kkuint8**  _red,
                      kkuint8*   _greenArea,
                      kkuint8**  _green,
                      kkuint8*   _blueArea,
                      kkuint8**  _blue,
                      bool       _takeOwnership
                     );


    /**
     *@brief  Will take ownership of 'otherRaster' raster dynamically allocated data and copy its non dynamically allocated data.
     *@details  Dynamic structures for Fourier Transform and BlobId's will be set to NULL on the 'otherRaster' instance.
     */
    void  TakeOwnershipOfAnotherRastersData (Raster&  otherRaster);

    float    CentroidCol ()  const;  /**< @return  returns  the centroid's column  */
    float    CentroidRow ()  const;  /**< @return  returns  the centroid's row     */

    bool     Color ()        const  {return  color;}

    kkint32  Divisor ()      const  {return  divisor;}

    const
    KKStr&   FileName ()     const  {return  fileName;}

    kkint32  ForegroundPixelCount () const {return foregroundPixelCount;}

    kkint32      Height    ()    const  {return  height;}
    kkuint8      MaxPixVal ()    const  {return  maxPixVal;} /**< The maximum  pixel value encountered in the image. */
    kkuint8**    Rows      ()    const  {return  green;}     /**< returns a pointer to a 2D array that allows the caller to access the raster data by row and column. */
    const KKStr& Title     ()    const  {return  title;}
    kkint32      TotPixels ()    const  {return  totPixels;} /**< The total number of pixels  (Height * Width). */
    kkint32      Width     ()    const  {return  width;}

    kkuint8**    Red       ()    const  {return  red;}        /**< returns a pointer to two dimensional array for 'Red' color channel. */
    kkuint8**    Green     ()    const  {return  green;}      /**< returns a pointer to two dimensional array for 'Green' color channel; note this is the same as 'Rows'.  */
    kkuint8**    Blue      ()    const  {return  blue;}       /**< returns a pointer to two dimensional array for 'Blue' color channel. */
    kkuint8*     RedArea   ()    const  {return  redArea;}
    kkuint8*     GreenArea ()    const  {return  greenArea;}
    kkuint8*     BlueArea  ()    const  {return  blueArea;}

    float*       FourierMagArea ()  const {return fourierMagArea;}

    kkuint8      BackgroundPixelTH    () const {return backgroundPixelTH;}
    kkuint8      BackgroundPixelValue () const {return backgroundPixelValue;}
    kkuint8      ForegroundPixelValue () const {return foregroundPixelValue;}

    kkint32  TotalBackgroundPixels () const;

    void     BackgroundPixelTH    (kkuint8          _backgroundPixelTH)    {backgroundPixelTH    = _backgroundPixelTH;}
    void     BackgroundPixelValue (kkuint8          _backgroundPixelValue) {backgroundPixelValue = _backgroundPixelValue;}
    void     Divisor              (kkint32        _divisor)              {divisor              = _divisor;}
    void     ForegroundPixelCount (kkint32        _foregroundPixelCount) {foregroundPixelCount = _foregroundPixelCount;}
    void     ForegroundPixelValue (kkuint8          _foregroundPixelValue) {foregroundPixelValue = _foregroundPixelValue;}
    void     FileName             (const KKStr&   _fileName)             {fileName             = _fileName;}
    void     MaxPixVal            (kkuint8          _maxPixVal)            {maxPixVal            = _maxPixVal;}
    void     Title                (const KKStr&   _title)                {title                = _title;}
    void     WeOwnRasterData      (bool           _weOwnRasterData)      {weOwnRasterData      = _weOwnRasterData;}

    bool     BackgroundPixel (kkint32  row,
                              kkint32  col
                             )  const;


    kkuint8         GetPixelValue (kkint32 row,  kkint32 col)  const;


    void          GetPixelValue (kkint32   row,
                                 kkint32   col,
                                 kkuint8&  r,
                                 kkuint8&  g,
                                 kkuint8&  b
                                )  const;


    void          GetPixelValue (kkint32     row,
                                 kkint32     col,
                                 PixelValue& p
                                )  const;


    kkuint8         GetPixelValue (ColorChannels  channel,
                                   kkint32        row,
                                   kkint32        col
                                  )  const;


    void          SetPixelValue (const Point&       point,
                                 const PixelValue&  pixVal
                                );


    void          SetPixelValue (kkint32  row,
                                 kkint32  col,
                                 kkuint8  pixVal
                                );


    void          SetPixelValue (kkint32            row,
                                 kkint32            col,
                                 const PixelValue&  pixVal
                                );


    void          SetPixelValue (kkint32  row,
                                 kkint32  col,
                                 kkuint8  r,
                                 kkuint8  g,
                                 kkuint8  b
                                );


    void          SetPixelValue (ColorChannels  channel,
                                 kkint32        row,
                                 kkint32        col,
                                 kkuint8        pixVal
                                );


    /**
     *@brief returns true if there are any foreground pixels within 'edgeWidth' pixels of the top, bottom, left, or right edges of the image.
     */
    bool          AreThereEdgePixels (kkint32 edgeWidth);


    /**
     *@brief Returns a image that is the result of a BandPass using Fourier Transforms.
     *@details A 2D Fourier transform is performed.  The range specified is from 0.0 to 1.0 where range is
     * determined from the center of the image to the farthest corner where the center is 0.0 and the farthest
     * corner is 1.0.  Pixels in the resultant 2D Transform that are "NOT" in the specified range are set to
     * 0.0.  A reverse transform is then performed and the resultant image is returned.
     *@param[in] lowerFreqBound  Lower range of frequencies to retain; between 0.0 and 1.0.
     *@param[in] upperFreqBound  Upper range of frequencies to retain; between 0.0 and 1.0.
     *@param[in] retainBackground
     *@return The result image.
     */
    RasterPtr     BandPass (float  lowerFreqBound,    /**< Number's between 0.0 and 1.0  */
                            float  upperFreqBound,    /**< Represent fraction.           */
                            bool   retainBackground
                           );


    RasterPtr     BinarizeByThreshold (kkuint8  min,
                                       kkuint8  max
                                      )  const;

    /**
     *@brief  Return the ID of the blob that the specified pixel location belongs to.
     *@details If a connected component (ExtractBlobs) was performed on this image then the pixels that belong
     *         to blobs were assigned a blob ID. These ID's are retained with the original image in 'blobIds'.
     *@param[in] row Row in image.
     *@param[in] col Column in image.
     *@returns BlobID of pixel location or -1 of does not belong to a blob.
     *@see ExtractBlobs
     */
    kkint32       BlobId (kkint32  row,  kkint32  col)  const;


    /**
     *@brief  Builds a 2d Gaussian kernel
     *@details Determines the size of the Gaussian kernel based off the specified sigma parameter. returns a
     *         2D matrix representing the kernel which will have 'Len' x 'Len' dimensions.  The caller will be
     *         responsible for deleting the kernel.
     *@param[in]  sigma   parameter used to control the width of the Gaussian kernel
     *@returns A 2-dimensional matrix representing the Gaussian kernel.
     */
    static
    MatrixDPtr    BuildGaussian2dKernel (float  sigma);  // Used by the Gaussian Smoothing algorithm.


    kkint32       CalcArea ();

    
    /**
     *@brief Calculates the occurrence of different intensity levels.
     *@details The pixel values 0-255 are split into 8 ranges.  (0-31), (32-63),  (64-95), (96-127), (128-159),
     *         (160-191), (192-223), (224-255).  The background range (0-31) are not counted.
     *@param[out]  area Total number of foreground pixels in the image.
     *@param[out]  intensityHistBuckets  An array of 8 buckets where each bucket represents an intensity range.
     */
    void          CalcAreaAndIntensityHistogram (kkint32&  area,
                                                 kkuint32  intensityHistBuckets[8]
                                                )
                                                  const;

    /**
     *@brief  Calculates a Intensity Histogram including Background pixels in the image.
     *@details All background pixels that are inside the image will also be included in the counts. This is done
     *         by building a mask on the original image then performing a FillHole operation. This mask is then
     *         used to select pixels for inclusion in the histogram.
     */
    void          CalcAreaAndIntensityHistogramWhite (kkint32&  area,
                                                      kkuint32  intensityHistBuckets[8]
                                                     );
    


    void          CalcAreaAndIntensityFeatures16 (kkint32&  area,
                                                  float&    weighedSize,
                                                  kkuint32  intensityHistBuckets[16]
                                                 );


    /**
     *@brief  Calculates both Intensity Histograms, one not including internal background pixels and one with
     *        plus size and weighted size.
     *@details
     *        This method incorporates the functionality of several methods at once.  The idea being that while
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
     *@param[out]  weightedSize  Area that takes intensity into account.  The largest pixel will have a value of 1.0.
     *@param[out]  intensityHistBuckets A 8 element array containing a histogram by intensity range.
     *@param[out]  areaWithWhiteSpace  Area including any whitespace enclosed inside the image.
     *@param[out]  intensityHistBucketsWhiteSpace  A 8 element array containing a histogram by intensity range,
     *             with enclosed whitespace pixels included.
     */
    void          CalcAreaAndIntensityFeatures (kkint32& area,
                                                float&   weightedSize,
                                                kkuint32 intensityHistBuckets[8],
                                                kkint32& areaWithWhiteSpace,
                                                kkuint32 intensityHistBucketsWhiteSpace[8]
                                               )
                                                 const;


    /**
     *@brief  Calculates both Intensity Histograms, one not including internal background pixels and one with
     *         plus size and weighted size.
     *@details 
     *        This method incorporates the functionality of several methods at once.  The idea being that while
     *        we are iterating through the raster image we might as well get all the data we can so as to save
     *        total overall processing time.
     *@code
     * Histogram Ranges:
     *   0:   0  - 31    4: 128 - 159
     *   1:  32  - 63    5: 192 - 223
     *   2:  64  - 95    6: 192 - 223
     *   3:  96 - 127    7: 224 - 255
     *@endcode
     *
     *@param[out]  area          Number of foreground pixels.
     *@param[out]  weightedSize  Area that takes intensity into account.  The largest pixel will have a value of 1.0.
     *@param[out]  intensityHistBuckets  A 8 element array containing a histogram by intensity range where each bucket 
     * represents a range of 32.
     */
    void          CalcAreaAndIntensityFeatures (kkint32&  area,
                                                float&    weightedSize,
                                                kkuint32  intensityHistBuckets[8]
                                               )  const;


    void          CalcCentroid (kkint32&  size,
                                kkint32&  weight,
                                float&  rowCenter,  
                                float&  colCenter,
                                float&  rowCenterWeighted,
                                float&  colCenterWeighted
                               )
                               const;


    void          CalcOrientationAndEigerRatio (float&  eigenRatio,
                                                float&  orientationAngle
                                               );

    float         CalcWeightedArea ()  const;


    double        CenMoment (kkint32 colMoment,
                             kkint32 rowMoment,
                             double  centerCol,
                             double  centerRow 
                            )  const;

    void          CentralMoments (float  features[9])  const;

    float         CenMomentWeighted (kkint32 p, 
                                     kkint32 q, 
                                     float   ew, 
                                     float   eh
                                    ) const;

    void          CentralMomentsWeighted (float  features[9])  const;

    void          Closing ();

    void          Closing (MaskTypes  mask);


    /**
     *@brief  Computes central moments; one set where each pixel is treated as 1 or 0(Foreground/Background) and the 
     *other where each pixel is weighted by intensity value.
     *@details See M. K. Hu, Visual pattern recognition by moment invariants IRE Trans; Inform. Theory, vol. IT, no. 8, pp. 179?187, 1962.
     *While performing this computation the mutable fields 'centroidRow' and 'centroidCol' will be recomputed and their values
     * can be retrieved by their respective access methods.
     *@param[out] foregroundPixelCount  Number of pixels that are considered Foreground; as per the 'Foreground' method.
     *@param[out] weightedPixelCount  The sum of all pixels that are Foreground pixels but weighted by their intensity; such 
     *  that each foreground pixel will be divided by 255.
     *@param[out] centralMoments
     *@param[out] centralMomentsWeighted
     */
    void          ComputeCentralMoments (kkint32&  _foregroundPixelCount,
                                         float&    weightedPixelCount,
                                         float     centralMoments[9],
                                         float     centralMomentsWeighted[9]
                                        )  
                                          const;

    void          ConnectedComponent (kkuint8 connectedComponentDist);

    void          ConnectedComponent8Conected ();

    RasterPtr     CreateColor ()  const;

    RasterPtr     CreateDilatedRaster ()  const;

    RasterPtr     CreateDilatedRaster (MaskTypes  mask)  const;

    void          Dilation ();

    void          Dilation (RasterPtr  dest)  const;


    void          Dilation (MaskTypes  mask);

    void          Dilation (RasterPtr  dest,
                            MaskTypes  mask
                           )
                            const;

    void          Dilation (MorphOp::StructureType  _structure,
                            kkuint16                _structureSize,
                            kkint32                 _foregroundCountTH
                           );

    RasterPtr     CreateErodedImage (MaskTypes  mask)  const;


    RasterPtr     CreateGrayScale ()  const;

    /**
     *@brief Creates a image using a KLT Transform with the goal of weighting in favor the color
     * channels with greatest amount of variance.
     *@details The idea is to weight each color channel by the amount of variance. This is accomplished by
     *  producing a covariance matrix of the three color channels and then taking the Eigen-Vector with the
     *  largest eigen value and using its components to derive weights for each channel for the conversion 
     *  from RGB to grayscale.
     */
    RasterPtr     CreateGrayScaleKLT ()  const;

    /**
     *@brief  Same as 'CreateKLT' except it will only take into account 
     *        pixels specified by the 'mask' image.
     *@param[in]  mask  Raster object where pixels that are greater than 'backgroundPixelTH' are to be considered.
     */
    RasterPtr     CreateGrayScaleKLTOnMaskedArea (const Raster&  mask)  const;

    static
    RasterPtr     CreatePaddedRaster (BmpImage&  image,
                                      kkint32    padding
                                     );

    RasterPtr     CreateSmoothImage (kkint32  maskSize = 3)  const;
    
    RasterPtr     CreateSmoothedMediumImage (kkint32 maskSize)  const;

    RasterPtr     CreateGaussianSmoothedImage (float sigma)  const;

    /**
     *@brief Produces a color image using the 'greenArea' channel, assuming that each unique value will be
     *       assigned a unique color.
     *@details
     *        Assuming that each value in the  channel(GreenArea) will be assigned  a different color
     *        useful for image created by  "SegmentorOTSU::SegmentImage".
     *@see Raster::CreateKLT
     */
    RasterPtr     CreateColorImageFromLabels ();


    /**
     *@brief Returns image where each blob is labeled with a different color.
     *@details
     *        Only useful if 'ExtractBlobs' was performed on this instance. Eight different colors are used and
     *        they are selected by the modules of the blobId(blobId % 8).  Assignments are 0:Red, 1:Green,
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
     *@param[in]  blobs      List of blob's that you want copied into new Raster instance that is created..
     *@returns Image consisting of specified blobs only.
     */
    RasterPtr     CreateFromOrginalImageWithSpecifidBlobsOnly (RasterPtr    origImage,
                                                               BlobListPtr  blobs
                                                              );

    /** 
     *@brief Draw a circle who's center is at 'point' and radius in pixels is 'radius' using color 'color'. 
     *@param[in]  point   Location in image where the center of circle is to be located.
     *@param[in]  radius  The radius in pixels of the circle that is to be drawn.
     *@param[in]  color   The color that is to be used to draw the circle with.
     */
    void          DrawCircle (const Point&       point,
                              kkint32            radius,
                              const PixelValue&  DrawCircle
                             );


    void          DrawCircle (float              centerRow, 
                              float              centerCol, 
                              float              radius,
                              const PixelValue&  pixelValue
                             );


    void          DrawCircle (float              centerRow,    /**< Row that will contain the center of the circle.    */
                              float              centerCol,    /**< Column that will contain the center of the circle. */
                              float              radius,       /**< The radius of the circle in pixels.                */
                              float              startAngle,   /**< Start and End angles should be given in radians    */
                              float              endAngle,     /**< Where the angles are with respect to the compass   */
                              const PixelValue&  pixelValue    /**< Pixel value that is to be assigned to locations in the image that are part of the circle. */
                             );


    void          DrawDot (const Point&       point,
                           const PixelValue&  paintColor,
                           kkint32            size
                          );


    void          DrawFatLine (Point       startPoint,
                               Point       endPoint, 
                               PixelValue  pv,
                               float       alpha
                              );


    void          DrawGrid (float              pixelsPerMinor,
                            kkuint32           minorsPerMajor,
                            const PixelValue&  hashColor,
                            const PixelValue&  gridColor
                           );


    void          DrawLine (kkint32 bpRow,  kkint32 bpCol,
                            kkint32 epRow,  kkint32 epCol
                           );


    void          DrawLine (kkint32 bpRow,    kkint32 bpCol,
                            kkint32 epRow,    kkint32 epCol,
                            kkuint8 pixelVal
                           );


    void          DrawLine (const Point&  beginPoint,
                            const Point&  endPoint,
                            kkuint8       pixelVal
                           );

    void          DrawLine (const Point&       beginPoint,
                            const Point&       endPoint,
                            const PixelValue&  pixelVal
                           );


    void          DrawLine (kkint32 bpRow,    kkint32 bpCol,
                            kkint32 epRow,    kkint32 epCol,
                            kkuint8 r,
                            kkuint8 g,
                            kkuint8 b
                           );


    void          DrawLine (kkint32 bpRow,    kkint32 bpCol,
                            kkint32 epRow,    kkint32 epCol,
                            kkuint8 r,
                            kkuint8 g,
                            kkuint8 b,
                            float   alpha
                           );


    void          DrawLine (kkint32  bpRow,    kkint32 bpCol,
                            kkint32  epRow,    kkint32 epCol,
                            PixelValue  pixelVal
                           );

    void          DrawLine (kkint32  bpRow,    kkint32 bpCol,
                            kkint32  epRow,    kkint32 epCol,
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
                                 kkuint8           redVal,
                                 kkuint8           greenVal,
                                 kkuint8           blueVal
                                );


    void          DrawPointList (Point             offset,
                                 const PointList&  borderPixs,
                                 kkuint8           redVal,
                                 kkuint8           greenVal,
                                 kkuint8           blueVal
                                );

    PointListPtr  DeriveImageLength () const;


    /** @brief reduces image to edge pixels only.  */
    void          Edge ();

    void          Edge (RasterPtr  dest);

    /** @brief removes spurs from image. */
    void          Erosion ();

    void          Erosion (MaskTypes  mask);

    void          Erosion (MorphOp::StructureType  _structure,
                           kkuint16                _structureSize,
                           kkint32                 _backgroundCountTH
                          );

    
    void          Erosion (RasterPtr  dest) const;  /**< Place into destination a eroded version of this instances image.*/

    void          Erosion (RasterPtr  dest,
                           MaskTypes  mask
                          )
                            const;

    void          ErosionChanged (MaskTypes  mask, kkint32 row, kkint32 col);

    void          ErosionChanged1 (MaskTypes  mask, kkint32 row, kkint32 col);

    void          ErosionBoundary (MaskTypes  mask, 
                                   kkint32    blobRowStart, 
                                   kkint32    blobRowEnd, 
                                   kkint32    blobColStart, 
                                   kkint32    blobColEnd
                                  );


    /**
     *@brief  Extracts a specified blob from this image;  useful to extract individual detected blobs.
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
                                       kkint32        padding
                                       ) const;


    /**
     *@brief Will extract a list of connected components from this instance.
     *@details
     *        Will perform a connected component analysis and label each individual blob.  A list of blob
     *        descriptors will be returned.  These blob descriptors can then be used to access individual
     *        blobs.  See 'ExtractABlob' for an example on how to use this method.  The 'ForegroundPixel'
     *        method is used to determine if a given pixel is foreground or background.
     *
     *@param[in]  dist  The distance in pixels that two different pixel locations have to be for them to
     *            be considered connected.  "dist = 1" would indicate that two pixels have to be directly
     *            connected.
     *@returns A list of Blob descriptor instances.
     *@see  ExtractABlob, ExtractABlobTightly, Blob
     */
    BlobListPtr   ExtractBlobs (kkint32  dist);

    
    /**
     *@brief Will return a gray-scale image consisting of the specified color channel only.
     */
    RasterPtr     ExtractChannel (ColorChannels  channel);

    /**
     *@brief  Extracts the pixel locations where the 'mask' images pixel location is a foreground pixel. 
     */
    RasterPtr     ExtractUsingMask (RasterPtr  mask);

    RasterPtr     FastFourier ()  const;

    RasterPtr     FastFourierKK ()  const;

    void          FillHole ();

    RasterPtr     CreateFillHole () const;



    /**
     *@brief  Fills holes in the image using the 'mask' raster as a work area.
     *@details  Any pixel that is not a foreground pixels that has not path by a cross structure to the 
     *  edge of the image will be painted with the foreground pixel value.  The 'mask' raster instance provided
     *  will be used as a temporary work area.  If its dimensions are not the same as this instance it will
     *  e resized.
     */
    void          FillHole (RasterPtr  mask);


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
     *@param[in] color that is to be filled in.
     */
    void          FillBlob (RasterPtr   origImage,
                            BlobPtr     blob,
                            PixelValue  pixelValue
                           );

    void          FillRectangle (kkint32            tlRow,
                                 kkint32            tlCol,
                                 kkint32            brRow,
                                 kkint32            brCol,
                                 const PixelValue&  fillColor
                                );

    void          FindBoundingBox (kkint32&  tlRow,
                                   kkint32&  tlCol,
                                   kkint32&  brRow,
                                   kkint32&  brCol
                                  )  const;

    void  FindBoundingBox (Point& topLeft, Point& botRight)  const;


    /**
     *@brief Returns an image that reflects the differences between this image and the image supplied in the parameter.
     *@details  Each pixel will represent the magnitude of the difference between the two raster instances for that 
     * pixel location.  If there are no differences than a raster of all 0's will be returned. If dimensions are different
     * then the largest dimensions will be sued.
     *@param[in]  r  Raster to compare with.
     *@returns A raster that will reflect the differences between the two instances where each pixel will represent 
     * the magnitude of the differences.
     */
    RasterPtr     FindMagnitudeDifferences (const Raster&  r);


    void          FollowContour (float  countourFreq[5])  const;


    void          FourierExtractFeatures (float  fourierFeatures[5])  const;

    bool          ForegroundPixel (kkint32  row,  
                                   kkint32  col
                                  )  const;


    /**
     * @brief Creates a raster from a compressedBuff created by 'SimpleCompression'.
     */
    static
    RasterPtr     FromSimpleCompression (const kkuint8* compressedBuff,
                                         kkuint32       compressedBuffLen
                                        ); 

    /**
     *@brief  Creates a new instance of Raster object from zLib compressed data.
     *@details Performs the inverse operation of Raster::ToCompressor.
     *@param[in] compressedBuff  Pointer to buffer area containing compressed data originally created by 'ToCompressor'.
     *@param[in] compressedBuffLen  Length in bytes of 'compressedBuff'.
     *@returns If successful a pointer to a new instance of 'Raster'; if there is an error will return NULL.
     *@see  ToCompressor
     */
    static
    RasterPtr     FromCompressor (const kkuint8*  compressedBuff,
                                  kkuint32        compressedBuffLen
                                 ); 

    kkuint8**     GetSubSet (kkuint8** _src,
                             kkint32   _row,
                             kkint32   _col,
                             kkint32   _height,
                             kkint32   _width
                            )  const;

    RasterPtr     HalfSize ();

    HistogramPtr  Histogram (ColorChannels  channel)  const;

    RasterPtr     HistogramEqualizedImage ()  const;

    RasterPtr     HistogramEqualizedImage (HistogramPtr  equalizedHistogram) const;

    HistogramPtr  HistogramGrayscale ()  const;

    RasterPtr     HistogramImage (ColorChannels  channel)  const;

    RasterPtr     HistogramGrayscaleImage ()  const;

    kkMemSize     MemoryConsumedEstimated ()  const;

    RasterPtr     Padded (kkint32 padding);  // Creates a Padded raster object.

    RasterPtr     ReversedImage ();

    RasterPtr     StreatchImage (float  rowFactor,
                                 float  colFactor
                                )  const;

    void          ReverseImage ();   // Reverse the image Foreground and Background.

    void          Opening ();

    void          Opening (MaskTypes mask);

    void          PaintPoint (kkint32            row,
                              kkint32            col,
                              const PixelValue&  pv,
                              float              alpha
                             );

    void          PaintFatPoint (kkint32           row,
                                 kkint32           col,
                                 const PixelValue  pv,
                                 float             alpha
                                );


    RasterPtr     ReduceByEvenMultiple (kkint32  multiple)  const;

    RasterPtr     ReduceByFactor (float factor)  const;  //  0 < factor <= 1.0  ex: 0.5 = Make raster half size

    /**
     *@brief  Locates most complete blob; that is the one with the largest (Height x Width); and removes all
     * other images from the blob.
     */
    void          ReduceToMostCompleteBlob (kkuint8 connectedComponentDist);

    RasterPtr     Rotate (float  turnAngle);

    /** 
     *@brief Determine the point in  an original Raster instance that has derived from a rotated raster instance.
     *@details  Imagine you have raster "A"  and you rotate it by 45 degrees. You then locate a point of interest 
     *  "P" but you want to locate the same point in the original image.
     *@param height  Height of original Raster this rotated Raster derived from.
     *@param width   Width of original Raster this rotated Raster derived from.
     *@param rotatedPoint  Point in this ratoated Raster.
     *@param turnAngle Angle that this Raster was rotated by.
     *@returns The "Point" in the original raster that coresponds to [rotatedPoint] in this Raster.
     */
    Point         RotateDerivePreRotatedPoint (kkint32  origHeight,
                                               kkint32  origWidth,
                                               Point&   rotatedPoint, 
                                               float    turnAngle
                                              )
                                              const;

    RasterPtr     SegmentImage (bool  save = false);

     /// <summary>
     /// Compresses the image in Raster using a simple Run length algorithm and returns a pointer to
     /// compressed data.
     /// </summary>
     ///
     /// <remarks>
     /// Using a simple run length compression algorithm compress the data in Raster and return a
     /// pointer to the resultant buffer.  The caller will take ownership of the compressed data and be
     /// responsible for deleting it.  The function 'FromCompressor' can take the compressed data with
     /// its length and recreate the original Raster object.
     /// </remarks>
     ///
     /// <param name="buffLen"> [in,out] Length of the compressed buffer returned. </param>
     ///
     /// <returns>  pointer to compressed data;  null if it fails, else an kkuint8*. </returns>
     kkuint8*       SimpleCompression (kkuint32&  buffLen)  const;
    

    RasterPtr     SobelEdgeDetector () const;

    RasterListPtr SplitImageIntoEqualParts (kkint32 numColSplits,
                                            kkint32 numRowSplits
                                           )  const;

    RasterPtr     SwapQuadrants ()  const;

    ///<summary> Thresholds image in HSI space.</summary>
    ///<remarks>
    ///  Returns an image with only the pixels that are within a specified distance in HSI space to the supplied  HSI
    ///  parameters. All pixels that are not within the specified distance will be set to <paramref name='flagValue'/>.
    ///</remarks>
    ///<param name='thresholdH'>  Hue in radians(0.0 thru 2Pie).</param>
    ///<param name='thresholdS'>  Saturation (0.0 thru 1.0).</param>
    ///<param name='thresholdI'>  Intensity (0.0 thru 1.0).</param>
    ///<param name='distance'>    Euclidean Distance (0.0 thru 1.0) that a pixel must be within in HSI space to be included.</param>
    ///<param name='flagValue'>   PixelValue to set for pixels that are NOT within 'distance' of threshold.</param>
    ///<returns> A image where pixels that are within the threshold will retain their original pixel values and 
    /// the ones that are not will be set to 'flagValue'.</returns>
    RasterPtr     ThresholdInHSI (float              thresholdH,
                                  float              thresholdS, 
                                  float              thresholdI, 
                                  float              distance,
                                  const PixelValue&  flagValue
                                 );

    RasterPtr     ThinContour ()  const;

    RasterPtr     TightlyBounded (kkuint32 borderPixels)  const;  /**< Returns the smallest image that contains all the foreground pixels plus column and row padding specified by 'borderPixels'. */

    RasterPtr     Transpose ()  const;

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
     *           8 - 8:    Color    0 = ,  1 = Color
     *           9 -  :   Green Channel  (Height * Width bytes)
     *           xxxxx:   Red  Channel, if color image.
     *           xxxxx:   Blue Channel, if color image.
     *@endcode
     *@param[out]  compressedBuffLen Length of the compressed buffer returned.
     *@return pointer to compressed data.
     */
    kkuint8*      ToCompressor (kkuint32&  compressedBuffLen)  const;


    /**
     *@brief Creates the same derived "Raster" instance; tthat is every derived class of "Raster" implements this method; this way you do not need to know the underlying class to get another instance of the sdame type.
     */
    virtual
      RasterPtr  AllocateARasterInstance (kkint32  _height,
                                          kkint32  _width,
                                          bool     _color
                                         )  const;

    virtual
      RasterPtr  AllocateARasterInstance (const Raster& r)  const;

    virtual
      RasterPtr  AllocateARasterInstance (const Raster& _raster,  /**<  Source Raster                                       */
                                          kkint32       _row,     /**<  Starting Row in '_raster' to copy from.             */
                                          kkint32       _col,     /**<  Starting Col in '_raster' to copy from.             */
                                          kkint32       _height,  /**<  Height of resultant raster. Will start from '_row'  */
                                          kkint32       _width    /**<  Width of resultant raster.                          */
                                         )  const;

  private:
    void   AllocateBlobIds ();

    void   AllocateImageArea ();

    void   AllocateFourierMagnitudeTable ();

    void   CleanUpMemory ();

    inline
    bool   BackgroundPixel (kkuint8  pixel)  const;


    // Used by the Gaussian Smoothing algorithm.
    void   BuildGaussian2dKernel (float     sigma,
                                  kkint32&  len,
                                  float**&  kernel
                                 )  const;


    inline
    void   CalcDialatedValue (kkint32   row,
                              kkint32   col,
                              kkint32&  totVal,
                              kkuint8&  numNeighbors
                             )  const;


    inline
    bool   CompletlyFilled3By3 (kkint32  row, 
                                kkint32  col
                               )  const;


    void  DeleteExistingBlobIds ();


    kkuint8  DeltaMagnitude (kkuint8 c1, kkuint8 c2);


    void   FillHoleGrow (kkint32  _row, 
                         kkint32  _col
                        );

    bool   Fit (MaskTypes  mask,
                kkint32    row, 
                kkint32    col
               )  const;

    
    bool   ForegroundPixel (kkuint8  pixel)  const;


    kkuint8  Hit (MaskTypes  mask,
                  kkint32    row, 
                  kkint32    col
                 )  const;


    bool     IsThereANeighbor (MaskTypes  mask,
                               kkint32    row, 
                               kkint32    col
                              )  const;


    void  Moment (kkint64& m00,
                  kkint64& m10,
                  kkint64& m01
                 )  const;


    void   MomentWeighted (float& m00,
                           float& m10,
                           float& m01
                          )  const;



    /**
     *@brief  Computes two sets of moments;  Black and White  and  Weighted.
     *@details  The Black and white are only concerned weather the pixels are Foreground while the weighted 
     * weight each pixel by its intensity value.
     */
    void  Moments(kkint64& m00,
                  kkint64& m10,
                  kkint64& m01,
                  float&   mw00,
                  float&   mw10,
                  float&   mw01
                 )  const;

    inline
    kkint32  NearestNeighborUpperLeft (kkint32  centRow,
                                       kkint32  centCol,
                                       kkint32  dist
                                      );

    inline
    kkint32  NearestNeighborUpperRight (kkint32  centRow,
                                      kkint32  centCol,
                                      kkint32  dist
                                     );


    void     SmoothImageChannel (kkuint8** src,
                                 kkuint8** dest,
                                 kkint32   maskSize
                                )  const;

    void     SmoothUsingKernel (MatrixD&   kernel,
                                kkuint8**  src,
                                kkuint8**  dest
                               )  const;


  protected:
    kkuint8         backgroundPixelValue;
    kkuint8         backgroundPixelTH;     /**< Threshold used to split Background and foreground pixel/ */
    kkint32**       blobIds;               /**< Used when searching for connected components  */
    mutable float   centroidCol;
    mutable float   centroidRow;
    bool            color;
    kkint32         divisor;
    KKStr           fileName;
    mutable kkint32 foregroundPixelCount;
    kkuint8         foregroundPixelValue;
    float**         fourierMag;           /**< Only used if image is result of a Fourier Transform   */
    float*          fourierMagArea;       /**< Only used if image is result of a Fourier Transform   */
    kkint32         height;
    mutable kkuint8 maxPixVal;
    KKStr           title;                /**< Title such as 'Class" that can be assigned to an image. */
    mutable kkint32 totPixels;
    bool            weOwnRasterData;
    kkint32         width;

    kkuint8*        redArea;        // Each color channel is allocated as a single block
    kkuint8*        greenArea;      // for 2 dimensional access use corresponding 2d variables
    kkuint8*        blueArea;       // red for redAreas, green for grenArea.  If 
                                    // image then only green channel is used.

    // The next three variables provide row indexing into there respective color channels.  For performance
    // and simplicity purposes I allocate each channel in a continuous block of memory but to allow for
    // simple accessing by 'row' and 'col' I create the following 3 variables.  Depending on what you
    // are trying to do you could use the appropriate variable.
    kkuint8**       red;            // Provides row indexes into 'redArea'.
    kkuint8**       green;          // Provides row indexes into 'greenArea'.
    kkuint8**       blue;           // Provides row indexes into 'blueArea'.


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


  typedef  Raster::RasterPtr       RasterPtr;

  typedef  Raster::RasterConstPtr  RasterConstPtr;

#define  _Raster_Defined_


  typedef  struct  
  {
    kkint32  row;
    kkint32  col;
  }  MovDir;


  
  class  RasterList:  public  KKQueue<Raster>
  {
  public:
    RasterList (bool  _owner):
        KKQueue<Raster> (_owner)
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

#define  _RasterList_Defined_


}  /* namespace KKB; */
#endif
