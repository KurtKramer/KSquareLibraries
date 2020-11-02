/* BMPImage.cpp -- Manages the reading and writing of BMP image files.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"


#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <string.h>
#include <string>
#include <vector>
#include "MemoryDebug.h"
using namespace std;


#ifdef WIN32
#include <windows.h>
#else
#include <cstdio>
#endif


#include "BMPImage.h"
#include "KKException.h"
#include "OSservices.h"
#include "Raster.h"
using namespace KKB;


#ifndef  WIN32

union  BmpImage::WordParts
{
  WORD   w;
  struct  
  {
     uchar  c1;
     uchar  c2;
  }  parts;
};


union  BmpImage::DWordParts
{
  DWORD   w;
  struct  
  {
     uchar  c1;
     uchar  c2;
     uchar  c3;
     uchar  c4;
  }  parts;
};



union  BmpImage::LongParts
{
  LONG   l;
  struct  
  {
     uchar  c1;
     uchar  c2;
     uchar  c3;
     uchar  c4;
  }  parts;
};



void  RotateWORD (WORD&  w)
{
  BmpImage::WordParts& wp = reinterpret_cast<BmpImage::WordParts&>(w);

  BmpImage::WordParts  temp = wp;

  wp.parts.c1 = temp.parts.c2;
  wp.parts.c2 = temp.parts.c1;
}  /* RotateWORD */



void  RotateDWORD (DWORD&  w)
{
  BmpImage::DWordParts& wp = reinterpret_cast<BmpImage::DWordParts&>(w);

  BmpImage::DWordParts  temp = wp;

  wp.parts.c1 = temp.parts.c4;
  wp.parts.c2 = temp.parts.c3;
  wp.parts.c3 = temp.parts.c2;
  wp.parts.c4 = temp.parts.c1;
}  /* RotateDWORD */



void  RotateLONG (LONG&  l)
{
  BmpImage::LongParts& lp = reinterpret_cast<BmpImage::LongParts&>(l);

  BmpImage::LongParts  temp = lp;

  lp.parts.c1 = temp.parts.c4;
  lp.parts.c2 = temp.parts.c3;
  lp.parts.c3 = temp.parts.c2;
  lp.parts.c4 = temp.parts.c1;
}  /* RotateLONG */


void  WriteWORD (FILE*  outFile, WORD w)
{
  RotateWORD (w);
  std::fwrite (&w,  sizeof (w), 1ul, outFile);
}


void  WriteDWORD (FILE*  outFile,  DWORD dw)
{
  RotateDWORD (dw);
  std::fwrite (&dw,  sizeof (dw), 1, outFile);
}



void  WriteLONG (FILE*  outFile, LONG l)
{
  RotateLONG (l);
  std::fwrite (&l,  sizeof (l), 1, outFile);
}


void  WriteBYTE (FILE*  outFile, BYTE b)
{
  std::fwrite (&b,  sizeof (b), 1, outFile);
}
#endif



template<typename T>
DWORD toDWORD (T x)
{
  return static_cast<DWORD> (x);
}



struct  BmpImage::CodePair
{
  uchar  pixel;
  uchar  count;
};



/**
 *@class  BmpImage::CodedPixels
 *@brief  This object is used to help encode the data stored in BMPImage::image into 8 or 4 bit compressed formats used by BMP files.                                            *
 */
class  BmpImage::CodedPixels 
{
public:
  CodedPixels (kkint32  _height,
               kkint32  _width
              );
  ~CodedPixels ();

  void  AddPixel (uchar  pixel);  /*!< This method is called once for each pixel in the BMP image */
                                              
  void  EOL ();                   /*!< This is called once at the end of each row of pixels.      */

  struct PixelDataStruct {kkuint32 len; uchar* buff; int structSize;};

  uchar*  CreatePixelDataStructure4Bit (kkint32&  len);  /**< Creates the appropriate compressed data structure for the BMP 4-bit
                                                          * compressed file format.  The length of the structure created will be
                                                          * returned in the 'len' parameter.  The caller will be responsible for
                                                          * deleting the returned data structure.
                                                          */

  uchar*  CreatePixelDataStructure8Bit (kkint32&  len);  /**< Creates the appropriate compressed data structure for the BMP 8-bit
                                                          * compressed file format.  The length of the structure created will be
                                                          * returned in the 'len' parameter.  The caller will be responsible for
                                                          * deleting the returned data structure.
                                                          */

private:
  kkint32  height;
  kkint32  width;
  
  kkint32  codesAllocated;
  kkint32  used;

  CodePairPtr codes;
  CodePairPtr top;
};



BmpImage::CodedPixels::CodedPixels (kkint32 _height,
                                    kkint32 _width
                                   ):
  height (_height),
  width  (_width),
  top (NULL)
{

  codesAllocated = height * (width + 4);
  used = 0;

  codes = new CodePair[codesAllocated];
}



BmpImage::CodedPixels::~CodedPixels ()
{
  delete codes;
}



void  BmpImage::CodedPixels::AddPixel (uchar  pixel)
{
  if  (used >= codesAllocated)
  {
    // We are exceeding the codes Size.
    cerr << "CodedPixels::AddPixel *** Error ***, Exceeding  codesAllocated." << std::endl;
    return;
  }

  if  (!top)
  {
    top = &(codes[0]);
    used++;
    top->count = 1;
    top->pixel = pixel;
  }

  else if  ((pixel != top->pixel)  ||  (top->count == 0))
  {
    used++;
    top++;
    top->count = 1;
    top->pixel = pixel;
  }

  else 
  {
    if  (top->count >= 254)
    {
      used++;
      top++;
      top->count = 1;
      top->pixel = pixel;
    }
    else
    {
      top->count++;
    }
  }
}  /* AddPixel */



void  BmpImage::CodedPixels::EOL ()
{
  used++;
  top++;
  top->count = 0;
  top->pixel = 0;
}  /* EOL */




uchar*  BmpImage::CodedPixels::CreatePixelDataStructure4Bit (kkint32&  len)
{
  kkint32  buffSize = codesAllocated;

  uchar* buff = new uchar [buffSize + 4];  // + 4 For Safety
  memset (buff, 0, tosize_t (codesAllocated));

  kkint32  curPair = 0;
  top = &(codes[0]);

  kkint32  bp = 0;

  while  (curPair < used)
  {
    if  (bp >= buffSize)
    {
      kkint32  newBuffSize = buffSize * 2;
      uchar* newBuff = new uchar[newBuffSize];
      memset (newBuff, 0, tosize_t (newBuffSize));
      memcpy (newBuff, buff, tosize_t (buffSize));
      delete[] buff;
      buff = newBuff;
      newBuff = NULL;
      buffSize = newBuffSize;
    }


    {
      if  ((top->count == 0)  &&  (top->pixel == 0))
      {
        // Add End Of Line 
        buff[bp] = 0;
        bp++;
        buff[bp] = 0;
        bp++;

        curPair++;
        top++;
      }

      else
      {
        if  (top->count > 254)
        {
          buff[bp] = 254;
          bp++;

          buff[bp] = static_cast<uchar>(top->pixel * 16 + top->pixel);
          bp++;

          top->count = static_cast<uchar>(top->count - 254);
        }

        else
        {
          buff[bp] = top->count;
          bp++;

          buff[bp] = static_cast<uchar>(top->pixel * 16 + top->pixel);
          bp++;

          curPair++;
          top++;
        }
      }
    }
  }

  buff[bp] = 0;
  bp++;

  buff[bp] = 1;
  bp++;

  len = bp;
  return  buff;
} /* CreatePixelDataStructure */



uchar*  BmpImage::CodedPixels::CreatePixelDataStructure8Bit (kkint32&  len)
{
  kkint32  buffSize = codesAllocated;

  uchar* buff = new uchar [buffSize + 4];  // + 4 For Safety
  memset (buff, 0, tosize_t (codesAllocated));

  kkint32  curPair = 0;
  top = &(codes[0]);

  kkint32  bp = 0;

  while  (curPair < used)
  {
    if  (bp >= (buffSize - 1))
    {
      kkint32  newBuffSize = buffSize * 2;
      uchar* newBuff = new uchar[newBuffSize + 2];
      if  (newBuff == NULL)
      {
        KKStr  errMsg = "BmpImage::CodedPixels::CreatePixelDataStructure8Bit    ***ERROR***     Allocation of 'newBuff'  failed.";
        cerr << std::endl << std::endl << errMsg << std::endl << std::endl;
        throw  KKException (errMsg);
      }

      memset (newBuff, 0,    tosize_t (newBuffSize));
      memcpy (newBuff, buff, tosize_t (buffSize));
      delete[] buff;
      buff = newBuff;
      newBuff = NULL;
      buffSize = newBuffSize;
    }


    {
      if  ((top->count == 0)  &&  (top->pixel == 0))
      {
        // Add End Of Line 
        buff[bp] = 0;
        bp++;
        buff[bp] = 0;
        bp++;

        curPair++;
        top++;
      }

      else
      {
        if  (top->count > 254)
        {
          buff[bp] = 254;
          bp++;

          buff[bp] = top->pixel;
          bp++;

          top->count = static_cast<uchar>(top->count - 254);
        }

        else
        {
          buff[bp] = top->count;
          bp++;

          buff[bp] = top->pixel;
          bp++;

          curPair++;
          top++;
        }
      }
    }
  }


  buff[bp] = 0;
  bp++;

  buff[bp] = 1;
  bp++;

  len = bp;     
  return  buff;
} /* CreatePixelDataStructure */



