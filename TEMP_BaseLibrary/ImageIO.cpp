/* ImageIO.cpp -- O/S Independent routines to load and save Raster Images.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#include "FirstIncludes.h"
#include <iostream>
#include <iostream>
#include <vector>
#include <string.h>

#if  defined(OS_WINDOWS)
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")
#endif

#include "MemoryDebug.h"
using namespace std;


//#ifdef  WIN32
#define  USE_BMPIMAGE
//#define  USE_GDIPLUS
//#else
//#define  USE_CIMG
//#endif


#ifdef  USE_WIN32
#endif


#ifdef  USE_CIMG
  #include "CImg.h"
#endif

#ifdef  USE_OPENCV
  #include "cv.h"
  #include "highgui.h"
#endif


#ifdef  USE_BMPIMAGE
  #include  "BMPImage.h"
#endif

#ifdef   USE_GDIPLUS
#include <windows.h>
#include <gdiplus.h>
#include "BMPImage.h"
#endif


#include "MemoryDebug.h"
#include "BasicTypes.h"
#include "Histogram.h"
#include "KKException.h"
#include "OSservices.h"
#include "Raster.h"

#include "ImageIO.h"


namespace KKU
{


RasterPtr  ReadImage (const KKStr&  imageFileName);

RasterPtr  ReadImagePNG      (const KKStr&  imageFileName);
RasterPtr  ReadImagePGM      (const KKStr&  imageFileName);
RasterPtr  ReadImageUsingGDI (const KKStr& imageFileName);


void  SaveImage  (const Raster&  image, 
                  const KKStr&   imageFileName
                 );

void  SaveImagePGM (const Raster&  image, 
                    const KKStr&   imageFileName
                   );

void  SaveImagePNG (const Raster&  image, 
                    const KKStr&   imageFileName
                   );

void  DefineImageIoAtExit ();

void  ImageIoFinaleCleanUp ();

bool  imageIoAtExitDefined = false;

#ifdef  USE_CIMG



RasterPtr  ReadImage (const KKStr& imageFileName)
{
  int32 row, col;

  CImg<short> img = imageFileName.Str ();

  bool  color;

  if  (img.dimv () == 1)
     color = false;
  else
     color = true;
  
  RasterPtr  image = new Raster (img.dimy (), img.dimx ());

  int32  grayValue;
  int32  r, g, b;

  for  (row = 0;  row < img.dimy(); row++)
  {
    for (col = 0;  col < img.dimx();  col++)
    {
      if  (color)
      {
        r = *img.ptr (col, row, 0, 0);
        g = *img.ptr (col, row, 0, 1);
        b = *img.ptr (col, row, 0, 2);

        grayValue = (int32)((FFLOAT)r * (FFLOAT)0.30 + 
                          (FFLOAT)g * (FFLOAT)0.59 + 
                          (FFLOAT)b * (FFLOAT)0.11 +
                          (FFLOAT)(0.50)
                         );

        image->SetPixelValue (row, col, grayValue);
      }
      else
      {
        image->SetPixelValue (row, col, *img.ptr (col, row, 0, 0));
      }
    }
  }

  image->FileName (imageFileName);

  return image;
}  /* ReadImage */



void  SaveImage  (const Raster&  image, 
                  const KKStr&  imageFileName
                 )
{
  int32 row, col;

  CImg<short> img (image.Width (), image.Height (), 1, 1);

  for  (row = 0;  row < image.Height (); row++)
  {
    for  (col = 0;  col < image.Width ();  col++)
    {
      img.data[img.offset (col, row, 0, 0)] = image.GetPixelValue (row, col);
    }
  }

  img.save_other (imageFileName.Str ());
}  /* SaveImage */



void  DisplayImage  (const Raster& image)
{
  int32  col, row;

  for  (row = 0;  row < image.Height (); row++)
  {
    for  (col = 0;  col < image.Width ();  col++)
    {
      cout  <<  image.GetPixelValue ( row, col) << " ";
    }

    cout << std::endl;
  }
}  /* DisplayImage */




