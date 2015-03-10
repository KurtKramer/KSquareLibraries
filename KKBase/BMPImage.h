/* BMPImage.h -- Manages the reading and writing of BMP image files.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef  _BMPIMAGE_
#define  _BMPIMAGE_


#ifdef  WIN32
#include <windows.h>
#include <conio.h>
#else
#include <stdio.h>
#include "BMPheader.h"
#endif

#include "KKStr.h"

//typedef unsigned char uchar;


namespace KKB
{
  #ifndef  _RASTER_
  class  Raster;
  typedef  Raster*  RasterPtr;

  class  PixelValue;
  typedef  PixelValue*  PixelValuePtr;
  #endif

  /**
   *@class  BmpImage
   *@brief Used to encode and decode BMP Images.
   *@details  Purpose of this class it to facilitate the loading and saving of BMP Image files. Not all
   * formats are supported, but the compressed version is, which most others don't support.  As a result 
   * this class will write considerably smaller BMP files than other utilities that I have tried.  This
   * results in faster writing images due to less disk-io.  This is especially useful when working with 
   * grayscale images that utilize only a small number of bits per pixel.  For example a image that is 
   * 4096 x 4096 using only 8 levels of grayscale (3 bits/Pixel) can compress to just several KB in many
   * cases rather than the 16MB that a non compressed version of BMP would require.
   * 
   * This code has been around for +10 years.  AT some point in the future when convenient it should be
   * rewritten to simplify.
   */
  class  BmpImage 
  {
  private:

    #pragma pack(push,1)
    struct  BITMAPINFOHEADER
    {
      DWORD    biSize;
      LONG     biWidth;
      LONG     biHeight;
      WORD     biPlanes;
      WORD     biBitCount;
      DWORD    biCompression;
      DWORD    biSizeImage;
      LONG     biXPelsPerMeter;
      LONG     biYPelsPerMeter;
      DWORD    biClrUsed;
      DWORD    biClrImportant;
    };


    struct BITMAPFILEHEADER 
    {
      WORD    bfType;
      DWORD   bfSize;
      WORD    bfReserved1;
      WORD    bfReserved2;
      DWORD   bfOffBits;
    };


    struct RGBQUAD 
    {
      BYTE  rgbBlue;
      BYTE  rgbGreen;
      BYTE  rgbRed;
      BYTE  rgbReserved;
    };
    #pragma pack(pop)


  public:
    BmpImage (kkint32  _height,
              kkint32  _width,
              kkint32  _numOfColors
             );

    /**
     *@brief  Constructs a BMP image from the file specified by '_fileName'.
     *@param[in]  _fileName     Name of file to load BMP image from.
     *@param[out] successful   Returns true if file successful in loading.
     */
    BmpImage (const KKStr&  _fileName,
              bool&         successful
             );


    /**
     *@brief Constructs a BMPImage instance from a Raster image; this is one way to save a Raster image to disk.
     */
    BmpImage (const Raster&  raster);


    void  InitializeFields (kkint32  _height,
                            kkint32  _width
                           );

    ~BmpImage ();

    bool     Color ()     const  {return color;}          /**< @brief Returns true if a Color image. */

    const
    KKStr&   FileName ()  const  {return fileName;}

    bool     FourBitUncompressed ();

    kkuint32 Height ()    const  {return  bmh.biHeight;}

    const   
    uchar*   BlueRow (kkint32 row)  const;                    /**< @brief Returns the specified Row from the Blue Channel. */

    uchar**  Image ()            {return  image;}           /**< @brief Returns back two dimension matrix of image; if color it will be the green channel. */

    const   
    uchar*   ImageRow (kkint32 row)  const;                   /**< @brief Returns the specified Row from the Green Channel. */

    const   
    uchar*   RedRow (kkint32 row)  const;                     /**< @brief Returns the specified Row from the Red Channel. */

    uchar    MaxPixVal () const  {return  uchar (maxPixVal);}

    kkuint32 Width ()     const  {return  bmh.biWidth;}


    bool  AreThereEdgePixels ();

    void  ClearImage();

    void  DoubleSize ();

    void  DownSize ();

    void  EliminateVerticalLines ();

    uchar&  Pixel (kkint32  row, 
                   kkint32  col
                  );

    void  AddPixel (kkuint32 row, 
                    kkuint32 col, 
                    uchar  pixValue
                   );


    void  Binarize ();

    void  Print ();
    
    /**  @brief Used to expand dimensions of image by 6 pixels so as to make sure that no image is along the edge. */
    void  ReAllocateForBiggerScreen (); 

    void  Save (const KKStr&  fileName);


    /**
     *@brief  Saves image using 4 bit compressed grayscale where Background = 255 and foreground = 0
     *@details  If image is color will convert to grayscale 1st.
     * Palette will be set to 0 = 255, 1 = 238, 2 = 221, 3 = 204...  255 = 0.
     */
    void  SaveGrayscaleInverted4Bit (const KKStr&  _fileName);


    /**
     *@brief  Saves image using compressed grayscale where Background = 255 and foreground = 0
     *@details  If image is color will convert to grayscale 1st.
     * Palette will be set to 0 = 255, 1 = 254, 2 = 253, ...  255 = 0.
     */
    void  SaveGrayscaleInverted8Bit (const KKStr&  _fileName);


    void  Set16Colors ();

    void  Set256Colors ();


    void  SetPaletteEntry (kkint32            palletIndex,
                           const PixelValue&  pixValue
                          );

    /**
     *@brief  Will set the pixel value of the specified row and col to 'pixel'.
     */
    void  SetPixelValue (kkint32  row, 
                         kkint32  col, 
                         kkint32  pixel
                        );


    class   CodedPixels;  /**<  @brief Class needed to support BMP Compression */
    struct  CodePair;
    struct  Bmp1BitRec;
    struct  Bmp4BitRecs;
    struct  BMP_24BitPixel;
    union   DWordParts;
    union   LongParts;
    union   WordParts;

    typedef  CodePair*  CodePairPtr;


  private:
    class  PalletBuilder;
    typedef  PalletBuilder*  PalletBuilderPtr;

    void  AllocateRaster ();

    kkint32 BMIcolorArraySize (BITMAPINFOHEADER&  _bmh);

    PalletBuilderPtr  BuildPalletFromRasterData ();

    void  CleanUpMemory ();

    void  Load1BitColor (FILE*  inFile,
                         bool&  successfull
                        );

    void  Load4BitColor (FILE*  inFile,
                         bool&  successfull
                        );

    void  Load4BitColorCompressed (FILE*  inFile,
                                   bool&  successfull
                                  );

    void  Load8BitColor (FILE*  inFile,
                         bool&  successfull
                        );

    void  Load8BitColorCompressed (FILE*  inFile,
                                   bool&  successfull
                                  );

    void  Load24BitColor (FILE*  inFile,
                          bool&  successfull
                         );

    bool  ReversedGrayscaleImage ();

    void  SaveGrayScale (FILE*  outFile);

    void  SaveColor (FILE*  outFile);

    void  SaveColorCompressed256 (PalletBuilderPtr  palletBuilder,
                                  FILE*             outFile
                                 );

    void  SaveColor24BPP (FILE*  outFile);

    void  SetUp4BitPallet ();

    void  SetUp8BitPallet ();

    void  SetUp16BitPallet (RGBQUAD*  palette);

    void  SetUp256BitPalette (RGBQUAD*  palette);


    bool              color;  // true if color image.
    BITMAPFILEHEADER  hdr;
    BITMAPINFOHEADER  bmh;
    KKStr             fileName;
    uchar**           red;
    uchar**           image;             /**< Used if grayscale.  */
    uchar**           blue;
    kkint32           maxPixVal;         /**< Largest PixelValue  */
    kkint32           numOfColors;
    RGBQUAD*          palette;
    kkint32           paletteEntries;
    kkint32           paletteMap[256];
  };

  typedef  BmpImage*  BmpImagePtr;
}  /* namespace KKB; */

#endif