BmpImage::BmpImage (const KKStr&  _fileName,
                    bool&         successfull
                   ):

  color          (false),
  fileName       (_fileName),
  red            (NULL),
  image          (NULL),
  blue           (NULL),
  numOfColors    (16),
  palette        (NULL),
  paletteEntries (0)

{
  FILE*  inFile = osFOPEN (fileName.Str (), "rb");
  if  (!inFile)
  {
    successfull = false;
    return;
  }

  successfull = true;

  kkint32  y;

  if (std::fread (&hdr, sizeof (hdr), 1, inFile) <= 0)
  {
    successfull = false;
    std::fclose (inFile);
    return;
  }

  uchar  buff[4];
  memcpy (buff, &hdr, sizeof (buff));
  if  ((buff[0] == 'B')  &&  (buff[1] == 'M'))
  {
    // We have a Bit Map file.
  }
  else if  ((buff[0] == 137)  &&  (buff[1] == 'P')  &&  (buff[2] == 'N')  &&  (buff[3] == 'G'))
  {
    // We are looking at a PNG (Portable Network Graphics) file.
    cerr << std::endl 
         << "File[" << _fileName << "]  is a PNG formatted file." << std::endl
         << std::endl;
    successfull = false;
    std::fclose (inFile);
    return;
  }
  else
  {
    cerr << std::endl 
         << "File[" << _fileName << "]  is of a unknown file format." << std::endl
         << std::endl;
    successfull = false;
    std::fclose (inFile);
    return;
  }
  
  if  (std::fread (&bmh, sizeof (bmh), 1, inFile) <= 0)
  {
    successfull = false;
    std::fclose (inFile);
    return;
  }

  if  ((bmh.biCompression == BI_RGB)  &&  (bmh.biBitCount == 24))
  {
    numOfColors = 16777216;
    color = true;
    delete  palette;
    palette = NULL;
    Load24BitColor (inFile, successfull);
    std::fclose (inFile);
    return;
  }

  if  ((bmh.biCompression == BI_RGB)  &&  (bmh.biBitCount == 8))
  {
    Load8BitColor (inFile, successfull);
    std::fclose (inFile);
    return;
  }

  else if  ((bmh.biCompression == BI_RGB)  ||  (bmh.biCompression == BI_RLE4))
    numOfColors = 16;

  else  if  (bmh.biBitCount == 1)
    numOfColors = 16;

  else
    numOfColors = 256;


  paletteEntries = BMIcolorArraySize (bmh);
  if  (paletteEntries < 0)
  {
    successfull = false;
    std::fclose (inFile);
    return;
  }

  bool  imageIsRevGrayscale = false;
  if  (paletteEntries)
  {
    palette =  new RGBQUAD[paletteEntries];
    std::fread (palette, sizeof (RGBQUAD), tosize_t (paletteEntries), inFile);
    imageIsRevGrayscale = ReversedGrayscaleImage ();
  }
  else
  {
    numOfColors   = 256;
    paletteEntries = 256;
    palette = new RGBQUAD[paletteEntries];
    SetUp256BitPalette (palette);
  }

  // Lets Build Palette Map
  for  (int x = 0; x < 256; ++x)
    paletteMap[x] = 0;

  if  (numOfColors == 16)
  {
    for  (kkint32 palletIdx = 0; palletIdx < paletteEntries; palletIdx++)
    {
      if  (imageIsRevGrayscale)
        y = 255 - palette[palletIdx].rgbGreen;
      else
        y = palette[palletIdx].rgbGreen;
      if  (y < 0)  y = 0;  else if  (y > 255)  y = 255;
      paletteMap[palletIdx] = y;
    }
  }
  else
  {
    for  (kkint32 palletIdx = 0; palletIdx < paletteEntries; palletIdx++)
    {
      if  (imageIsRevGrayscale)
        y = 255 - palette[palletIdx].rgbGreen;
      else
        y = palette[palletIdx].rgbGreen;
      if  (y < 0)  y = 0;  else if  (y > 255)  y = 255;
      paletteMap[palletIdx] = y;
    }
  }

  delete palette;
  paletteEntries = 256;
  palette = new RGBQUAD[paletteEntries];
  SetUp256BitPalette (palette);

  kkint32  row;

  image = new uchar*[bmh.biHeight];
  for  (row = 0; row < bmh.biHeight; row++)
  {
    image[row] = new uchar[bmh.biWidth];
    memset (image[row], 0, tosize_t (bmh.biWidth));
  }

  std::fseek (inFile, static_cast<long> (hdr.bfOffBits), SEEK_SET);

  if  (bmh.biBitCount == 1)
  {
    Load1BitColor (inFile, successfull);
  }

  else if  (bmh.biBitCount == 4)
  {
    if  (bmh.biCompression == BI_RGB)
    {
      Load4BitColor (inFile, successfull);
    }

    else if  (bmh.biCompression == BI_RLE4)
    {
      Load4BitColorCompressed (inFile, successfull);
    }

    else
    {
      cerr << "***ERROR***  Invalid Compression Mode[" << bmh.biCompression 
           << "] Specified for 4 bit File["
           << fileName << "]."
           << std::endl;
      successfull = false;
      // WaitForEnter ();
    }
  }

  else if  (bmh.biBitCount == 8)
  {
    if  (bmh.biCompression == BI_RGB)
    {
      Load8BitColor (inFile, successfull);
    }

    else if  (bmh.biCompression == BI_RLE8)
    {
      Load8BitColorCompressed (inFile, successfull);
    }

    else
    {
      cerr << "***ERROR***  Invalid Compression Mode[" << bmh.biCompression 
           << "] Specified for 8 bit File["
           << fileName << "]."
           << std::endl;
      successfull = false;
    }
  }

  else if  (bmh.biBitCount == 16)
  {
    cerr << "***ERROR***  16 Bit Not Supported.  File[" << fileName << "]." << std::endl;
    // WaitForEnter ();
  }

  else if  (bmh.biBitCount == 24)
  {
    cerr << "***ERROR***  24 Bit Not Supported.  File[" << fileName << "]."  << std::endl;
  }

  else if  (bmh.biBitCount == 32)
  {
    cerr << "***ERROR***  32 Bit Not Supported.  File[" << fileName << "]."  << std::endl;
    // WaitForEnter ();
  }

  std::fclose (inFile);
}



BmpImage::BmpImage (kkint32  _height,
                    kkint32  _width,
                    kkint32  _numOfColors
                   ):
   
   color       (false),
   fileName    (),
   red         (NULL),
   image       (NULL),
   blue        (NULL),
   numOfColors (_numOfColors),
   palette     (NULL)
{
  InitializeFields (_height, _width);
}



BmpImage::BmpImage (const Raster&  raster):
  color   (false),
  red     (NULL),
  image   (NULL),
  blue    (NULL),
  palette (NULL)
{
  numOfColors = 256;  // kk 2005-03-10

  color = raster.Color ();

  InitializeFields (raster.Height (), raster.Width ());
  fileName = "";

  kkint32  row;
  kkint32  col;

  uchar** rasterData = raster.Rows ();
  uchar** rasterRed  = raster.Red ();
  uchar** rasterBlue = raster.Blue ();

  for  (row = 0; row < bmh.biHeight; row++)
  {
    for  (col = 0; col < bmh.biWidth; col++)
    {
      image[row][col] = rasterData[row][col];
      if  (color)
      {
        red[row][col]  = rasterRed [row][col];
        blue[row][col] = rasterBlue[row][col];
      }
    }
  }
}



BmpImage::~BmpImage ()
{
  CleanUpMemory ();
}



/**
 *@brief  Returns true if BMP file is a a reversed GrayScale Image.
 *@details  The original version in PICES had special checks to detect if a SIPPER image;  if
 * there are issues with this routine suggest comparing with BasiLibrary in PICES.
 */