void  DisplayImage (const Raster&  raster,
                    const KKStr&  title
                   )
{
  DisplayImage (raster);
}


#endif



#ifdef  USE_GDIPLUS


RasterPtr  ReadImage (const KKStr&  imageFileName)
{
  KKStr  extension = osGetFileExtension (imageFileName);
  extension.Upper ();
  if  (extension == "PGM")
  {
    RasterPtr  pgmImage = ReadImagePGM (imageFileName);
    if  (pgmImage)
      return  pgmImage;
  }

  WCHAR*  wideName = imageFileName.StrWide ();

  Gdiplus::Bitmap i (wideName, false);

  if  (i.GetLastStatus () != Gdiplus::Ok)
    return NULL;

  int32  height = i.GetHeight ();
  int32  width  = i.GetWidth ();

  int32  row;
  int32  col;
  Gdiplus::Color   color;
  Gdiplus::Status  status;

  uchar r,g,b;

  RasterPtr  image = new Raster (height, width, true);

  for  (row = 0;  row < height;  row++)
  {
    for  (col = 0;  col < width;  col++)
    {
      status = i.GetPixel (col, row, &color);
      r = color.GetR ();
      g = color.GetG ();
      b = color.GetB ();
      image->SetPixelValue (row, col, r, g, b);
    }
  }


  delete  wideName;

  return  image;
}  /* ReadImage */



void  SaveImage  (const Raster&  image, 
                  const KKStr&  imageFileName
                 )
{
  KKStr  extension = osGetFileExtension (imageFileName);
  extension.Upper ();
  if  (extension == "PGM")
  {
    SaveImagePGM (image, imageFileName);
    return;
  }  

  BmpImage  bmpImage (image);
  bmpImage.Save (imageFileName);
}  /* SaveImage */





void  DisplayImage  (const Raster& image)
{
  int32  col, row;

  for  (row = 0;  row < image.Height (); row++)
  {
    for  (col = 0;  col < image.Width ();  col++)
    {
      cout  <<  image.GetPixelValue ( row, col) << " ";
    }

    cout << std::endl;
  }
}  /* DisplayImage */




#endif



#ifdef  USE_BMPIMAGE


RasterPtr  ReadImage (const KKStr&  imageFileName)
{
  bool  successful = false;
  RasterPtr  image = NULL;

  KKStr  extension = osGetFileExtension (imageFileName).ToLower ();
  if  (extension == "pgm")
  {
    image = ReadImagePGM (imageFileName);
  }

  else if  (extension == "bmp")
  {
    BmpImage  bmpImage (imageFileName, successful);
    if  (!successful)
      return  NULL;
    image = new Raster (bmpImage);
  }

  else if  (extension == "png")
  {
    image = ReadImagePNG (imageFileName);
  }

#if  defined(OS_WINDOWS)
  else if  ((extension == "jpg")  ||  (extension == "tif")  ||  (extension == "tiff"))
  {
    image = ReadImageUsingGDI (imageFileName);
    if  (!image->Color ())
      image->ReverseImage ();
  }
#endif
  return  image;
}  /* ReadImage */






bool                gdiStarted            = false;
GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR           gdiplusToken;

