/* ImageIO.cpp -- O/S Independent routines to load and save Raster Images.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"
#include <cstdio>
#include <iostream>
#include <iostream>
#include <vector>
#include <string.h>

#if  defined(KKOS_WINDOWS)

#pragma warning( push )  
#pragma warning( disable : 4458 )  

#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

#pragma warning( pop ) 

#endif

#include "MemoryDebug.h"
using namespace std;

#include "ImageIO.h"
#include "BMPImage.h"
#include "KKBaseTypes.h"
#include "KKException.h"
#include "Histogram.h"
#include "OSservices.h"
#include "Raster.h"
using  namespace  KKB;


namespace  KKB
{
  RasterPtr  ReadImagePGM (const KKStr& imageFileName);

  RasterPtr  ReadImagePPM (const KKStr& imageFileName);

  RasterPtr  ReadImageUsingGDI (const KKStr& imageFileName);


  KKStr  ReadImagePpmField (FILE*   in,
                            bool&   eof
                           );

  void  SaveImagePGM (const Raster&  image, 
                      const KKStr&   imageFileName
                     );

  void  SaveImagePNG (const Raster&  image, 
                      const KKStr&   imageFileName
                     );

  void  SaveImagePPM (const Raster&  image, 
                      const KKStr&   imageFileName
                     );

  void  DefineImageIoAtExit ();

  void  ImageIoFinaleCleanUp ();


  bool  imageIoAtExitDefined = false;
}



void  KKB::DefineImageIoAtExit ()
{
  if  (!imageIoAtExitDefined)
  {
    atexit (ImageIoFinaleCleanUp);
    imageIoAtExitDefined = true;
  }
}



void  KKB::DisplayImage  (const Raster& image)
{
  kkint32  col, row;

  for  (row = 0;  row < image.Height (); row++)
  {
    for  (col = 0;  col < image.Width ();  col++)
    {
      cout  <<  image.GetPixelValue ( row, col) << " ";
    }

    cout << std::endl;
  }
}  /* DisplayImage */



void  KKB::DisplayImage (const Raster&  raster,
                         const KKStr&  title
                        )
{
  cout << std::endl  << title << std::endl;
  DisplayImage (raster);
}



RasterPtr  KKB::ReadImage (const KKStr&  imageFileName)
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

  else if  (extension == "ppm")
  {
    image = ReadImagePPM (imageFileName);
  }

#if  defined(KKOS_WINDOWS)
  else if  ((extension == "jpg")  ||  (extension == "tif")  ||  (extension == "tiff"))
  {
    image = ReadImageUsingGDI (imageFileName);
    if  (image)
    {
      if  (!image->Color ())
        image->ReverseImage ();
    }
  }
#endif

  return  image;
}  /* ReadImage */


#if  defined(KKOS_WINDOWS)

#include <Gdipluspixelformats.h>


bool  gdiStarted = false;
ULONG_PTR  gdiplusToken;