bool  BmpImage::ReversedGrayscaleImage ()
{
  bool  allChannelsSameValue = true;
  bool  alwaysDecending      = true;   // Reversed images pixel values will always be decending.
  //kkint32  firstUnbalancedChannel = (kkint32)0;

  for  (kkint32 x = 0;  (x < paletteEntries)  &&  allChannelsSameValue;  ++x)
  {
    RGBQUAD&   p = (palette[x]);
    allChannelsSameValue = (p.rgbBlue == p.rgbGreen)  &&  (p.rgbBlue == p.rgbRed)  &&  (p.rgbGreen == p.rgbRed);
    //firstUnbalancedChannel = x;
    if  ((x > 0)  &&  (p.rgbBlue >= palette[x - 1].rgbBlue))
      alwaysDecending = false;
  }

  if  (allChannelsSameValue  &&  alwaysDecending  &&  (palette[0].rgbBlue == 255)  &&  (palette[paletteEntries - 1].rgbBlue == 0))
    return true;
  else
    return false;
}  /* ReversedGrayscaleImage */



kkint32  BmpImage::BMIcolorArraySize (BITMAPINFOHEADER&  _bmh)

{
  if  (_bmh.biBitCount == 0)
  {
    return -1;
  }

  else if  (_bmh.biBitCount == 1)
  {
    return 2;
  }

  else if  (_bmh.biBitCount == 4)
  {
    return 16;
  }

  else if  (_bmh.biBitCount == 8)
  {
    if  (_bmh.biClrUsed == 0)
      return 255;
    else
      return static_cast<kkint32> (_bmh.biClrUsed);
  }

  else if  (_bmh.biBitCount == 16)
  {
    if  (_bmh.biCompression ==  BI_RGB)
      return  0;
  }

  else if  (_bmh.biBitCount == 24)
  {
    return  static_cast<kkint32> (_bmh.biClrUsed);
  }

  return  -1;
}



void  BmpImage::SetUp4BitPallet ()
{
  delete  palette;   palette = NULL;
  paletteEntries = 16;
  palette = new RGBQUAD[paletteEntries];

  uchar  pixelVal = 255;
  for  (kkint32 x = 0;  x < 16;  ++x)
  {
    palette[x].rgbBlue   = pixelVal;
    palette[x].rgbGreen  = pixelVal;
    palette[x].rgbRed    = pixelVal;
    pixelVal = static_cast<uchar> (pixelVal - 17);
  }
}  /* SetUp4BitPallet*/



void  BmpImage::SetUp8BitPallet ()
{
  delete  palette;   palette = NULL;
  paletteEntries = 256;
  palette = new RGBQUAD[paletteEntries];

  uchar  pixelVal = 255;
  for  (kkint32 x = 0;  x < 256;  ++x)
  {
    palette[x].rgbBlue  = pixelVal;
    palette[x].rgbGreen = pixelVal;
    palette[x].rgbRed   = pixelVal;
    --pixelVal;
  }
}  /* SetUp8BitPallet */



void  BmpImage::SetUp16BitPallet (RGBQUAD*  pal)
{
  pal[0].rgbBlue  = 255;
  pal[0].rgbGreen = 255;
  pal[0].rgbRed   = 255;

  pal[1].rgbBlue  = 219;
  pal[1].rgbGreen = 219;
  pal[1].rgbRed   = 219;

  pal[2].rgbBlue  = 182;
  pal[2].rgbGreen = 182;
  pal[2].rgbRed   = 182;

  pal[3].rgbBlue  = 146;
  pal[3].rgbGreen = 146;
  pal[3].rgbRed   = 146;

  pal[4].rgbBlue  = 109;
  pal[4].rgbGreen = 109;
  pal[4].rgbRed   = 109;

  pal[5].rgbBlue  = 73;
  pal[5].rgbGreen = 73;
  pal[5].rgbRed   = 73;

  pal[6].rgbBlue  = 36;
  pal[6].rgbGreen = 36;
  pal[6].rgbRed   = 36;

  pal[7].rgbBlue  = 0;
  pal[7].rgbGreen = 0;
  pal[7].rgbRed   = 0;

  pal[8].rgbBlue  = 255;
  pal[8].rgbGreen = 40;
  pal[8].rgbRed   = 40;

  pal[9].rgbBlue  = 40;
  pal[9].rgbGreen = 40;
  pal[9].rgbRed   = 255;

  pal[10].rgbBlue  = 40;
  pal[10].rgbGreen = 210;
  pal[10].rgbRed   = 40;

  pal[11].rgbBlue  = 40;
  pal[11].rgbGreen = 210;
  pal[11].rgbRed   = 210;

  pal[12].rgbBlue  = 210;
  pal[12].rgbGreen = 40;
  pal[12].rgbRed   = 40;

  pal[13].rgbBlue  = 210;
  pal[13].rgbGreen = 40;
  pal[13].rgbRed   = 210;

  pal[14].rgbBlue  = 210;
  pal[14].rgbGreen = 210;
  pal[14].rgbRed   = 40;

  pal[15].rgbBlue  = 210;
  pal[15].rgbGreen = 210;
  pal[15].rgbRed   = 210;
}  /* SetUp16BitPallet */



void  BmpImage::SetPaletteEntry (kkint32            palletIndex,
                                 const PixelValue&  pixValue
                                )
{
  if  ((palletIndex < 0)  ||  (palletIndex >  255))
  {
    // Invalid Entry specified
    cerr << std::endl << std::endl << std::endl 
         << "BmpImage::SetPaletteEntry      Invalid PalletIndex[" << palletIndex << "]" << std::endl
         << std::endl;

    return;
  }

  if  (!palette)
  {
    cerr << std::endl << std::endl << std::endl 
         << "BmpImage::SetPaletteEntry      The palette is not defined!" << std::endl
         << std::endl;

    return;
  }

  palette[palletIndex].rgbBlue  = pixValue.b;
  palette[palletIndex].rgbGreen = pixValue.g;
  palette[palletIndex].rgbRed   = pixValue.r;
}  /*   SetPaletteEntry */
 


void  BmpImage::SetUp256BitPalette (RGBQUAD*  pal)
{
  for  (kkuint32 x = 0; x < 256; x++)
  {
    uchar pixVal = static_cast<uchar> (x);    
    pal[x].rgbBlue  = pixVal;
    pal[x].rgbGreen = pixVal;
    pal[x].rgbRed   = pixVal;
  }
}  /* SetUp256BitPalette */



/**
 *@brief  Returns true if palette is for a grayscale image.
 */
bool  GrayScaleImage (RGBQUAD*  palette,
                      kkint32   palletSize
                     )
{
  bool  isGrayScaleImage = true;

  for  (kkint32 x = 0;  ((x < palletSize)  &&  isGrayScaleImage);  ++x)
  {
    kkint32  expectedColor = 255 - x;
    if  ((palette[x].rgbBlue  != expectedColor)  ||
         (palette[x].rgbGreen != expectedColor)  ||
         (palette[x].rgbRed   != expectedColor)
        )
    {
      isGrayScaleImage = false;
    }
  }

  return  isGrayScaleImage;
}  /* GrayScaleImage */



void  BmpImage::InitializeFields (kkint32  _height,
                                  kkint32  _width
                                 )
{
  hdr.bfType       = 19778;
  hdr.bfReserved1  = 0;
  hdr.bfReserved2  = 0;
  // hdr.bfOffBits    = sizeof (bmh)

  bmh.biSize       = sizeof (bmh);
  bmh.biWidth      = _width;
  bmh.biHeight     = _height;
  bmh.biPlanes     = 1;

  paletteEntries = 0;

  AllocateRaster ();

  if  (numOfColors <= 16)
  {
    delete  palette;  palette = NULL;
    bmh.biBitCount    = 4;
    paletteEntries    = 16;
    bmh.biCompression = BI_RLE4; // BI_RGB;
    palette = new RGBQUAD[paletteEntries];

    SetUp16BitPallet (palette);
  }
  else
  {
    delete  palette;  palette = NULL;
    bmh.biBitCount    = 8;
    paletteEntries     = 256;
    bmh.biCompression = BI_RLE8; // BI_RGB;
    palette = new RGBQUAD[paletteEntries];
    SetUp256BitPalette (palette);
  }

  bmh.biSizeImage       = 0;
  bmh.biXPelsPerMeter   = 2835;
  bmh.biYPelsPerMeter   = 2835;
  bmh.biClrUsed         = 0;
  bmh.biClrImportant    = 0;

  hdr.bfOffBits  = static_cast<DWORD> (sizeof (bmh) + sizeof (RGBQUAD) * tosize_t (paletteEntries));
}  /* InitializeFields */



class  BmpImage::PalletBuilder
{
public:
  class  RGBQUAD_Pred
  {
  public:
    bool  operator() (const RGBQUAD& left,
                      const RGBQUAD& right
                     )  const
    {
      if  (left.rgbRed != right.rgbRed)
        return (left.rgbRed < right.rgbRed);

      if  (left.rgbGreen != right.rgbGreen)
        return (left.rgbGreen < right.rgbGreen);
      
      return  (left.rgbBlue < right.rgbBlue);
    }
  };


  PalletBuilder ():
	lastColorsSet (false),
	lastRed       (0),
	lastGreen     (0),
	lastBlue      (0)

  {
    lastColorsSet = false;
  }

  kkint32  NumOfColors ()  const  {return static_cast<kkint32> (colorsUsed.size ());}