RasterPtr  KKU::ReadImageUsingGDI (const KKStr&  imageFileName)
{
  if  (!gdiStarted)
  {
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR           gdiplusToken;
    // Initialize GDI+.
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    //GdiplusShutdown(gdiplusToken);

    gdiStarted = true;
    DefineImageIoAtExit ();
  }

  RasterPtr  r = NULL;
  wchar_t*  imageFileNameWide = imageFileName.ToWchar_t ();

  Bitmap*  bm = Bitmap::FromFile (imageFileNameWide, false);
  if  (bm == NULL)
  {
    cerr << endl << "KKB::ReadImageUsingGDI   ***ERROR***  Reading file: " << imageFileName << endl;
  }
  else
  {
    kkuint32  height = bm->GetHeight ();
    kkuint32  width  = bm->GetWidth ();

    BitmapData* bitmapData = new BitmapData ();
    Gdiplus::Rect rect (0, 0, width, height);

    bm->LockBits(&rect, Gdiplus::ImageLockModeRead, bm->GetPixelFormat (), bitmapData);

    Gdiplus::PixelFormat  pixFormat = bitmapData->PixelFormat;
    kkint32  stride = bitmapData->Stride;
    void*  scan0 = bitmapData->Scan0;

    if  (pixFormat == PixelFormat24bppRGB)
    {
      r = new Raster (height, width, true);

      kkint32  nOffset = stride - width * 3;
      kkint32  bytesPerRow = width * 3 + nOffset;

      uchar  red   = 255;
      uchar  green = 255;
      uchar  blue  = 255;
      
      kkuint32  row, col;

      uchar*  ptr = (uchar*)(void*)scan0;

      for  (row = 0;  row < height;  ++row)
      {
        for  (col = 0;  col < width;  ++col)
        {
          red   = *ptr;  ++ptr;
          green = *ptr;  ++ptr;
          blue  = *ptr;  ++ptr;

          r->SetPixelValue (row, col, red, green, blue);
        }
        
        ptr += nOffset;
      }
    }

    else if  (pixFormat == PixelFormat8bppIndexed)
    {
      kkint32  paletteSize = bm->GetPaletteSize ();

      ColorPalette* palette = (ColorPalette*)malloc (paletteSize);
      bm->GetPalette (palette, paletteSize);

      INT  paletteHasAlpha     = palette->Flags & PaletteFlagsHasAlpha;
      INT  paletteHasGrayScale = palette->Flags & PaletteFlagsGrayScale;
      INT  paletteHasHalftone  = palette->Flags & PaletteFlagsHalftone;

      r = new Raster (height, width, false);
      kkint32  nOffset = stride - width;
      kkint32  bytesPerRow = width + nOffset;

      uchar  index = 255;
      
      kkuint32  row, col;

      uchar*  ptr = (uchar*)(void*)scan0;

      for  (row = 0;  row < height;  ++row)
      {
        for  (col = 0;  col < width;  ++col)
        {
          index = *ptr;  ++ptr;

          ARGB argb = palette->Entries[index];

          kkint32  grayScaleValue = argb % 256;

          r->SetPixelValue (row, col, (uchar)grayScaleValue);
        }
        
        ptr += nOffset;
      }

      delete  palette;
      palette = NULL;
    }

    bm->UnlockBits (bitmapData);

    delete  bm;
    bm = NULL;
  }

  delete  imageFileNameWide;
  imageFileNameWide = NULL;
  return  r;
}

#endif

















RasterPtr  ReadImagePGM (const KKStr& imageFileName)
{
  FILE* i = osFOPEN (imageFileName.Str (), "rb");
  if  (!i)
    return NULL;

  int32  height    = -1;
  int32  pixelSize = -1;
  int32  width     = -1;

  {
    // We are going to read in header part of file.

    bool  eof = false;
    KKStr  nextLine = osReadRestOfLine (i, eof);
    if  (eof  ||  (nextLine[(int16)0] != 'P')  ||  (nextLine[(int16)1] != '5'))
    {
      fclose (i);
      return NULL;
    }

    bool  eol = false;
    int32  headerFieldsRead = 0;
    while  (headerFieldsRead < 3)
    {
      KKStr  nextField = osReadNextToken (i, " \t\n\r", eof, eol);
      if  (eof)
        break;
      if  (nextField.FirstChar () != '#')
      {
        switch  (headerFieldsRead)
        {
        case 0: width     = nextField.ToInt ();  break;
        case 1: height    = nextField.ToInt ();  break;
        case 2: pixelSize = nextField.ToInt ();  break;
        }
        headerFieldsRead++;
      }
    }
  }

  if  ((pixelSize < 0)  ||  (pixelSize > 255)  ||
       (width     < 1)  ||  (height < 1)
      )
  {
    fclose (i);
    cerr << endl << endl
      << "ReadImagePGM   ***ERROR***  ImageFile[" << imageFileName << "]  Invalid Header"
      << "  width[" << width << "]  height[" << height << "]  pixelSize[" << pixelSize << "]"
      << endl
      << endl;
    return  NULL;
  }

  RasterPtr  image = new Raster (height, width, false);

  int32  row, col;

  uchar* colBuff = new uchar[width + 10];

  for  (row = 0;  row < height;  row++)
  {
    fread (colBuff, 1, width, i);

    for  (col = 0;  col < width;  col++)
    {
      image->SetPixelValue (row, col, colBuff[col]);
    }
  }

  fclose (i);
     
  return  image;
}  /* ReadImagePGM */







