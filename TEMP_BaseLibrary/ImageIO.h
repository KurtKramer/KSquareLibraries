/* ImageIO.cpp -- O/S Independent routines to load and save Raster Images.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */

#ifndef  _IMAGEIO_
#define  _IMAGEIO_

#include  "Str.h"


namespace KKU
{
  /**
   *@file ImageIO.h
   *@brief  Methods for reading and writing Images.
   */


  #ifndef  _RASTER_
  class  Raster;
  typedef  Raster*  RasterPtr;
  #endif


  void  DisplayImage (const Raster&  raster,
                      const KKStr&   title
                     );

  void  DisplayImage  (const Raster& image);


  RasterPtr  ReadImage (const KKStr& imageFileName);


  bool  SupportedImageFileFormat (const KKStr&  imageFileName);


  void  SaveImage  (const Raster&  image, 
                    const KKStr&   imageFileName
                   );

  void  SaveImageInverted (Raster&       raster, 
                           const KKStr&  imageFileName
                          );


  /**
   *@brief Saves image as BMP file using 4 bit compressed grayscale where Background = 255 and foreground = 0
   *@details If image is color will convert to grayscale first. Palette entries will be set to 0 = 255, 1 = 238,
   * 2 = 221, 3 = 204...  255 = 0.
   */
  void  SaveImageGrayscaleInverted4Bit (const Raster&  image, 
                                        const KKStr&  _fileName
                                       );


  /**
   *@brief  Saves image as BMP using compressed grayscale where Background = 255 and foreground = 0
   *@details  If image is color will convert to grayscale first.
   * Palette will be set to 0 = 255, 1 = 254, 2 = 253, ...  255 = 0.
   */
  void  SaveImageGrayscaleInverted8Bit (const Raster&  image, 
                                        const KKStr&  _fileName
                                       );



  RasterPtr  SegmentPolutionImage (RasterPtr  r,
                                   KKStr      fileName
                                  );

} /* namespace KKU; */

#endif