  kkint32  PalletIndex (uchar red,
                        uchar green,
                        uchar blue
                       )
  {
    RGBQUAD  key;
    key.rgbRed   = red;
    key.rgbGreen = green;
    key.rgbBlue  = blue;
    idx = colorsUsed.find (key);
    if  (idx == colorsUsed.end ())
      return -1;
    else
      return idx->second;
  }


  void  AddColor (uchar red,
                  uchar green,
                  uchar blue
                 )
  {
    if  (lastColorsSet  &&  (red == lastRed)  &&  (green == lastGreen)  &&  (blue == lastBlue))
      return;

    RGBQUAD  key;
    key.rgbRed   = red;
    key.rgbGreen = green;
    key.rgbBlue  = blue;
    idx = colorsUsed.find (key);
    if  (idx == colorsUsed.end ())
    {
      kkint32  numEntries = static_cast<kkint32> (colorsUsed.size ());
      colorsUsed.insert (pair<RGBQUAD,kkint32> (key, numEntries));
    }

    lastRed = red;   lastGreen = green;  lastBlue = blue;
    lastColorsSet = true;
  }


  void  BuildPallet (RGBQUAD*&  palette,
                     kkint32&   size
                    )
  {
    size = static_cast<kkint32> (colorsUsed.size ());
    delete  palette;  palette = NULL;
    if  (size < 1)
      return;

    palette = new RGBQUAD[size];
    for  (idx = colorsUsed.begin ();  idx != colorsUsed.end ();  idx++)
    {
      palette[idx->second] = idx->first;
    }
  }

private:
  map<RGBQUAD, kkint32, RGBQUAD_Pred>            colorsUsed;
  map<RGBQUAD, kkint32, RGBQUAD_Pred>::iterator  idx;

  bool   lastColorsSet;
  uchar  lastRed;
  uchar  lastGreen;
  uchar  lastBlue;
};  /* PalletBuilder */



BmpImage::PalletBuilderPtr  BmpImage::BuildPalletFromRasterData ()
{
  PalletBuilderPtr  palletBuilder = new PalletBuilder ();

  kkint32  row, col;
  uchar*  rowRed   = NULL;
  uchar*  rowGreen = NULL;
  uchar*  rowBlue  = NULL;

  for  (row = 0;  row < bmh.biHeight;  row++)
  {
    rowRed   = red  [row];
    rowGreen = image[row];
    rowBlue  = blue [row];

    for  (col = 0;  col < bmh.biWidth;  col++)
    {
      palletBuilder->AddColor (rowRed[col], rowGreen[col], rowBlue[col]);
    }
  }

  palletBuilder->BuildPallet (palette, paletteEntries);
  return  palletBuilder;
}  /* BuildPalletFromRasterData */



void  BmpImage::Set16Colors ()
{
  if  (palette)
    delete palette;

  numOfColors = 16;
  paletteEntries = 16;
  palette = new RGBQUAD[paletteEntries];
  SetUp16BitPallet (palette);
}



void  BmpImage::Set256Colors ()
{
  if  (palette)
    delete palette;

  numOfColors   = 256;
  paletteEntries = 256;
  palette = new RGBQUAD[paletteEntries];
  SetUp256BitPalette (palette);
}



#ifdef  WIN32
struct  BmpImage::Bmp1BitRec
{
  uchar  pix8: 1;
  uchar  pix7: 1;
  uchar  pix6: 1;
  uchar  pix5: 1;
  uchar  pix4: 1;
  uchar  pix3: 1;
  uchar  pix2: 1;
  uchar  pix1: 1;
};

#else
struct BmpImage::Bmp1BitRec
{
  uchar  pix1: 1;
  uchar  pix2: 1;
  uchar  pix3: 1;
  uchar  pix4: 1;
  uchar  pix5: 1;
  uchar  pix6: 1;
  uchar  pix7: 1;
  uchar  pix8: 1;
};
#endif



void  BmpImage::Load1BitColor (FILE*  inFile,
                               bool&  successfull
                              )
{
  successfull = true;

  kkuint32  bmpRowWidthInBytes = static_cast<kkuint32> (bmh.biWidth) / 8;

  while  ((bmpRowWidthInBytes % 2) > 0)
  {
    bmpRowWidthInBytes++;
  }

  Bmp1BitRec*  packedRowData = new Bmp1BitRec [bmpRowWidthInBytes];
  uchar*       rowData       = new uchar [bmpRowWidthInBytes * 8];

  Bmp1BitRec  b;
  kkuint32    byteNum;
  kkint32     col;
  kkint32     row;
  size_t      x;

  for  (row = bmh.biHeight - 1; row >= 0; row--)
  {
    x = std::fread (packedRowData, 1, bmpRowWidthInBytes, inFile);
    if  (x < bmpRowWidthInBytes)
    {
      successfull = false;
      cerr << "***ERROR***  BmpImage::Load1BitColor  Error Reading File." << std::endl;
      return;
    }

    col = 0;

    for  (byteNum = 0; byteNum < x; byteNum++)
    {
      b = packedRowData[byteNum];

      rowData[col + 0] = b.pix1;
      rowData[col + 1] = b.pix2;
      rowData[col + 2] = b.pix3;
      rowData[col + 3] = b.pix4;
      rowData[col + 4] = b.pix5;
      rowData[col + 5] = b.pix6;
      rowData[col + 6] = b.pix7;
      rowData[col + 7] = b.pix8;

      col = col + 8;
    }

    for  (col = 0; col < bmh.biWidth; col++)
    {
      SetPixelValue (row, col, paletteMap[rowData[col]]);
    }
  }

  ReAllocateForBiggerScreen ();

  delete[]  rowData;
  delete[]  packedRowData;
}  /* Load1BitColor */



void  BmpImage::ReAllocateForBiggerScreen ()
{
  size_t  newHeight = tosize_t (bmh.biHeight + 6);
  size_t  newWidth  = tosize_t (bmh.biWidth  + 6);
  
  uchar**  newImage = new uchar*[newHeight];
  uchar**  newRed   = NULL;
  uchar**  newBlue  = NULL;
  if  (color)
  {
     newRed  = new uchar*[newHeight];
     newBlue = new uchar*[newHeight];
  }

  for  (size_t newRow = 0;  newRow < newHeight;  ++newRow)
  {
    newImage[newRow] = new uchar[newWidth];
    memset (newImage[newRow], 0, newWidth);
    if  (color)
    {
      newRed [newRow] = new uchar[newWidth];
      newBlue[newRow] = new uchar[newWidth];
      memset (newRed  [newRow], 0, newWidth);
      memset (newBlue [newRow], 0, newWidth);
    }
  }

  for  (LONG oldRow = 0, newRow = 3;  oldRow < bmh.biHeight;  ++oldRow, ++newRow)
  {
    memcpy ((newImage[newRow] + 3), image[oldRow], tosize_t (bmh.biWidth));
    if  (color)
    {
      memcpy ((newRed[newRow]  + 3), red [oldRow], tosize_t (bmh.biWidth));
      memcpy ((newBlue[newRow] + 3), blue[oldRow], tosize_t (bmh.biWidth));
    }
  }

  for  (LONG oldRow = 0;  oldRow < bmh.biHeight;  ++oldRow)
  {
    delete  image[oldRow];
    image[oldRow] = NULL;
    if  (color)
    {
      delete  red [oldRow];  red [oldRow] = NULL;
      delete  blue[oldRow];  blue[oldRow] = NULL;
    }
  }
  delete image;
  delete red;
  delete blue;

  image = newImage;
  if  (color)
  {
    red  = newRed;    newRed  = NULL;
    blue = newBlue;   newBlue = NULL;
  }
  bmh.biHeight = static_cast<LONG> (newHeight);
  bmh.biWidth  = static_cast<LONG> (newWidth);
}  /* RealocateForBiggerScreen */



#ifdef WIN32

struct  BmpImage::Bmp4BitRecs
{
  uchar  pix2: 4;
  uchar  pix1: 4;
};

#else
struct  BmpImage::Bmp4BitRecs
{
  uchar  pix1: 4;
  uchar  pix2: 4;
};
#endif



void  BmpImage::Load4BitColor (FILE*  inFile,
                               bool&  successfull
                              )
{
  successfull = true;

  kkint32 height = toint32_t (bmh.biHeight);
  kkint32 width  = toint32_t (bmh.biWidth);

  size_t  bmpRowWidthInBytes = (tosize_t (width) + 1) / 2;

  size_t  paddingBytes = bmpRowWidthInBytes % 4;
  if  (paddingBytes != 0)
    paddingBytes = 4 - paddingBytes;

  bmpRowWidthInBytes = bmpRowWidthInBytes + paddingBytes;

  Bmp4BitRecs*  rowData = new Bmp4BitRecs [bmpRowWidthInBytes];

  for  (kkint32 row = height - 1;  row >= 0;  --row)
  {
    if  (std::fread (rowData, sizeof (Bmp4BitRecs), bmpRowWidthInBytes, inFile) < bmpRowWidthInBytes)
    {
      successfull = false;
      cerr << "***ERROR***  BmpImage::Load4BitColor  Error Reading File" << std::endl;
      return;
    }

    for  (kkint32 col = 0;  col < width;  ++col)
    {
      kkint32  offset = col / 2;

      uchar  nextPixel;

      if  ((col % 2) == 0)
      {
        nextPixel = rowData[offset].pix1;
      }
      else
      {
        nextPixel = rowData[offset].pix2;
      }

      SetPixelValue (row, col, paletteMap[nextPixel]);
    }
  }

  delete[]  rowData;
}  /* Load4BitColor */