RasterPtr  ReadImagePNG (const KKStr&  imageFileName)
{
  RasterPtr  result = NULL;
  bool  color = false;
  //int32 row = 0;
  //int32 col = 0;
  bool  eof = false;
  bool  eol = false;


  FILE* i = osFOPEN (imageFileName.Str (), "rb");
  if  (!i)
    return NULL;
  
  KKStr  fileTypeLine = osReadNextToken (i, "\n", eof, eol);
  if  (eol || eof)
  {
    fclose (i);  i = NULL;
    return NULL;
  }

  KKStr  dimensions = osReadNextToken (i, "\n", eof, eol);
  int32  width  = dimensions.ExtractTokenInt (" \n\r\t");
  int32  height = dimensions.ExtractTokenInt (" \n\r\t");

  KKStr  pixelDepthStr = osReadNextToken (i, "\n", eof, eol);
  int32  pixelDepth = pixelDepthStr.ToInt ();

  if  ((width < 0)  ||  (height < 0)  ||  (pixelDepth != 255))
  {
    cerr << std::endl << "ReadImagePNG   Dimensions Width[" << width << "],  Height[" << height << "],  PixelDepth[" << pixelDepth << "]  are invalid" << std::endl;
    fclose (i);
    return NULL;
  }

  fclose (i);  i = NULL;

  if  (fileTypeLine.EqualIgnoreCase ("P5"))
    color = false;

  else if  (fileTypeLine.EqualIgnoreCase ("P6"))
    color = true;

  else 
  {
    cerr << std::endl << "ReadImagePNG   Invalid File Type[" << fileTypeLine << "]" << std::endl;
    fclose (i);
    return NULL;
  }

  int32  totalPixels = width * height;

  result = new Raster (height, width, color);

  if  (color)
  {
    uchar* red   = result->RedArea   ();
    uchar* green = result->GreenArea ();
    uchar* blue  = result->BlueArea  ();

    int32  buffSize = totalPixels * 3;
    uchar*  buffer = new uchar[buffSize];
    int32  charsRead = fread (buffer, 1, buffSize, i);
    if  (charsRead < buffSize)
    {
      fclose (i);
      return NULL;
    }

    uchar*  buffPtr = buffer;
    for  (int32 x = 0;  (x < totalPixels);  x++)
    {
      red   [x] = *buffPtr;  ++buffPtr;
      green [x] = *buffPtr;  ++buffPtr;
      blue  [x] = *buffPtr;  ++buffPtr;
    }

    delete  buffer;
    buffer = NULL;
    fileTypeLine = "";
  }
  else
  {
    uchar* green = result->GreenArea ();
    fread (green, 1, totalPixels, i);

    //  Grayscale images are presumed to be saved with foreground = white (255, 255, 255).
    // When loading into memory we need to invert so that foreground = 0.
    for  (int32 x = 0;  x < totalPixels;  x++)
      green[x] = 255 - green[x];
  }

  fclose (i);
  i = NULL;

  return  result;
}  /* ReadImagePNG */