RasterPtr  KKB::ReadImageUsingGDI (const KKStr&  imageFileName)
{
  if  (!gdiStarted)
  {
    GdiplusStartupInput gdiplusStartupInput;

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

      uchar  red   = 255;
      uchar  green = 255;
      uchar  blue  = 255;
      
      kkuint32  row, col;

      bool  grayScaleImage = true;

      uchar*  ptr = (uchar*)(void*)scan0;
      for  (row = 0;  row < height;  ++row)
      {
        for  (col = 0;  col < width;  ++col)
        {
          red   = *ptr;  ++ptr;
          green = *ptr;  ++ptr;
          blue  = *ptr;  ++ptr;
          r->SetPixelValue (row, col, red, green, blue);

          if  ((red != green)  ||  (red != blue))
            grayScaleImage= false;
        }
        ptr += nOffset;
      }

      if  (grayScaleImage)
      {
        RasterPtr  grayScaleR = r->CreateGrayScale ();
        delete  r;
        r = grayScaleR;
        grayScaleR = NULL;
      }
    }

    else if  (pixFormat == PixelFormat8bppIndexed)
    {
      kkint32  paletteSize = bm->GetPaletteSize ();

      ColorPalette* palette = (ColorPalette*)malloc (paletteSize);
      bm->GetPalette (palette, paletteSize);

      r = new Raster (height, width, false);
      kkint32  nOffset = stride - width;

      uchar  index = 255;

      uchar*  ptr = (uchar*)(void*)scan0;

      for  (kkuint32 row = 0;  row < height;  ++row)
      {
        for  (kkuint32 col = 0;  col < width;  ++col)
        {
          index = *ptr;  ++ptr;

          ARGB argb = palette->Entries[index];

          kkint32  grayScaleValue = argb % 256;

          r->SetPixelValue (row, col, (uchar)grayScaleValue);
        }
        
        ptr += nOffset;
      }

      free (palette);
      palette = NULL;
    }

    bm->UnlockBits (bitmapData);

    delete  bitmapData;  bitmapData = NULL;
    delete  bm;          bm = NULL;
  }

  delete  imageFileNameWide;
  imageFileNameWide = NULL;
  return  r;
}



RasterPtr  KKB::ReadImagePGM (const KKStr& imageFileName)
{
  FILE* i = osFOPEN (imageFileName.Str (), "rb");
  if  (!i)
    return NULL;

  kkint32  height    = -1;
  kkint32  pixelSize = -1;
  kkint32  width     = -1;

  {
    // We are going to read in header part of file.
    KKStr  nextLine = "";
    bool  eof = false;
    nextLine = osReadRestOfLine2 (i, eof);
    if  (eof  ||  (nextLine[(kkint16)0] != 'P')  ||  (nextLine[(kkint16)1] != '5'))
    {
      std::fclose (i);
      return NULL;
    }

    kkint32  headerFieldsRead = 0;
    nextLine = osReadRestOfLine2 (i, eof);
    while  (!eof)
    {
      if  (nextLine[(kkint16)0] != '#')
      {
        if  (headerFieldsRead == 0)
        {
          width  = nextLine.ExtractTokenInt (" \t\n\r");
          height = nextLine.ExtractTokenInt (" \t\n\r");
        }
        if  (headerFieldsRead == 1)
        {
          pixelSize = nextLine.ToInt ();
          break;
        }
        headerFieldsRead++;
      }
      nextLine = osReadRestOfLine2 (i, eof);
    }
  }

  if  ((pixelSize < 0)  ||  (pixelSize > 255)  ||
       (width     < 1)  ||  (height < 1)
      )
  {
    std::fclose (i);
    cerr << endl << endl
      << "ReadImagePGM   ***ERROR***  ImageFile[" << imageFileName << "]  Invalid Header"
      << "  width[" << width << "]  height[" << height << "]  pixelSize[" << pixelSize << "]"
      << endl
      << endl;
    return  NULL;
  }

  RasterPtr  image = new Raster (height, width, false);

  kkint32  row, col;

  uchar* colBuff = new uchar[width + 10];

  for  (row = 0;  row < height;  row++)
  {
    std::fread (colBuff, 1, width, i);

    for  (col = 0;  col < width;  col++)
    {
      image->SetPixelValue (row, col, colBuff[col]);
    }
  }

  std::fclose (i);
     
  return  image;
}  /* ReadImagePGM */