void  BmpImage::Load8BitColor (FILE*  inFile,
                               bool&  successfull
                              )
{
  successfull = true;
  color = true;
  numOfColors = 256;
  paletteEntries = numOfColors;
  AllocateRaster ();

  std::fseek (inFile, sizeof (hdr) + sizeof (bmh), SEEK_SET);
  std::fread (palette, sizeof (RGBQUAD), tosize_t (paletteEntries), inFile);

  kkint32 height = toint32_t (bmh.biHeight);
  kkint32 width  = toint32_t (bmh.biWidth);

  kkuint32  bmpRowWidthInBytes = touint32_t (width);
  kkuint32  paddingBytes = bmpRowWidthInBytes % 4;
  if  (paddingBytes != 0)
    paddingBytes = 4 - paddingBytes;

  bmpRowWidthInBytes = bmpRowWidthInBytes + paddingBytes;

  std::fseek (inFile, hdr.bfOffBits, SEEK_SET);

  uchar*  rowData = new uchar[bmpRowWidthInBytes];

  for  (kkint32 row = height - 1;  row >= 0;  --row)
  {
    memset (rowData, 0, bmpRowWidthInBytes);
    size_t buffRead = std::fread (rowData, 1, bmpRowWidthInBytes, inFile);
    if  (buffRead < bmpRowWidthInBytes)
    {
      KKStr errMsg (128);
      errMsg << "BmpImage::Load8BitColor   ***ERROR***   Bytes read: " << buffRead << " less than expected: " << bmpRowWidthInBytes;
      cerr << endl << errMsg << endl << endl;  
      successfull = false;
      delete[]  rowData;  rowData = NULL;
      return;
    }

    kkint32  col;

    for  (col = 0;  col < width;  ++col)
    {
      kkint32  palletIdx = rowData[col];

      blue  [row][col] = palette[palletIdx].rgbBlue;
      image [row][col] = palette[palletIdx].rgbGreen;
      red   [row][col] = palette[palletIdx].rgbRed;
    }
  }

  successfull = true;

  delete[]  rowData;   rowData = NULL;

}  /* Load8BitColor */



void  BmpImage::Load4BitColorCompressed (FILE*  inFile,
                                         bool&  successfull
                                        )
{
  successfull = true;

  size_t  imageBuffSize = bmh.biSizeImage;

  uchar*  imageBuff = new uchar[imageBuffSize];

  size_t  buffRead = std::fread (imageBuff, sizeof (uchar), imageBuffSize, inFile);

  if  (buffRead < imageBuffSize)
  {
    KKStr errMsg (128);
    errMsg << "BmpImage::Load8BitColor   ***ERROR***   Bytes read: " << buffRead << " less than expected: " << imageBuffSize;
    cerr << endl << errMsg << endl << endl;  
    successfull = false;
    return;
  }

  kkint32  col = 0;
  kkint32  row = bmh.biHeight - 1;

  size_t  ifIDX = 0;

  while  (ifIDX < imageBuffSize)
  {
    if  (imageBuff[ifIDX] == 0)
    {
      // We have an Escape Sequence.
      ++ifIDX;

      if  (imageBuff[ifIDX] == 0)
      {
        // End of Row.
        --row;
        col = 0;
        ++ifIDX;
      }

      else if  (imageBuff[ifIDX] == 1)
      {
        // End of BitMap
        ifIDX = imageBuffSize;
      }

      else if  (imageBuff[ifIDX] == 2)
      {
        ++ifIDX;
        col = col + imageBuff[ifIDX];

        ++ifIDX;
        row = row + imageBuff[ifIDX];

        ++ifIDX;
      }

      else
      {
        // We have Absolute Mode
        kkint32  len = imageBuff[ifIDX];
        ++ifIDX;

        for  (kkint32 x = 0;  x < len;)
        {
          uchar  pix1 = imageBuff[ifIDX] / 16;
          uchar  pix2 = imageBuff[ifIDX] % 16;
          SetPixelValue (row, col, paletteMap[pix1]);
          ++x;
          ++col;

          if  (x < len)
          {
            SetPixelValue (row, col, paletteMap[pix2]);
            ++x;
            ++col;
          } 

          ++ifIDX;
        }

        if (((len + 1) / 2) % 2 != 0)
           ++ifIDX;
      }
    }

    else
    {
      // We have a RLE
      kkint32  len = imageBuff[ifIDX];
      ++ifIDX;

      uchar  pix1 = imageBuff[ifIDX] / 16;
      uchar  pix2 = imageBuff[ifIDX] % 16;

      for  (kkint32 x = 0;  x < len;)
      {
        SetPixelValue (row, col, paletteMap[pix1]);
        ++x;
        ++col;

        if  (x < len)
        {
          SetPixelValue (row, col, paletteMap[pix2]);
          ++x;
          ++col;
        }
      }

      ++ifIDX;
    }
  }

  delete[]  imageBuff;
}  /* Load4BitColorCompressed */



void  BmpImage::Load8BitColorCompressed (FILE*  inFile,
                                         bool&  successfull
                                        )
{
  successfull = true;

  kkuint32  imageBuffSize = bmh.biSizeImage;

  uchar*  imageBuff = new uchar[imageBuffSize];

  size_t  buffRead = std::fread (imageBuff, sizeof (uchar), imageBuffSize, inFile);

  if  (buffRead < imageBuffSize)
  {
    cerr << "***ERROR***, Load4BitColorCompressed   *** Error ***, Invalid File Format." << std::endl;
    successfull = false;
    return;
  }

  kkint32  col = 0;
  kkint32  row = bmh.biHeight - 1;

  kkuint32  ifIDX = 0;

  while  (ifIDX < imageBuffSize)
  {
    if  (imageBuff[ifIDX] == 0)
    {
      // We have an Escape Sequence.
      ++ifIDX;

      if  (imageBuff[ifIDX] == 0)
      {
        // End of Row.
        --row;
        col = 0;
        ++ifIDX;
      }

      else if  (imageBuff[ifIDX] == 1)
      {
        // End of BitMap

        ifIDX = imageBuffSize;
      }

      else if  (imageBuff[ifIDX] == 2)
      {
        ++ifIDX;
        col = col + imageBuff[ifIDX];

        ++ifIDX;
        row = row + imageBuff[ifIDX];

        ++ifIDX;
        // Insert Spaces
      }

      else
      {
        // We have Absolute Mode
        kkint32  len = imageBuff[ifIDX];
        ++ifIDX;

        for  (kkint32 x = 0;  x < len;)
        {
          SetPixelValue (row, col, paletteMap[imageBuff[ifIDX]]);
          ++x;
          ++col;
          ++ifIDX;
        }

        if  ((len % 2) != 0)
          ++ifIDX;
      }
    }

    else
    {
      // We have a RLE
      kkint32  len = imageBuff[ifIDX];
      ++ifIDX;

      kkint32  pixelVal = paletteMap[imageBuff[ifIDX]];

      if  (pixelVal == 0)
      {
        col = col + len;
      }
      else
      {
        for  (kkint32 x = 0;  x < len;)
        {
          SetPixelValue (row, col, pixelVal);
          ++x;
          ++col;
        }
      }

      ++ifIDX;
    }
  }

  delete[]  imageBuff;
} /* Load8BitColorCompressed */



struct BmpImage::BMP_24BitPixel
{
  uchar  blue;
  uchar  green;
  uchar  red;
};

  

void  BmpImage::Load24BitColor (FILE*  inFile,
                                bool&  successfull
                               )
{
  successfull = true;
  color = true;
  AllocateRaster ();

  kkuint32  bmpRowWidthInBytes = touint32_t (bmh.biWidth) * 3;
  kkuint32  paddingBytes = bmpRowWidthInBytes % 4;
  if  (paddingBytes != 0)
    paddingBytes = 4 - paddingBytes;

  bmpRowWidthInBytes = bmpRowWidthInBytes + paddingBytes;

  std::fseek (inFile, hdr.bfOffBits, SEEK_SET);

  uchar*  rowData = new uchar[bmpRowWidthInBytes];

  kkint32  row;

  for  (row = bmh.biHeight - 1; row >= 0; row--)
  {
    memset (rowData, 0, bmpRowWidthInBytes);
    size_t buffRead = std::fread (rowData, 1, bmpRowWidthInBytes, inFile);
    if  (buffRead < bmpRowWidthInBytes)
    {
      cerr << std::endl 
        << "***ERROR*** BmpImage::Load24BitColor, Error Reading File" << std::endl
        << std::endl;
      successfull = false;
      delete[]  rowData;  rowData = NULL;
      return;
    }

    for  (kkint32 col = 0;  col < bmh.biWidth;  ++col)
    {
      kkint32  offset = col * 3;
      blue  [row][col] = rowData[offset];
      image [row][col] = rowData[offset + 1];
      red   [row][col] = rowData[offset + 2];
    }
  }

  successfull = true;

  delete[]  rowData;   rowData = NULL;
}  /* Load24BitColor */