void  SaveImage  (const Raster&  image, 
                  const KKStr&   imageFileName
                 )
{
  KKStr  extension = osGetFileExtension (imageFileName);
  extension.Upper ();

  if  (extension == "BMP")
  {
    try
    {
      BmpImage  bmpImage (image);
      bmpImage.Save (imageFileName);
    }
    catch  (const KKStr&  errMsg)
    {
      throw KKException (errMsg);
    }
    catch  (const KKException&  errMsg)
    {
      KKStr msg = "SaveImage  Exception  Saving image using 'BmpImage'";
      cerr << std::endl << msg << std::endl << std::endl;
      throw KKException (msg, errMsg);
    }
    catch (...)
    {
      KKStr  errMsg = "SaveImage   Exception occurred calling 'BmpImage::Save'  for file[" + imageFileName + "]";
      cerr << std::endl << errMsg << std::endl << std::endl;
      throw  KKException (errMsg);
    }
  }

  else if  (extension == "PGM")
  {
    SaveImagePGM (image, imageFileName);
  }

  else if  (extension == "PNG")
  {
    SaveImagePNG (image, imageFileName);
  }

  else
  {
    KKStr errMsg = "ImageIO::SaveImage    Extension[" + extension + "] is not supported.";

    cerr << std::endl << std::endl << std::endl
         << "SaveImage     *** ERROR ***    " << std::endl
         << errMsg << std::endl
         << std::endl;
    throw  KKException (errMsg);
  }

  return;
}  /* SaveImage */



void  SaveImagePGM (const Raster&  image, 
                    const KKStr&   imageFileName
                   )
{
  FILE* o = osFOPEN (imageFileName.Str (), "wb");

  {
    KKStr  headerStr (15);
    headerStr << "P5"            << endl
              << image.Width ()  << endl
              << image.Height () << endl
              << (int16)255      << endl;

    const char* h = headerStr.Str ();
    fwrite (h, 1, headerStr.Len (), o);
  }

  int32  totalPixels = image.Height () * image.Width ();

  if  (image.Color ())
  {
    RasterPtr  grayScaleImage = image.CreateGrayScale ();
    const uchar* g = grayScaleImage->GreenArea ();
    fwrite (g, 1, totalPixels, o);
    delete  grayScaleImage;
  }
  else
  {
    const uchar* g = image.GreenArea ();
    fwrite (g, 1, totalPixels, o);
  }
 
  fclose (o);
}  /* SaveImagePGM */




void  SaveImagePNG (const Raster&  image, 
                    const KKStr&   imageFileName
                   )
{
  FILE* o = osFOPEN (imageFileName.Str (), "wb");
  if  (!o)
  {
    KKStr  errMsg = "SaveImagePNG   Error opening File[" + imageFileName + "]";
    cerr << std::endl << errMsg  << std::endl << std::endl;
    throw KKException (errMsg);
  }

  {
    KKStr  headerStr (15);
    headerStr << "P6"            << endl
              << image.Width ()  << endl
              << image.Height () << endl
              << (int16)255      << endl;

    const char* h = headerStr.Str ();
    fwrite (h, 1, headerStr.Len (), o);
  }

  int32  totalPixels = image.Height () * image.Width ();

  if  (image.Color ())
  {
    const uchar*  red   = image.RedArea   ();
    const uchar*  green = image.GreenArea ();
    const uchar*  blue  = image.BlueArea  ();

    for  (int32 x = 0;  x < totalPixels;  x++)
    {
      fwrite (red,   1, 1, o);  ++red;
      fwrite (green, 1, 1, o);  ++green;
      fwrite (blue,  1, 1, o);  ++blue;
    }
  }
  else
  {
    uchar*  green = image.GreenArea ();
    uchar  buff3[3];

    for  (int32 x = 0;  x < totalPixels;  x++)
    {
      uchar  intensity = 255 - (*green);
      buff3[0] = intensity;
      buff3[1] = intensity;
      buff3[2] = intensity;
      fwrite (buff3, 1, 3, o);
      ++green;
    }
  }

  fclose (o);
  o = NULL;

  return ;
}  /* SaveImagePNG */