KKStr  KKB::ReadImagePpmField (FILE*   in,
                               bool&   eof
                              )
{
  eof = false;
  char  token[2048];
  auto  maxTokenLen = sizeof (token) - 1;

  kkint32  ch = 0;

  bool  startOfTokenFound = false;
  while  (!startOfTokenFound)
  {
    // Skip leading white space
    ch = std::fgetc (in);  eof = (std::feof (in) != 0);
    while  ((!eof)  &&  (strchr (" #\t\n\r", ch) != NULL)  &&  (ch != '\n')  &&  (ch != '#'))
      {ch = std::fgetc (in); eof = (std::feof (in)!= 0);}

    if  (eof)  
      return KKStr::EmptyStr ();

    else if  (ch == '#')
    {
      // Skip the rest of the line.
      while  ((!eof)  &&  (ch != '\n'))
        {ch = std::fgetc (in); eof = (std::feof (in)!= 0);}
    }

    else
    {
      startOfTokenFound = true;
    }
  }

  kkuint64 tokenLen = 0;

  // Read till first delimiter or eof or eol
  while  ((!eof)  &&  (strchr (" #\t\n\r", ch) == 0))
  {
    token[tokenLen] = (uchar)ch;
    tokenLen++;
    if  (tokenLen >= maxTokenLen)
      break;
    ch = fgetc (in); eof = (feof (in)!= 0);
  }
  token[tokenLen] = 0;  // Terminating NULL character.
  if  ((!eof)  &&  (ch == '#'))
  {
    // Skip the rest of the line.
    while  ((!eof)  &&  (ch != '\n'))
      {ch = fgetc (in); eof = (feof (in)!= 0);}
  }

  if  (eof  &  (tokenLen > 0))
  {
    eof = false;
    ungetc ('\n', in);
  }
  return  token;
}  /* ReadImagePpmField */



RasterPtr  KKB::ReadImagePPM (const KKStr& imageFileName)
{
  FILE* i = osFOPEN (imageFileName.Str (), "rb");
  if  (!i)
    return NULL;

  kkint32  height     = -1;
  kkint32  pixelDepth = -1;
  kkint32  width      = -1;

  bool  eof = false;
  
  char  buff[10];
  kkint32  bytesRead = (kkint32)fread (buff, 1, 2, i);
  if  (bytesRead < 2)
  {
    fclose (i);
    return NULL;
  }

  bool  p3Format = ((buff[0] == 'P')  &&  (buff[1] == '3'));
  bool  p6Format = ((buff[0] == 'P')  &&  (buff[1] == '6'));

  if  (!p3Format  &&  !p6Format)
  {
    fclose (i);
    return NULL;
  }

  kkint32  fieldNum = 0;
  KKStr  field =  ReadImagePpmField (i, eof);
  while  ((fieldNum  < 3)  &&  (!eof))
  {
    switch  (fieldNum)
    {
    case  0: width      = field.ToInt32 ();   break;
    case  1: height     = field.ToInt32 ();   break;
    case  2: pixelDepth = field.ToInt32 ();   break;
    }
    ++fieldNum;
    field =  ReadImagePpmField (i, eof);
  }

  if  ((width < 1)  ||  (height < 1)  ||  (pixelDepth < 1)  ||  (pixelDepth > 65535))
  {
    cerr << endl << "ReadImagePPM   ***ERROR***    Invalid Header Fields:  "
      << "FileName[" << imageFileName << "]  "
      << "Width["  << width << "]  Height[" << height << "]  PixelDepth[" << pixelDepth << "]"
      << endl
      << endl;
    fclose (i);
    return NULL;
  }

  RasterPtr  image = new Raster (height, width, true);
  int  totalPixels = height * width;

  kkint32  pixelsRead = 0;

  uchar*  red   = image->RedArea ();
  uchar*  green = image->RedArea ();
  uchar*  blue  = image->RedArea ();
  if  (p3Format)
  {
    while  ((pixelsRead < totalPixels)  &&  (!eof))
    {
      KKStr  redField   = osReadNextToken (i, "\n\r\t ", eof);
      KKStr  greenField = osReadNextToken (i, "\n\r\t ", eof);
      KKStr  blueField  = osReadNextToken (i, "\n\r\t ", eof);

      kkint32  redValue    = Min (redField.ToInt32   (), pixelDepth);
      kkint32  greenValue  = Min (greenField.ToInt32 (), pixelDepth);
      kkint32  blueValue   = Min (blueField.ToInt32  (), pixelDepth);

      *red   = (uchar)((255 *  redValue)   / pixelDepth);   ++red;
      *green = (uchar)((255 *  greenValue) / pixelDepth);   ++green;
      *blue  = (uchar)((255 *  blueValue)  / pixelDepth);   ++blue;

      ++pixelsRead;
    }
  }
  else
  {
    char  rgbBuff[3];
    while  ((pixelsRead < totalPixels)  &&  (!eof))
    {
      fread (rgbBuff, 1, 3, i);  eof = (feof (i) != 0);
      *red   = rgbBuff[0];   ++red;
      *green = rgbBuff[1];   ++green;
      *blue  = rgbBuff[2];   ++blue;
      ++pixelsRead;
    }
  }

  fclose (i);
     
  return  image;
}  /* ReadImagePPM */