void  BmpImage::AllocateRaster ()
{
  CleanUpMemory ();

  if  (paletteEntries > 1000000)
  {
    cerr << std::endl << std::endl
      << "BmpImage::AllocateRaster   ***ERROR***    paletteEntries[" << paletteEntries << "]   Is a unreasonable value." << std::endl
      << std::endl;
    paletteEntries = 0;
  }

  if  (paletteEntries > 0)
    palette = new RGBQUAD[paletteEntries];

  image = new uchar*[bmh.biHeight];
  if  (color)
  {
    red  = new uchar*[bmh.biHeight];
    blue = new uchar*[bmh.biHeight];
  }

  for  (kkint32 row = 0;  row < bmh.biHeight;  ++row)
  {
    image[row] = new uchar[bmh.biWidth];
    memset (image[row], 0, tosize_t (bmh.biWidth));
    if  (color)
    {
      red[row]  = new uchar[bmh.biWidth];
      blue[row] = new uchar[bmh.biWidth];
      memset (red [row], 0, tosize_t (bmh.biWidth));
      memset (blue[row], 0, tosize_t (bmh.biWidth));
    }
  }
}  /* AllocateRaster */



void  BmpImage::CleanUpMemory ()
{
  if  (image)
  {
    for  (long x = bmh.biHeight - 1;  x >= 0;  --x)
    {
      delete  image[x];
      image[x] = NULL;
    }
    delete  image;
    image = NULL;
  }

  if  (red)
  {
    // If red exists  then  blue must exists.

    for  (long x = bmh.biHeight - 1;  x >= 0;  x--)
    {
      delete  red [x];  red [x] = NULL;
      delete  blue[x];  blue[x] = NULL;
    }

    delete  red;   red  = NULL;
    delete  blue;  blue = NULL;
  }

  delete  palette;  palette = NULL;
}  /* CleanUpMemory */



void  BmpImage::DownSize ()
{
  kkint32  newHeight = bmh.biHeight / 2;
  kkint32  newWidth  = bmh.biWidth  / 2;

  uchar**  newImage = new uchar*[newHeight];
  for  (kkint32 newRow = 0;  newRow < newHeight;  ++newRow)
  {
    kkint32 oldRow = newRow * 2;

    newImage[newRow] = new uchar[newWidth];
   
    for  (kkint32 newCol = 0;  newCol < newWidth;  ++newCol)
    {
      kkint32 oldCol = newCol * 2;
      newImage[newRow][newCol] = image[oldRow][oldCol];
    }
  }

  for  (kkint32 oldRow = 0;  oldRow < bmh.biHeight;  ++oldRow)
  {
    delete  image[oldRow];
    image[oldRow] = NULL;
  }

  delete  image;

  image = newImage;

  bmh.biHeight = newHeight;
  bmh.biWidth  = newWidth;
}  /* DownSize */



uchar&  BmpImage::Pixel (kkint32  row, 
                         kkint32  col)
{
  return  image[row][col];
}



void  BmpImage::SetPixelValue (kkint32  row, 
                               kkint32  col, 
                               kkint32  pixel
                              )
{
  if  (pixel < 0)
  {
    cerr << "BmpImage::SetPixelValue    ***ERROR***   pixel[" << pixel << "] out of range.  Needs to be in range of 0..255." << std::endl;
    pixel = 0;
  }
  else if  (pixel > 255)
  {
    cerr << "BmpImage::SetPixelValue    ***ERROR***   pixel[" << pixel << "] out of range.  Needs to be in range of 0..255." << std::endl;
    pixel = 255;
  }

  if  ((row < 0)  ||  (row >= bmh.biHeight))
  {
    cerr << "BmpImage::SetPixelValue  *** Error ***, Row[" << row 
         << "] out of range[0-" << bmh.biHeight << "]."
         << std::endl;
    return;
  }

  if  ((col < 0)  ||  (col >= bmh.biWidth))
  {
    cerr << "BmpImage::SetPixelValue  *** Error ***, Col[" << col 
         << "] out of range[0-" << bmh.biWidth << "]."
         << std::endl;
    return;
  }
  image[row][col] = touchar_t (pixel);
}  /* SetPixelValue */



bool  BmpImage::AreThereEdgePixels ()
{
  kkuint32  height = Height ();
  kkuint32  width  = Width ();

  if  (height < 6)
    return true;

  if  (width  < 6)
    return  true;
  
  uchar*  row0 = image[0];
  uchar*  row1 = image[1];
  uchar*  row2 = image[2];

  uchar*  rowL0 = image[height - 3];
  uchar*  rowL1 = image[height - 2];
  uchar*  rowL2 = image[height - 1];

  for  (kkuint32 col = 0;  col < width;  col++)
  {
    if  ((row0[col] > 0)   ||
         (row1[col] > 0)   ||
         (row2[col] > 0)   ||
         (rowL0[col] > 0)  ||
         (rowL1[col] > 0)  ||
         (rowL2[col] > 0)
        )
      return  true;
  }

  kkuint32  lastCol0 = width - 3;
  kkuint32  lastCol1 = width - 2;
  kkuint32  lastCol2 = width - 1;

  kkuint32  lastRowToCheck = height - 3;

  for  (kkuint32 row = 3;  row < lastRowToCheck;  row++)
  {
    if  ((image[row][0]        > 0)  ||
         (image[row][1]        > 0)  ||
         (image[row][2]        > 0)  ||
         (image[row][lastCol0] > 0)  ||
         (image[row][lastCol1] > 0)  ||
         (image[row][lastCol2] > 0)
        )
      return  true;
  }

  return  false;
}  /* EdgePixels */



void  BmpImage::EliminateVerticalLines ()
{
  bool* potVertLine = new bool[bmh.biWidth];
  bool* pvl = potVertLine;

  for  (long col = 0;  col < bmh.biWidth;  ++col)
  {
    *pvl = true;
    for  (kkint32 row = 0;  ((row < bmh.biHeight) && (*pvl));  ++row)
    {
      if  (image[row][col] == 7)
        *pvl = false;
    }
    
    ++pvl;
  }

  for (long col = 0;  col < bmh.biWidth;  ++col)
  {
    if  (potVertLine[col])  
    {
      long  firstCol = col;
      long  lastCol  = col;

      if  (col < (bmh.biWidth - 1))
      {
        while  ((col < bmh.biWidth)  &&  (potVertLine[col]))
        {
          ++col;
        }
      }

      if  (!potVertLine[col])
      {
        // We terminated loop because there were no more potentialVertLine's  not
        // because we ran out of columns.
        lastCol = col - 1;
      }

      //  Scan down vertically, and any place that we are not in contact both left and right
      //  with other pixels erase from picture,

      bool leftSideIsClear = true;
      bool rightSizeIsClear = true;

      uchar  leftPixel = 0;
      uchar  rightPixel = 0;

      for  (kkint32 row = 0; row < bmh.biHeight; row++)
      {
        if  (firstCol > 0)
        {
          leftPixel = image[row][firstCol - 1];
          if  (leftPixel != 0)
             leftSideIsClear = false;
        }
        else
        {
          leftPixel = 0;
        }
        
        if  (lastCol < (bmh.biWidth - 1))
        {
          rightPixel = image[row][lastCol + 1];

          if  (rightPixel != 0)
             rightSizeIsClear = false;
        }

        if  (leftSideIsClear &&  rightSizeIsClear)
        {
          for  (long x = firstCol; x <= lastCol; x++)
            image[row][x] = 0;
        }
        else
        {
          for  (long x = firstCol; x <= lastCol; x++)
            image[row][x] = touchar_t ((leftPixel + rightPixel) / 2);

          // Set up boolean for next loop around.
          leftSideIsClear = true;
          rightSizeIsClear = true;
        }
      }
    }
  }

  delete[]  potVertLine;
} /* EliminateVerticalLines */



void  BmpImage::Print ()
{
  for  (long x = 0;  x < bmh.biHeight;  ++x)
  {
    for  (long y = 0;  y < Min (static_cast<long> (bmh.biWidth), 76L);  ++y)
    {
      if  (image[x][y])
        cout << " ";
      else
        cout << "*";
    }

    cout << std::endl;
  }
}  /*  Print  */