void  KKU::DefineImageIoAtExit ()
{
  if  (!imageIoAtExitDefined)
  {
    atexit (ImageIoFinaleCleanUp);
    imageIoAtExitDefined = true;
  }
}


void  KKU::ImageIoFinaleCleanUp ()
{
  if  (gdiStarted)
  {
    GdiplusShutdown(gdiplusToken);
    gdiStarted = false;
  }
}



void  DisplayImage  (const Raster& image)
{
  int32  col = 0, row = 0;

  for  (row = 0;  row < image.Height (); row++)
  {
    for  (col = 0;  col < image.Width ();  col++)
    {
      cout  <<  image.GetPixelValue ( row, col) << " ";
    }

    cout << std::endl;
  }
}  /* DisplayImage */







#ifdef  USE_WINDOWS
RasterPtr  ReadImage (const KKStr&  imageFileName)
{




}  /* ReadImage */

#endif



#ifdef  USE_OPENCV

IplImage*  CreateIplImage (const Raster&  raster);


inline  
void  UpdateLowHighBucket (int32&   lowBucket,
                           int32&   highBucket,
                           uchar  pixelValue
                          )
{
  if  (pixelValue < 126)
  {
    lowBucket++;
  }
  else
  {
    highBucket++;
  }
}  /* UpdateLowHighBucket */




RasterPtr  ReadImage (const KKStr&  imageFileName)
{
  IplImage*  image;
  RasterPtr  raster;

  image = cvLoadImage (imageFileName.Str (), -1);  // -1 = Load with as many channels as in image.
                                                   //  0 = Load with 1 Channel only
                                                   //  1 = Load as a Color Image.
  if  (!image)
    return NULL;

  int32 col;
  bool  color;
  int32 height;
  int32 row;
  int32 width;

  if  (image->nChannels == 1)
    color = false;

  else if  (image->nChannels == 3)
    color = true;

  height = image->height;
  width  = image->width;

  raster = new Raster (height, width, color);

  if  (color)
  {
    // Need to assign Channel Idexes to appropriate index.
    int32  bIDX = 0;
    int32  gIDX = 1;
    int32  rIDX = 2;

    bool  rgbAlwaysEqual = true;

    for  (int32 x = 0; x < 3; x++)
    {
      switch  (toupper (image->channelSeq[x]))
      {
        case 'R': rIDX = x;  break;
        case 'G': gIDX = x;  break;
        case 'B': bIDX = x;  break;
      }
    }

    int32 r, g, b;

    for  (row = 0;  row < height;  row++)
    {
      for  (col = 0;  col < width;  col++)
      {
        CvScalar all3 = cvGet2D (image,  row, col);

        r = (int32)all3.val[rIDX];
        g = (int32)all3.val[gIDX];
        b = (int32)all3.val[bIDX];

        if  ((r != g)  ||  (r != b))
          rgbAlwaysEqual = false;

        raster->SetPixelValue (row, col, r, g, b);
      }
    }
    //rgbAlwaysEqual = true;
    if  (rgbAlwaysEqual)
    {
      // We really have a gray scale image
      RasterPtr  gsRaster= raster->CreateGrayScale ();
      delete  raster;
      raster = gsRaster;  
      gsRaster = NULL;
      color = false;
    }
  }
  else
  {
    for  (row = 0;  row < height;  row++)
    {
      for  (col = 0;  col < width;  col++)
      {
        CvScalar g = cvGet2D (image, row, col);
        raster->SetPixelValue (row, col, (int32)g.val[0]);
      }
    }
  }


  cvReleaseImage (&image);

  int32  numSmallPixelVales = 0;
  int32  numLargePixelVales = 0;

  int32 lastRow = height - 1;
  int32 lastCol = width - 1;

  if  (!color)
  {
    uchar**  g = raster->Rows ();
    for  (col = 0;  col < width;  col++)
    {
      UpdateLowHighBucket (numSmallPixelVales, numLargePixelVales, g[0][col]);
      UpdateLowHighBucket (numSmallPixelVales, numLargePixelVales, g[lastRow][col]);
    }

    for  (row = 0;  row < height;  row++)
    {
      UpdateLowHighBucket (numSmallPixelVales, numLargePixelVales, g[row][0]);
      UpdateLowHighBucket (numSmallPixelVales, numLargePixelVales, g[row][lastCol]);
    }

    if  (numSmallPixelVales < numLargePixelVales)
    {
      // We have loaded a white forground on a Black background
      for  (row = 0;  row < height;  row++)
      {
        for  (col = 0;  col < width;  col++)
        {
          g[row][col] = 255 - g[row][col];
        }
      }
    }
  }

  raster->FileName (imageFileName);
  return  raster;
}  /* OpenFile */





