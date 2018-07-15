/* Raster.h -- Class that one raster image.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#if !defined(_SEGMENTOROTSU_)
#define _SEGMENTOROTSU_

#include  "KKBaseTypes.h"
#include  "Matrix.h"
#include  "KKStr.h"
#include  "Raster.h"
#include  "RunLog.h"


namespace KKB
{
  /**
   *@class   SegmentorOTSU
   *@author  Kurt Kramer
   *@details
   *@code
   **************************************************************************************
   **                                    SegmentorOTSU                                  *
   **                                                                                   *
   **  Will segment an image using an implementation of OTSU. Based on MatLab code I    *
   **  found on the Internet.  http://www.biomecardio.com/matlab/otsu.html              *
   **  by Otsu N,                                                                       *
   **     <a href="matlab:web('http://dx.doi.org/doi:10.1109/TSMC.1979.4310076')">      *
   **     A Threshold Selection Method from Gray-Level Histograms</a>,                  *
   **     IEEE Trans. Syst. Man Cybern. 9:62-66;1979                                    *
   **************************************************************************************
   *@endcode
   *@sa raster
   */

  class SegmentorOTSU
  {
  public:
    SegmentorOTSU (RunLog&  _log);
	  ~SegmentorOTSU ();

    /**
     *@brief  Segments image into 'numClasses'.
     *@param[in]  srcImage  Image to segment.  If it is a color image will be 
     *                      converted to Gray-Scale using 'CreateGrayScaleKLTOnMaskedArea'
     *@param[in]  numClasses Number of classes to segment image into.  Current only '2' and '3' are supported.
     *@param[out]  sep  
     *@return  Labeled gray-scale image where pixels will be labels into their respective class; between '1' and 'numClasses'.
     */
    RasterPtr  SegmentImage (RasterPtr  srcImage,
                             kkint32    numClasses,
                             double&    sep
                            );


    /**
     *@brief  Segments image into 'numClasses' taking into account only pixels 
     *        indicated by 'mask' image.
     *@param[in]  srcImage  Image to segment.  If it is a color image will be 
     *                      converted to Gray-Scale using 'CreateGrayScaleKLTOnMaskedArea'
     *@param[in]  mask  Indicates which pixels to consider when thresholding image.  Pixels
     *                  that are not part of mask will be assigned label '0'.
     *@param[in]  numClasses Number of classes to segment image into.  Current only '2' and '3' are supported.
     *@param[out]  sep  
     *@return  Labeled gray-scale image where pixels will be label into their respective class; between '1' and 'numClasses'.
     */
    RasterPtr  SegmentMaskedImage (RasterPtr  srcImage,
                                   RasterPtr  mask,
                                   kkint32    numClasses,
                                   double&    sep
                                  );

    
    /**
     *@brief  Will compute the average RGB values of the region indicated by the segmented image.
     *@param[in] origImage  The original image that 'SegmentImage' was performed on.
     *@param[in] segmentedImage  The segmented image that was returned by either 'SegmentImage' or 'SegmentMaskedImage'.
     *@param[in] segmentedClass  The class in 'segmentedImage' that we want to compute average RGB for.
     */
    PixelValue  ClassAverageRGB (const RasterPtr  origImage,
                                 const RasterPtr  segmentedImage,
                                 uchar            segmentedClass
                                );


    /**
     *@brief Determines which class in the segmented image is closet in RGB color space to the specified target color.
     *@param[in] origImage  The original image that 'SegmentImage' was performed on.
     *@param[in] segmentedImage  The segmented image that was returned by either 'SegmentImage' or 'SegmentMaskedImage'.
     *@param[in] targetColor  RGB Color that we want the chosen class to close to.
     *@returns  The class that has the average RGB value closest to targetColor.
     */
    uchar  GetClassClosestToTargetColor (const RasterPtr    origImage,
                                         const RasterPtr    segmentedImage,
                                         const PixelValue&  targetColor
                                        );

    uchar  Threshold1 ()  const  {return threshold1;}
    uchar  Threshold2 ()  const  {return threshold2;}


  private:
    void   LabelRaster (RasterPtr  result, 
                        uchar      pixelValue,
                        uchar      label,
                        RasterPtr  srcImage
                       );

    void   LabelRaster (RasterPtr  result, 
                        RasterPtr  mask,
                        uchar      pixelValue,
                        uchar      label,
                        RasterPtr  srcImage
                       );

    double  sig_func (VectorDouble          k,
                      kkint32               nbins,
                      const VectorDouble&   P,
                      kkint32               numClasses
                     );

   template<typename T>
   vector<T>  BDV (T  start,
                   T  inc,
                   T  end
                  );


   VectorDouble  Add (const VectorDouble&  left,
                      double               right
                     );


   template<typename T>
   vector<T>   CumSum (const vector<T>&  v);


   VectorDouble  DotMult (const VectorDouble&  left,
                          const VectorDouble&  right
                         );

   MatrixD  DotMult (const MatrixD&  left,
                     const MatrixD&  right
                    );


   VectorDouble  DotDiv (const VectorDouble&  left,
                         const VectorDouble&  right
                        );

   MatrixD  DotDiv (const MatrixD&  left,
                    const MatrixD&  right
                   );

   template<typename T>
   vector<T>   FlipLeftRight (const vector<T>&  v);

   VectorDouble  LinSpace (double  start,
                           double  end,
                           kkint32 numPoints
                          );

   void  MakeNanWhenLesOrEqualZero (MatrixD&  m);

   void  NdGrid (const VectorDouble&  x,
                 const VectorDouble&  y,
                 MatrixD&             xm,
                 MatrixD&             ym
                );


   VectorDouble  Power (const VectorDouble&  left,
                        double               right
                       );

   MatrixD  Power (const MatrixD&  left,
                   double          right
                  );

   VectorDouble  Round (const VectorDouble&  v);
  
   VectorDouble  Subt (const VectorDouble&  left,
                       const VectorDouble&  right
                      );

   VectorDouble  Subt (const VectorDouble&  left,
                       double               right
                      );

   VectorDouble  Subt (double               left,
                       const VectorDouble&  right
                      );

   template<typename T>
   vector<T>  SubSet (const vector<T>&  P, 
                      kkint32           start,
                      kkint32           end
                     );

   template<typename T>
   T  Sum (const vector<T>&  v);

   template<typename T>
   T  SumSubSet (const vector<T>&  P, 
                 kkint32           start,
                 kkint32           end
                );

   template<typename T>
   void  ZeroOutNaN (vector<T>&  v);

   void  ZeroOutNaN (MatrixD&  m);


   //VectorDouble  PWD

    uchar   threshold1;
    uchar   threshold2;
    double  NaN;
    RunLog&  log;
  };  /* SegmentorOTSU */
}  /* KKB */



#endif