void  BmpImage::Binarize ()
{
  for  (long row = 0;  row < bmh.biHeight;  ++row)
  {
    for  (long col = 0;  col < bmh.biWidth;  ++col)
    {
      if  (image[row][col] < 7)
        image[row][col] = 0;
    }
  }
}  /*  Binarize  */



void  BmpImage::SaveGrayscaleInverted4Bit (const KKStr&  _fileName)
{
  fileName = _fileName;
  FILE*  outFile = osFOPEN (fileName.Str (), "wb");
  if  (!outFile)
  {
    KKStr  errMsg = "BmpImage::SaveGrayscaleInverted4Bit,  Error opening BMP File[" + fileName + "].";
    cerr << errMsg << std::endl;
    throw KKException (errMsg);
  }

  CodedPixels pixelData (bmh.biHeight, bmh.biWidth);

  for  (long x = bmh.biHeight - 1;  x >= 0;  --x)
  {
    for  (long y = 0;  y < bmh.biWidth;  ++y)
    {
      if  (color)
      {
        kkint32  gsVal = toint32_t (tofloat (0.5f + red[x][y]) * 0.30f + 
                                    tofloat (image[x][y])      * 0.59f + 
                                    tofloat (blue[x][y])       * 0.11f);
        pixelData.AddPixel (touchar_t (gsVal >> 4));
      }
      else
      {
        pixelData.AddPixel (touchar_t (image[x][y] >> 4));
      }
    }

    pixelData.EOL ();
  }

  kkint32 imageBuffLen  = 0;
  uchar*  imageBuff     = NULL;

  numOfColors = 16;
  imageBuff = pixelData.CreatePixelDataStructure4Bit (imageBuffLen);
  bmh.biBitCount    = 4;
  bmh.biCompression = BI_RLE4; // BI_RGB;
  SetUp4BitPallet ();

  bmh.biSizeImage    = toDWORD (imageBuffLen);
  bmh.biClrUsed      = toDWORD (numOfColors);
  bmh.biClrImportant = toDWORD (numOfColors);

  hdr.bfSize    = 14 + 40 + toDWORD (paletteEntries) * 4 + bmh.biSizeImage;
  hdr.bfOffBits = 40 + 14 + toDWORD (paletteEntries) * 4;

  std::fwrite (&hdr,    sizeof (hdr), 1, outFile);
  std::fwrite (&bmh,    sizeof (bmh), 1, outFile);
  std::fwrite (palette, sizeof (RGBQUAD), tosize_t (paletteEntries), outFile);
  std::fwrite (imageBuff, 1, tosize_t (imageBuffLen), outFile);

  delete  imageBuff;
  std::fclose (outFile);
}  /* SaveGrayscaleInverted4Bit */



void  BmpImage::SaveGrayscaleInverted8Bit (const KKStr&  _fileName)
{
  fileName = _fileName;
  FILE*  outFile = osFOPEN (fileName.Str (), "wb");
  if  (!outFile)
  {
    KKStr  errMsg = "BmpImage::SaveGrayscaleInverted8Bit,  Error opening BMP File[" + fileName + "].";
    cerr << errMsg << std::endl;
    throw KKException (errMsg);
  }

  CodedPixels pixelData (bmh.biHeight, bmh.biWidth);

  for  (long x = bmh.biHeight - 1;  x >= 0;  --x)
  {
    for  (long y = 0;  y < bmh.biWidth;  ++y)
    {
      if  (color)
      {
        kkint32 gsVal = toint32_t (tofloat (0.5f + red[x][y]) * 0.30f + 
                                   tofloat (image[x][y]) * 0.59f + 
                                   tofloat (blue[x][y]) * 0.11f);
        pixelData.AddPixel (touchar_t (gsVal));
      }
      else
      {
        pixelData.AddPixel (image[x][y]);
      }
    }

    pixelData.EOL ();
  }

  kkint32 imageBuffLen = 0;
  uchar*  imageBuff    = NULL;

  numOfColors = 256;
  imageBuff = pixelData.CreatePixelDataStructure8Bit (imageBuffLen);
  bmh.biBitCount    = 8;
  bmh.biCompression = BI_RLE8; // BI_RGB;
  SetUp8BitPallet ();

  bmh.biSizeImage    = toDWORD (imageBuffLen); 
  bmh.biClrUsed      = toDWORD (numOfColors);
  bmh.biClrImportant = toDWORD (numOfColors);

  hdr.bfSize    = 14 + 40 + toDWORD (paletteEntries) * 4 + bmh.biSizeImage;
  hdr.bfOffBits = 40 + 14 + toDWORD (paletteEntries) * 4;

  std::fwrite (&hdr,    sizeof (hdr), 1, outFile);
  std::fwrite (&bmh,    sizeof (bmh), 1, outFile);
  std::fwrite (palette, sizeof (RGBQUAD), tosize_t (paletteEntries), outFile);
  std::fwrite (imageBuff, 1, tosize_t (imageBuffLen), outFile);

  delete  imageBuff;
  std::fclose (outFile);
}  /* SaveGrayscaleInverted8Bit */



void  BmpImage::Save (const KKStr&  _fileName)
{
  fileName = _fileName;
  std::FILE*  outFile = osFOPEN (fileName.Str (), "wb");
  if  (!outFile)
  {
    KKStr  errMsg = "BmpImage::Save,  Error opening BMP File[" + fileName + "].";
    cerr << errMsg << std::endl;
    throw KKException (errMsg);
  }

  if  (color)
  {
    SaveColor (outFile);
  }
  else
  {
    SaveGrayScale (outFile);
  }

  std::fclose (outFile);
}  /* Save */



void  BmpImage::SaveGrayScale (FILE*  outFile)
{
  CodedPixels pixelData (bmh.biHeight, bmh.biWidth);

  for  (long x = bmh.biHeight - 1;  x >= 0;  --x)
  {
    for  (long y = 0;  y < bmh.biWidth;  ++y)
    {
      pixelData.AddPixel (image[x][y]);
    }

    pixelData.EOL ();
  }

  kkint32 imageBuffLen;
  uchar*  imageBuff;

  numOfColors = 256;
  imageBuff = pixelData.CreatePixelDataStructure8Bit (imageBuffLen);
  bmh.biCompression  = BI_RLE8;
  bmh.biBitCount     = 8;

  bmh.biSizeImage    = toDWORD (imageBuffLen); 
  bmh.biClrUsed      = toDWORD (numOfColors);
  bmh.biClrImportant = toDWORD (numOfColors);
  paletteEntries     = numOfColors;

  hdr.bfSize = 14 + 40 + toDWORD (paletteEntries) * 4 + bmh.biSizeImage;
  hdr.bfOffBits = 40 + 14 + toDWORD (paletteEntries) * 4;

  #ifndef  WIN32
  WriteWORD  (outFile, hdr.bfType);
  WriteDWORD (outFile, hdr.bfSize);
  WriteWORD  (outFile, hdr.bfReserved1);
  WriteWORD  (outFile, hdr.bfReserved2);
  WriteDWORD (outFile, hdr.bfOffBits);
  #else

  std::fwrite (&hdr,   sizeof (hdr), 1, outFile);
  #endif

  #ifndef  WIN32
  WriteDWORD (outFile, bmh.biSize);
  WriteLONG  (outFile, bmh.biWidth);
  WriteLONG  (outFile, bmh.biHeight);
  WriteWORD  (outFile, bmh.biPlanes);
  WriteWORD  (outFile, bmh.biBitCount);
  WriteDWORD (outFile, bmh.biCompression); 
  WriteDWORD (outFile, bmh.biSizeImage);
  WriteLONG  (outFile, bmh.biXPelsPerMeter);
  WriteLONG  (outFile, bmh.biYPelsPerMeter);
  WriteDWORD (outFile, bmh.biClrUsed);
  WriteDWORD (outFile, bmh.biClrImportant);
  #else

  fwrite (&bmh,   sizeof (bmh), 1, outFile);
  #endif


  #ifndef  WIN32
  for  (kkint32 x = 0; x < paletteEntries; x++)
  {
    WriteBYTE (outFile, palette[x].rgbBlue);
    WriteBYTE (outFile, palette[x].rgbGreen);
    WriteBYTE (outFile, palette[x].rgbRed);
    WriteBYTE (outFile, palette[x].rgbReserved);
  }
  #else
  fwrite (palette, sizeof (RGBQUAD), tosize_t (paletteEntries), outFile);
  #endif

  std::fwrite (imageBuff, 1, tosize_t (imageBuffLen), outFile);
  delete  imageBuff;
}  /* SaveGrayScale */



void  BmpImage::SaveColor (FILE*  outFile)
{
  /** @todo  Need to finish implementing and testing compressed color.  */
  PalletBuilderPtr  palletBuilder = BuildPalletFromRasterData ();
  if  (palletBuilder->NumOfColors () <= 256)
    SaveColorCompressed256 (palletBuilder, outFile);
  else 
    SaveColor24BPP (outFile);
  delete  palletBuilder;
  palletBuilder = NULL;


  //SaveColor24BPP (outFile);
}  /* SaveColor */