void  SaveImage (const Raster&  raster, 
                 const KKStr&  imageFileName
                )
{
  // cout << "SaveImage [" << imageFileName << "]." << std::endl;

  IplImage*  image = CreateIplImage (raster);

  int32  returnCd = cvSaveImage (imageFileName.Str (), image);

  cvReleaseImage (&image);

  //cout  << "SaveImage  Exiting" << std::endl;
}  /* Save */







IplImage*  CreateIplImage (const Raster&  raster)
{
  #ifdef  DEBUG_KK
  cout << "CreateIplImage  Start"  << std::endl;
  #endif


  int32  numOfChannels;
  if  (raster.Color ())
    numOfChannels = 3;
  else
    numOfChannels = 1;


  CvSize imageSize;

  imageSize.width  = raster.Width ();
  imageSize.height = raster.Height ();

  IplImage*  image = cvCreateImage (imageSize, IPL_DEPTH_8U, numOfChannels);

  int32     col;
  CvScalar  pix;
  int32     row;


  if  (raster.Color ())
  {
    uchar  r, g, b;

    for  (row = 0;  row < raster.Height ();  row++)
    {
      for  (col = 0;  col < raster.Width ();  col++)
      {
        raster.GetPixelValue (row, col, r, g, b);

        pix.val[0] = b;
        pix.val[1] = g;
        pix.val[2] = r;

        cvSet2D (image, row, col, pix);
      }
    }
  }
  else
  {
    for  (row = 0;  row < raster.Height ();  row++)
    {
      for  (col = 0;  col < raster.Width ();  col++)
      {
        // if  (raster.BackgroundPixelValue () == 0)
        //   pix.val[0] = 255 - raster.GetPixelValue (row, col);
        // else
        //   pix.val[0] = raster.GetPixelValue (row, col);

        pix.val[0] = raster.GetPixelValue (row, col);
        pix.val[1] = 0;
        pix.val[2] = 0;

        cvSet2D (image, row, col, pix);
      }
    }
  }



  #ifdef  DEBUG_KK
  cout << "CreateIplImage  Done"  << std::endl;
  #endif

  return  image;
}  /* CreateIplImage */





void  DisplayImage (const Raster&  raster,
                    const KKStr&  title
                   )
{
  //  char Filename [MAX_PATH];
  //   wsprintf(Filename,"c:\\bla.bmp");
  //  IplImage *LoadedImage = cvLoadImage(Filename,-1);


  IplImage *LoadedImage = CreateIplImage (raster);
  cvNamedWindow (title.Str (), CV_WINDOW_AUTOSIZE);
  cvShowImage (title.Str (), LoadedImage);
  cvWaitKey (-1);
  cvReleaseImage (&LoadedImage);
}  /* DisplayImage */

#endif




void  SaveImageInverted (Raster&       raster, 
                         const KKStr&  imageFileName
                        )
{
  RasterPtr  invertedImage = new Raster (raster);

  int32  r, c;

  uchar**  g    = invertedImage->Green ();
  uchar**  red  = invertedImage->Red   ();
  uchar**  blue = invertedImage->Blue  ();

  for  (r = 0; r < invertedImage->Height ();  r++)
  {
    for  (c = 0;  c < invertedImage->Width ();  c++)
    {
      g[r][c] = 255 - g[r][c];
      if  (invertedImage->Color ())
      {
        red [r][c] = 255 - red [r][c];
        blue[r][c] = 255 - blue[r][c];
      }
    }
  }

  SaveImage (*invertedImage, imageFileName);
  delete  invertedImage;
  invertedImage = NULL;
}  /* SaveImageInverted */