void  KKB::SaveImage  (const Raster&  image, 
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



void  KKB::SaveImagePGM (const Raster&  image, 
                         const KKStr&   imageFileName
                        )
{
  FILE* o = osFOPEN (imageFileName.Str (), "wb");

  {
    KKStr  headerStr (15);
    headerStr << "P5"            << endl
              << image.Width ()  << endl
              << image.Height () << endl
              << (kkint16)255    << endl;

    const char* h = headerStr.Str ();
    fwrite (h, 1, headerStr.Len (), o);
  }

  kkint32  totalPixels = image.Height () * image.Width ();

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



void  KKB::SaveImagePNG (const Raster&  image, 
                         const KKStr&   imageFileName
                        )
{
  FILE* o = osFOPEN (imageFileName.Str (), "wb");
  KKCheck (o, "SaveImagePNG   Error opening File: " << imageFileName)
 
  {
    KKStr  headerStr (15);
    headerStr << "P6"            << endl
              << image.Width ()  << endl
              << image.Height () << endl
              << (kkint16)255    << endl;

    const char* h = headerStr.Str ();
    fwrite (h, 1, headerStr.Len (), o);
  }

  kkint32  totalPixels = image.Height () * image.Width ();

  if  (image.Color ())
  {
    const uchar*  red   = image.RedArea   ();
    const uchar*  green = image.GreenArea ();
    const uchar*  blue  = image.BlueArea  ();

    for  (kkint32 x = 0;  x < totalPixels;  x++)
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

    for  (kkint32 x = 0;  x < totalPixels;  x++)
    {
      uchar  intensity = (uchar)(255 - (*green));
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



void  KKB::SaveImagePPM (const Raster&  image, 
                         const KKStr&   imageFileName
                        )
{
  FILE* o = osFOPEN (imageFileName.Str (), "wb");
  KKCheck (o, "SaveImagePPM   Error opening File: " << imageFileName)
 
  {
    KKStr  headerStr (15);
    headerStr << "P6"            << endl
              << image.Width ()  << endl
              << image.Height () << endl
              << (kkint16)255    << endl;

    const char* h = headerStr.Str ();
    fwrite (h, 1, headerStr.Len (), o);
  }

  kkint32  totalPixels = image.Height () * image.Width ();

  if  (image.Color ())
  {
    const uchar*  red   = image.RedArea   ();
    const uchar*  green = image.GreenArea ();
    const uchar*  blue  = image.BlueArea  ();

    for  (kkint32 x = 0;  x < totalPixels;  x++)
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

    for  (kkint32 x = 0;  x < totalPixels;  x++)
    {
      uchar  intensity = *green;
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
}  /* SaveImagePPM */


#endif



void  KKB::SaveImageInverted (Raster&       raster, 
                              const KKStr&  imageFileName
                             )
{
  RasterPtr  invertedImage = new Raster (raster);

  kkint32  r, c;

  uchar**  g    = invertedImage->Green ();
  uchar**  red  = invertedImage->Red   ();
  uchar**  blue = invertedImage->Blue  ();

  for  (r = 0; r < invertedImage->Height ();  r++)
  {
    for  (c = 0;  c < invertedImage->Width ();  c++)
    {
      g[r][c] = (uchar)(255 - g[r][c]);
      if  (invertedImage->Color ())
      {
        red [r][c] = (uchar)(255 - red [r][c]);
        blue[r][c] = (uchar)(255 - blue[r][c]);
      }
    }
  }

  SaveImage (*invertedImage, imageFileName);
  delete  invertedImage;
  invertedImage = NULL;
}  /* SaveImageInverted */



void  KKB::SaveImageGrayscaleInverted4Bit (const Raster&  image, 
                                           const KKStr&  _fileName
                                          )
{
  KKStr  ext = osGetFileExtension (_fileName);
  KKCheck (ext.EqualIgnoreCase ("BMP"), "SaveImageGrayscaleInverted4Bit   Only 'BMP' files are supported;  FileName: " << _fileName)

  try
  {
    BmpImage  bmpImage (image);
    bmpImage.SaveGrayscaleInverted4Bit (_fileName);
  }
  catch  (const KKException&  errMsg)
  {
    KKStr msg = "SaveImageGrayscaleInverted4Bit  Exception  Saving image using 'BmpImage'  for file[" + _fileName + "]";
    cerr << std::endl << msg << std::endl << std::endl;
    throw KKException (msg, errMsg);
  }
  catch (...)
  {
    KKStr  errMsg = "SaveImageGrayscaleInverted4Bit   Exception occurred calling 'BmpImage::Save'  for file[" + _fileName + "]";
    cerr << std::endl << errMsg << std::endl << std::endl;
    throw  KKException (errMsg);
  }
}  /* SaveImageGrayscaleInverted4Bit */



/**
 *@brief  Saves image as BMP using compressed GrayScale where Background = 255 and foreground = 0
 *@details  If image is color will convert to GrayScale 1st.
 * Palette will be set to 0 = 255, 1 = 254, 2 = 253, ...  255 = 0.
 */
void  KKB::SaveImageGrayscaleInverted8Bit (const Raster&  image, 
                                           const KKStr&  _fileName
                                          )
{
  KKStr  ext = osGetFileExtension (_fileName);
  if  (!ext.EqualIgnoreCase ("BMP"))
  {
    KKStr msg;
    msg << "KKB::SaveImageGrayscaleInverted4Bit   Only 'BMP' files are supported;  FileName[" << _fileName << "].";
    cerr << endl << "***ERROR***    " << msg << endl << endl;
    throw KKException (msg);
  }

  try
  {
    BmpImage  bmpImage (image);
    bmpImage.SaveGrayscaleInverted8Bit (_fileName);
  }
  catch  (const KKException&  errMsg)
  {
    KKStr msg = "SaveImageGrayscaleInverted8Bit  Exception  Saving image using 'BmpImage'";
    cerr << std::endl << "***ERROR***    " << msg << std::endl << std::endl;
    throw KKException (msg, errMsg);
  }
  catch (...)
  {
    KKStr  errMsg = "SaveImageGrayscaleInverted8Bit   Exception occurred calling 'BmpImage::Save'  for file[" + _fileName + "]";
    cerr << std::endl << errMsg << std::endl << std::endl;
    throw  KKException (errMsg);
  }
}  /* SaveImageGrayscaleInverted4Bit */



bool  KKB::SupportedImageFileFormat (const KKStr&  imageFileName)
{
  KKStr  ext = osGetFileExtension (imageFileName);
  ext.Upper ();

  return  (
           (ext == "BMP")  ||
           (ext == "PGM")  ||
           (ext == "PNG") 
          );
}



void  KKB::ImageIoFinaleCleanUp ()
{
#if  defined(KKOS_WINDOWS)
  if  (gdiStarted)
  {
    GdiplusShutdown (gdiplusToken);
    gdiStarted = false;
  }
#endif
}