/** @brief  Will write a color compressed BMP file with a maximum of 256 colors. */
void  BmpImage::SaveColorCompressed256 (PalletBuilderPtr  palletBuilder,
                                        FILE*             outFile
                                       )
{
  uchar*  redRow   = NULL;
  uchar*  greenRow = NULL;
  uchar*  blueRow  = NULL;

  kkint32  palletIdx = 0;

  CodedPixels  pixelData (bmh.biHeight, bmh.biWidth);

  for  (LONG x = bmh.biHeight - 1;  x >= 0; --x)
  {
    redRow   = red[x];
    greenRow = image[x];
    blueRow  = blue[x];
    for  (LONG y = 0;  y < bmh.biWidth;  ++y)
    {
      palletIdx = palletBuilder->PalletIndex (redRow[y], greenRow[y], blueRow[y]);

      if  (palletIdx < 0)
        palletIdx = 0;

      else if  (palletIdx > 255)
        palletIdx = 255;

      pixelData.AddPixel (touchar_t (palletIdx));
    }

    pixelData.EOL ();
  }

  kkint32 imageBuffLen;
  uchar*  imageBuff;

  numOfColors = paletteEntries;
  imageBuff = pixelData.CreatePixelDataStructure8Bit (imageBuffLen);
  bmh.biCompression  = BI_RLE8;  /* 'BI_RLE8' is where each pixel is represented by a 8 bit number that indexes into a color palette. */
  bmh.biBitCount     = 8;

  bmh.biSizeImage    = toDWORD (imageBuffLen);
  bmh.biClrUsed      = toDWORD (numOfColors);
  bmh.biClrImportant = toDWORD (numOfColors);

  hdr.bfSize = 14 + 40 + toDWORD (paletteEntries) * 4 + bmh.biSizeImage;
  hdr.bfOffBits = 40 + 14 + toDWORD (paletteEntries) * 4;

  #ifndef  WIN32
  WriteWORD  (outFile, hdr.bfType);
  WriteDWORD (outFile, hdr.bfSize);
  WriteWORD  (outFile, hdr.bfReserved1);
  WriteWORD  (outFile, hdr.bfReserved2);
  WriteDWORD (outFile, hdr.bfOffBits);
  #else

  fwrite (&hdr, sizeof (hdr), 1, outFile);
  #endif


  #ifndef  WIN32
  WriteDWORD (outFile, bmh.biSize);
  WriteLONG  (outFile, bmh.biWidth);
  WriteLONG  (outFile, bmh.biHeight);
  WriteWORD  (outFile, bmh.biPlanes);
  WriteWORD  (outFile, bmh.biBitCount);
  WriteDWORD (outFile, bmh.biCompression); 
  WriteDWORD (outFile, bmh.biSizeImage);
  WriteLONG  (outFile, bmh.biXPelsPerMeter);
  WriteLONG  (outFile, bmh.biYPelsPerMeter);
  WriteDWORD (outFile, bmh.biClrUsed);
  WriteDWORD (outFile, bmh.biClrImportant);
  #else

  fwrite (&bmh, sizeof (bmh), 1, outFile);
  #endif

  #ifndef  WIN32
  for  (kkint32 x = 0; x < paletteEntries; ++x)
  {
    WriteBYTE (outFile, palette[x].rgbBlue);
    WriteBYTE (outFile, palette[x].rgbGreen);
    WriteBYTE (outFile, palette[x].rgbRed);
    WriteBYTE (outFile, palette[x].rgbReserved);
  }
  #else
  fwrite (palette, sizeof (RGBQUAD), paletteEntries, outFile);
  #endif

  std::fwrite (imageBuff, 1, tosize_t (imageBuffLen), outFile);
  delete  imageBuff;
}  /* SaveColorCompressed256 */



void  BmpImage::SaveColor24BPP (FILE*  outFile)
{
  kkuint32  bytesPerRow = Width () * 3;
  kkuint32  bufferPerRow = 0;
  while  ((bytesPerRow % 4) != 0)
  {
    bufferPerRow++;
    bytesPerRow++;
  }

  bmh.biSize    = 40;
  //bmh.biWidth = Width ();
  //bmh.biHeight  = height ();
  bmh.biPlanes = 1;
  bmh.biBitCount = 24;
  bmh.biCompression =  BI_RGB;
  bmh.biSizeImage = Height () * bytesPerRow;
  bmh.biXPelsPerMeter   = 2835;
  bmh.biYPelsPerMeter   = 2835;
  bmh.biClrUsed         = 0;
  bmh.biClrImportant    = 0;

  hdr.bfSize = 14 + 40 + 0 * 4 + bmh.biSizeImage;
  hdr.bfOffBits = 40 + 14 + 0 * 4;

  #ifndef  WIN32
  WriteWORD  (outFile, hdr.bfType);
  WriteDWORD (outFile, hdr.bfSize);
  WriteWORD  (outFile, hdr.bfReserved1);
  WriteWORD  (outFile, hdr.bfReserved2);
  WriteDWORD (outFile, hdr.bfOffBits);
  #else
  std::fwrite (&hdr,   sizeof (hdr), 1, outFile);
  #endif


  #ifndef  WIN32
  WriteDWORD (outFile, bmh.biSize);
  WriteLONG  (outFile, bmh.biWidth);
  WriteLONG  (outFile, bmh.biHeight);
  WriteWORD  (outFile, bmh.biPlanes);
  WriteWORD  (outFile, bmh.biBitCount);
  WriteDWORD (outFile, bmh.biCompression); 
  WriteDWORD (outFile, bmh.biSizeImage);
  WriteLONG  (outFile, bmh.biXPelsPerMeter);
  WriteLONG  (outFile, bmh.biYPelsPerMeter);
  WriteDWORD (outFile, bmh.biClrUsed);
  WriteDWORD (outFile, bmh.biClrImportant);
  #else

  std::fwrite (&bmh,   sizeof (bmh), 1, outFile);
  #endif

  
  //for  (kkuint32 row = Height () - 1;  row >= 0;  --row)
  
  for (kkuint32 row = Height ();;)
  {
    if  (row == 0)
       break;
    --row;

    for  (kkuint32 col = 0;  col < Width ();  ++col)
    {
      std::fwrite (&(blue [row][col]), 1, 1, outFile);
      std::fwrite (&(image[row][col]), 1, 1, outFile);
      std::fwrite (&(red  [row][col]), 1, 1, outFile);
    }

    uchar  pad = 0;
    for  (kkuint32 x = 0;  x < bufferPerRow;  ++x)
      std::fwrite (&pad, 1, 1, outFile);
  }
}  /* SaveColor24BPP */



void  BmpImage::AddPixel (kkuint32 row, 
                          kkuint32 col, 
                          uchar  pixValue
                         )
{
  image[row][col] = pixValue;
}



void BmpImage::ClearImage()
{
	for  (LONG row = 0;  row < bmh.biHeight;  ++row)
	{
		memset (image[row], 0, tosize_t (bmh.biWidth));
	}
}



const   
uchar* BmpImage::BlueRow (kkint32 row)  const
{
  if  (blue == NULL)
    throw KKException("BmpImage::BlueRow  'blue' channel is set to NULL.");
  if  ((row < 0)  ||  (row >= toint32_t (Height ())))
  {
    KKStr errMsg = "BmpImage::BlueRow   *** ERROR ***  Invalid Row: " + StrFromInt32 (row);
    cerr << '\n' << errMsg << "\n\n";
    throw KKException(errMsg);
  }

  return  blue[row];
}  /* BlueRow */



const   
uchar* BmpImage::ImageRow (kkint32 row)  const
{
  if  (image == NULL)
    throw KKException("::ImageRow  'image' set to NULL.");
  if  ((row < 0)  ||  (row >= toint32_t (Height ())))
  {
    KKStr errMsg = "BmpImage::ImageRow  row: " + StrFromInt32 (row) + " is out of range where Height: " + StrFromInt32 (row);
    cerr << '\n' << errMsg << "\n\n";
    throw KKException (errMsg);
  }

  return  image[row];
}  /* ImageRow */



const   
uchar* BmpImage::RedRow (kkint32 row)  const
{
  if (blue == NULL)
    throw KKException("BmpImage::RedRow  'red' channel is set to NULL.");
  if ((row < 0) || (row >= toint32_t (Height())))
  {
    KKStr errMsg = "BmpImage::BlueRed  row:" + StrFromInt32 (row) + " is out of range where Height:" + StrFromInt32 (row);
    cerr << '\n' << errMsg << "\n\n";
    throw KKException(errMsg);
  }

  return  red[row];
}  /* RedRow */



bool  BmpImage::FourBitUncompressed () 
{
  return  ((bmh.biBitCount == 4)  &&  (bmh.biCompression == BI_RGB));
}