void  SaveImageGrayscaleInverted4Bit (const Raster&  image, 
                                      const KKStr&  _fileName
                                     )
{
  try
  {
    BmpImage  bmpImage (image);
    bmpImage.SaveGrayscaleInverted4Bit (_fileName);
  }
  catch  (const KKStr&  errMsg)
  {
    throw KKException (errMsg);
  }
  catch  (const KKException&  errMsg)
  {
    KKStr msg = "SaveImage  Exception  Saving image using 'BmpImage'";
    cerr << std::endl << msg << std::endl << std::endl;
    throw KKException (msg, errMsg);
  }
  catch (...)
  {
    KKStr  errMsg = "SaveImage   Exception occured calling 'BmpImage::Save'  for file[" + _fileName + "]";
    cerr << std::endl << errMsg << std::endl << std::endl;
    throw  KKException (errMsg);
  }
}  /* SaveImageGrayscaleInverted4Bit */




/**
 *@brief  Saves image as BMP using compressed grayscale where Background = 255 and foreground = 0
 *@details  If image is color will convert to grayscale 1st.
 * Palette will be set to 0 = 255, 1 = 254, 2 = 253, ...  255 = 0.
 */
void  SaveImageGrayscaleInverted8Bit (const Raster&  image, 
                                      const KKStr&  _fileName
                                     )
{
  try
  {
    BmpImage  bmpImage (image);
    bmpImage.SaveGrayscaleInverted8Bit (_fileName);
  }
  catch  (const KKStr&  errMsg)
  {
    throw KKException (errMsg);
  }
  catch  (const KKException&  errMsg)
  {
    KKStr msg = "SaveImage  Exception  Saving image using 'BmpImage'";
    cerr << std::endl << msg << std::endl << std::endl;
    throw KKException (msg, errMsg);
  }
  catch (...)
  {
    KKStr  errMsg = "SaveImage   Exception occurred calling 'BmpImage::Save'  for file[" + _fileName + "]";
    cerr << std::endl << errMsg << std::endl << std::endl;
    throw  KKException (errMsg);
  }
}  /* SaveImageGrayscaleInverted4Bit */



bool  SupportedImageFileFormat (const KKStr&  imageFileName)
{
  KKStr  ext = osGetFileExtension (imageFileName);
  ext.Upper ();

  return  (
           (ext == "BMP")  ||  (ext == "GIF")  ||
           (ext == "JPG")  ||  (ext == "PGM")  ||
           (ext == "PNG")  ||  (ext == "TIF")  ||  
           (ext == "TIFF")
          );
}



RasterPtr  SegmentPolutionImage (RasterPtr  r,
                                 KKStr     fileName
                                )
{ 
  KKStr  rootName  = osGetRootName (fileName);
  KKStr  dirName = "c:\\Temp\\PolutionImages\\" + rootName;
  osCreateDirectoryPath (dirName);
  osAddLastSlash (dirName);

  SaveImage (*r, dirName + rootName + "_Orig.bmp");

  RasterPtr  gImage = NULL;

  if  (r->Color ())
    gImage = r->CreateGrayScale ();
  else
    gImage = new Raster (*r);

  SaveImage (*gImage, dirName + rootName + "_GrayScale.bmp");

  HistogramPtr h = gImage->HistogramGrayscale ();
  h->SaveGraphImage (dirName + rootName +"_Histogram.bmp");
  h->Save           (dirName + rootName +"_Histogram.txt");

  HistogramPtr  sh = h->Smooth (3);

  h->SaveGraphImage (dirName + rootName +"_HistogramSmoothed.bmp");


  delete  h;
  delete  sh;
  delete  gImage;

  return NULL;
}  /* SegmentPolutionImage */


} /* namespace KKU; */
