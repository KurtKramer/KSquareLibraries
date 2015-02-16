/* BMPImage.cpp -- Manages the reading and writing of BMP image files.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
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
#endif


#include "BMPImage.h"
#include "KKException.h"
#include "OSservices.h"
#include "Raster.h"
using namespace KKU;


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
  BmpImage::WordParts& wp = (BmpImage::WordParts&)w;

  BmpImage::WordParts  temp = wp;

  wp.parts.c1 = temp.parts.c2;
  wp.parts.c2 = temp.parts.c1;
}  /* RotateWORD */




void  RotateDWORD (DWORD&  w)
{
  BmpImage::DWordParts& wp = (BmpImage::DWordParts&)w;

  BmpImage::DWordParts  temp = wp;

  wp.parts.c1 = temp.parts.c4;
  wp.parts.c2 = temp.parts.c3;
  wp.parts.c3 = temp.parts.c2;
  wp.parts.c4 = temp.parts.c1;
}  /* RotateDWORD */




void  RotateLONG (LONG&  l)
{
  BmpImage::LongParts& lp = (BmpImage::LongParts&)l;

  BmpImage::LongParts  temp = lp;

  lp.parts.c1 = temp.parts.c4;
  lp.parts.c2 = temp.parts.c3;
  lp.parts.c3 = temp.parts.c2;
  lp.parts.c4 = temp.parts.c1;
}  /* RotateLONG */


void  WriteWORD (FILE*  outFile, WORD w)
{
  RotateWORD (w);
  fwrite (&w,  sizeof (w), 1, outFile);
}


void  WriteDWORD (FILE*  outFile,  DWORD dw)
{
  RotateDWORD (dw);
  fwrite (&dw,  sizeof (dw), 1, outFile);
}



void  WriteLONG (FILE*  outFile, LONG l)
{
  RotateLONG (l);
  fwrite (&l,  sizeof (l), 1, outFile);
}


void  WriteBYTE (FILE*  outFile, BYTE b)
{
  fwrite (&b,  sizeof (b), 1, outFile);
}


#endif




struct  BmpImage::CodePair
{
  uchar  pixel;
  uchar  count;
};




/*!
 \class  BmpImage::CodedPixels
 \brief  This object is used to help encode the data stored in BMPImage::image into 8 or 4 bit compressed formats used by BMP files.                                            *
 */
class  BmpImage::CodedPixels 
{
public:
  CodedPixels (int32  _height,
               int32  _width
              );
  ~CodedPixels ();

  void  AddPixel (uchar  pixel);  /*!< This method is called once for each pixel in the BMP image */
                                              
  void  EOL ();                   /*!< This is called once at the end of each row of pixels.      */

  uchar*  CreatePixelDataStructure4Bit (int32&  len);  /**< Creates the appropriate compressed data structure for the BMP 4-bit
                                                        * compressed file format.  The length of the structure created will be
                                                        * returned in the 'len' parameter.   The caller will be responsible for
                                                        * deleting the returned data structure.
                                                        */

  uchar*  CreatePixelDataStructure8Bit (int32&  len);  /**< Creates the appropriate compressed data structure for the BMP 8-bit
                                                        * compressed file format.  The length of the structure created will be
                                                        * returned in the 'len' parameter.     The caller will be responsible for
                                                        * deleting the returned data structure.
                                                        */

private:
  int32  height;
  int32  width;
  
  int32  codesAllocated;
  int32  used;

  CodePairPtr codes;
  CodePairPtr top;
};



BmpImage::CodedPixels::CodedPixels (int32 _height,
                                    int32 _width
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




uchar*  BmpImage::CodedPixels::CreatePixelDataStructure4Bit (int32&  len)
{
  int32  buffSize = codesAllocated;

  uchar* buff = new uchar [buffSize + 4];  // + 4 For Safety
  memset (buff, 0, codesAllocated);


  int32  curPair = 0;
  top = &(codes[0]);

  int32  bp = 0;

  while  (curPair < used)
  {
    if  (bp >= buffSize)
    {
      int32  newBuffSize = buffSize * 2;
      uchar* newBuff = new uchar[newBuffSize];
      memset (newBuff, 0, newBuffSize);
      memcpy (newBuff, buff, buffSize);
      delete buff;
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

          buff[bp] = top->pixel * 16 + top->pixel;
          bp++;

          top->count = top->count - 254;
        }

        else
        {
          buff[bp] = top->count;
          bp++;

          buff[bp] = top->pixel * 16 + top->pixel;
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



uchar*  BmpImage::CodedPixels::CreatePixelDataStructure8Bit (int32&  len)
{
  int32  buffSize = codesAllocated;

  uchar* buff = new uchar [buffSize + 4];  // + 4 For Safty
  memset (buff, 0, codesAllocated);

  int32  curPair = 0;
  top = &(codes[0]);

  int32  bp = 0;

  while  (curPair < used)
  {
    if  (bp >= (buffSize - 1))
    {
      int32  newBuffSize = buffSize * 2;
      uchar* newBuff = new uchar[newBuffSize + 2];
      if  (newBuff == NULL)
      {
        KKStr  errMsg = "BmpImage::CodedPixels::CreatePixelDataStructure8Bit    ***ERROR***     Allocation of 'newBuff'  failed.";
        cerr << std::endl << std::endl << errMsg << std::endl << std::endl;
        throw  KKException (errMsg);
      }

      memset (newBuff, 0, newBuffSize);
      memcpy (newBuff, buff, buffSize);
      delete buff;
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

          top->count = top->count - 254;
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

  color         (false),
  fileName      (_fileName),
  red           (NULL),
  image         (NULL),
  blue          (NULL),
  numOfColors   (16),
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

  size_t   x;
  int32    y;

  x = fread (&hdr, sizeof (hdr), 1, inFile);
  if  (x <= 0)
  {
    successfull = false;
    fclose (inFile);
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
    fclose (inFile);
    return;
  }
  else
  {
    cerr << std::endl 
         << "File[" << _fileName << "]  is of a unknown file format." << std::endl
         << std::endl;
    successfull = false;
    fclose (inFile);
    return;
  }


  x = fread (&bmh, sizeof (bmh), 1, inFile);
  if  (x <= 0)
  {
    successfull = false;
    fclose (inFile);
    return;
  }

  if  ((bmh.biCompression == BI_RGB)  &&  (bmh.biBitCount == 24))
  {
    numOfColors = 16777216;
    color = true;
    delete  palette;
    palette = NULL;
    Load24BitColor (inFile, successfull);
    fclose (inFile);
    return;
  }

  if  ((bmh.biCompression == BI_RGB)  &&  (bmh.biBitCount == 8))
  {
    Load8BitColor (inFile, successfull);
    fclose (inFile);
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
    fclose (inFile);
    return;
  }

  bool  imageIsRevGrayscale = false;
  if  (paletteEntries)
  {
    palette =  new RGBQUAD[paletteEntries];
    x = fread (palette, sizeof (RGBQUAD), paletteEntries, inFile);
    imageIsRevGrayscale = ReversedGrayscaleImage ();
  }
  else
  {
    numOfColors   = 256;
    paletteEntries = 256;
    palette = new RGBQUAD[paletteEntries];
    SetUp256BitPalette (palette);
  }

  // Lets Build Pallet Map
  for  (x = 0; x < 256; x++)
    paletteMap[x] = 0;

  if  (numOfColors == 16)
  {
    for  (int32 palletIdx = 0; palletIdx < paletteEntries; palletIdx++)
    {
      // 2011-03-09  KAK
      // In the SIPPER world we always reversed the image when saving and re-reading
      // because  255 was background and '0' was foreground.
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
    for  (int32 palletIdx = 0; palletIdx < paletteEntries; palletIdx++)
    {
      // 2011-03-09  KAK
      // In the SIPPER world we always reversed the image when saving and re-reading
      // because  255 was background and '0' was foreground.
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

  int32  row;

  image = new uchar*[bmh.biHeight];
  for  (row = 0; row < bmh.biHeight; row++)
  {
    image[row] = new uchar[bmh.biWidth];
    memset (image[row], 0, bmh.biWidth);
  }

  x = fseek (inFile, hdr.bfOffBits, SEEK_SET);

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

  fclose (inFile);
}




BmpImage::BmpImage (int32  _height,
                    int32  _width,
                    int32  _numOfColors
                   ):
   
   color       (false),
   fileName    (),
   red         (NULL),
   image       (NULL),
   blue        (NULL),
   numOfColors (_numOfColors),
   palette      (NULL)
{
  InitializeFields (_height, _width);
}




BmpImage::BmpImage (const Raster&  raster):
  color  (false),
  red    (NULL),
  image  (NULL),
  blue   (NULL),
  palette (NULL)
{
  numOfColors = 256;  // kk 2005-03-10

  color = raster.Color ();

  InitializeFields (raster.Height (), raster.Width ());
  fileName = "";

  int32  row;
  int32  col;

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
 */
bool  BmpImage::ReversedGrayscaleImage ()
{
  int32 x = 0;
  bool  allChannelsSameValue = true;
  bool  alwaysDecending      = true;   // Reversed images pixel values will always be decending.
  int32  firstUnbalancedChannel = 0;

  for  (x = 0;  (x < paletteEntries)  &&  allChannelsSameValue;  ++x)
  {
    RGBQUAD&   p = (palette[x]);
    allChannelsSameValue = (p.rgbBlue == p.rgbGreen)  &&  (p.rgbBlue == p.rgbRed)  &&  (p.rgbGreen == p.rgbRed);
    firstUnbalancedChannel = x;
    if  ((x > 0)  &&  (p.rgbBlue >= palette[x - 1].rgbBlue))
      alwaysDecending = false;
  }

  if  ((x == 254)  &&  (palette[253].rgbGreen == 255)  &&  (palette[254].rgbRed == 255))
  {
    // SIPPER generated images use pixel values 253 and 254 to flag sipper file over and under runs on a scan file.
    allChannelsSameValue = true;
    alwaysDecending = true;
  }

     

  if  (allChannelsSameValue  && alwaysDecending  &&  (palette[0].rgbBlue == 255)  &&  (palette[paletteEntries - 1].rgbBlue == 0))
    return true;

  if  (firstUnbalancedChannel == 8)
  {
    // See if this is a SIPPER BMP.
    if  ((palette[ 0].rgbBlue  == 255)  &&
         (palette[ 1].rgbBlue  == 219)  &&
         (palette[ 2].rgbBlue  == 182)  &&
         (palette[ 3].rgbBlue  == 146)  &&
         (palette[ 4].rgbBlue  == 109)  &&
         (palette[ 5].rgbBlue  ==  73)  &&
         (palette[ 6].rgbBlue  ==  36)  &&
         (palette[ 7].rgbBlue  ==   0)  &&
         (palette[ 8].rgbBlue  == 255)  &&  (palette[ 8].rgbGreen ==  40)  &&  (palette[ 8].rgbRed   ==  40)  &&
         (palette[ 9].rgbBlue  ==  40)  &&  (palette[ 9].rgbGreen ==  40)  &&  (palette[ 9].rgbRed   == 255)  &&
         (palette[10].rgbBlue  ==  40)  &&  (palette[10].rgbGreen == 210)  &&  (palette[10].rgbRed   ==  40)  &&
         (palette[11].rgbBlue  ==  40)  &&  (palette[11].rgbGreen == 210)  &&  (palette[11].rgbRed   == 210)  &&
         (palette[12].rgbBlue  == 210)  &&  (palette[12].rgbGreen ==  40)  &&  (palette[12].rgbRed   ==  40)  &&
         (palette[13].rgbBlue  == 210)  &&  (palette[13].rgbGreen ==  40)  &&  (palette[13].rgbRed   == 210)  &&
         (palette[14].rgbBlue  == 210)  &&  (palette[14].rgbGreen == 210)  &&  (palette[14].rgbRed   ==  40)  &&
         (palette[15].rgbBlue  == 210)  &&  (palette[15].rgbGreen == 210)  &&  (palette[15].rgbRed   == 210)
        )
    {
      // This is a Sipper Image.
      return true;
    }
  }

  return false;
}  /* ReversedGrayscaleImage */






int32  BmpImage::BMIcolorArraySize (BITMAPINFOHEADER&  _bmh)

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
      return  _bmh.biClrUsed;
  }

  else if  (_bmh.biBitCount == 16)
  {
    if  (_bmh.biCompression ==  BI_RGB)
      return  0;
  }

  else if  (_bmh.biBitCount == 24)
  {
    return  _bmh.biClrUsed;
  }

  return  -1;
}




void  BmpImage::SetUp4BitPallet ()
{
  delete  palette;   palette = NULL;
  paletteEntries = 16;
  palette = new RGBQUAD[paletteEntries];

  uchar  pixelVal = 255;
  for  (int32 x = 0;  x < 16;  x++)
  {
    palette[x].rgbBlue   = pixelVal;
    palette[x].rgbGreen  = pixelVal;
    palette[x].rgbRed    = pixelVal;
    pixelVal = pixelVal - 17;
  }
}  /* SetUp4BitPallet*/





void  BmpImage::SetUp8BitPallet ()
{
  delete  palette;   palette = NULL;
  paletteEntries = 256;
  palette = new RGBQUAD[paletteEntries];

  uchar  pixelVal = 255;
  for  (int32 x = 0;  x < 256;  x++)
  {
    palette[x].rgbBlue  = pixelVal;
    palette[x].rgbGreen = pixelVal;
    palette[x].rgbRed   = pixelVal;
    --pixelVal;
  }
}  /* SetUp8BitPallet */




void  BmpImage::SetUp16BitPallet (RGBQUAD*  palette)
{
  palette[0].rgbBlue  = 255;
  palette[0].rgbGreen = 255;
  palette[0].rgbRed   = 255;

  palette[1].rgbBlue  = 219;
  palette[1].rgbGreen = 219;
  palette[1].rgbRed   = 219;

  palette[2].rgbBlue  = 182;
  palette[2].rgbGreen = 182;
  palette[2].rgbRed   = 182;

  palette[3].rgbBlue  = 146;
  palette[3].rgbGreen = 146;
  palette[3].rgbRed   = 146;

  palette[4].rgbBlue  = 109;
  palette[4].rgbGreen = 109;
  palette[4].rgbRed   = 109;

  palette[5].rgbBlue  = 73;
  palette[5].rgbGreen = 73;
  palette[5].rgbRed   = 73;

  palette[6].rgbBlue  = 36;
  palette[6].rgbGreen = 36;
  palette[6].rgbRed   = 36;

  palette[7].rgbBlue  = 0;
  palette[7].rgbGreen = 0;
  palette[7].rgbRed   = 0;

  palette[8].rgbBlue  = 255;
  palette[8].rgbGreen = 40;
  palette[8].rgbRed   = 40;

  palette[9].rgbBlue  = 40;
  palette[9].rgbGreen = 40;
  palette[9].rgbRed   = 255;

  palette[10].rgbBlue  = 40;
  palette[10].rgbGreen = 210;
  palette[10].rgbRed   = 40;

  palette[11].rgbBlue  = 40;
  palette[11].rgbGreen = 210;
  palette[11].rgbRed   = 210;

  palette[12].rgbBlue  = 210;
  palette[12].rgbGreen = 40;
  palette[12].rgbRed   = 40;

  palette[13].rgbBlue  = 210;
  palette[13].rgbGreen = 40;
  palette[13].rgbRed   = 210;

  palette[14].rgbBlue  = 210;
  palette[14].rgbGreen = 210;
  palette[14].rgbRed   = 40;

  palette[15].rgbBlue  = 210;
  palette[15].rgbGreen = 210;
  palette[15].rgbRed   = 210;
}  /* SetUp16BitPallet */





void  BmpImage::SetPaletteEntry (int32              palletIndex,
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
 




void  BmpImage::SetUp256BitPalette (RGBQUAD*  palette)
{
  uchar  pixVal;

  int32  x;

  for  (x = 0; x < 256; x++)
  {
    // 2011-03-09  KAK
    // In the SIPPER world we always reversed the image when saving and re-reading
    // because  255 was background and '0' was foreground.
    //y = 255 - palette[palletIdx].rgbGreen;
    //pixVal = 255 - x;    
    pixVal = (uchar)x;    
    palette[x].rgbBlue  = pixVal;
    palette[x].rgbGreen = pixVal;
    palette[x].rgbRed   = pixVal;
  }


//  // Did the next several lines as a special case, to help detect 
//  // Underflow, and overflow pixels in the sipper file.
//  palette[253].rgbBlue  = 0;     //  Flag for a scan-line overflow
//  palette[253].rgbGreen = 255;
//  palette[253].rgbRed   = 0;

//  palette[254].rgbBlue  = 0;     //  Flag for a scan-line underflow
//  palette[254].rgbGreen = 0;
//  palette[254].rgbRed   = 255;
}  /* SetUp256BitPalette */




/**
 *@brief  Returns true if palette is for a grayscale image.
 */
bool  GrayScaleImage (RGBQUAD*  palette,
                      int32     palletSize
                     )
{
  bool  isGrayScaleImage = true;

  for  (int32 x = 0;  ((x < palletSize)  &&  isGrayScaleImage);  x++)
  {
    int32  expectedColor = 255 - x;
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



void  BmpImage::InitializeFields (int32  _height,
                                  int32  _width
                                 )
{
  hdr.bfType            = 19778;
  hdr.bfReserved1       = 0;
  hdr.bfReserved2       = 0;
  // hdr.bfOffBits         = sizeof (bmh


  bmh.biSize            = sizeof (bmh);
  bmh.biWidth           = _width;
  bmh.biHeight          = _height;
  bmh.biPlanes          = 1;

  paletteEntries = 0;

  AllocateRaster ();

  if  (numOfColors <= 16)
  {
    delete  palette;  palette = NULL;
    bmh.biBitCount    = 4;
    paletteEntries     = 16;
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

  // BITMAPFILEHEADER  hdr;
  // BITMAPINFOHEADER  bmh;

  hdr.bfOffBits  = sizeof (bmh) + sizeof (RGBQUAD) * paletteEntries;
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


  PalletBuilder ()
  {
  }

  int32  NumOfColors ()  const  {return colorsUsed.size ();}

  int32  PalletIndex (uchar red,
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
    RGBQUAD  key;
    key.rgbRed   = red;
    key.rgbGreen = green;
    key.rgbBlue  = blue;
    idx = colorsUsed.find (key);
    if  (idx == colorsUsed.end ())
    {
      int32  numEntries = colorsUsed.size ();
      colorsUsed.insert (pair<RGBQUAD,int32> (key, numEntries));
    }
  }


  void  BuildPallet (RGBQUAD*&  palette,
                     int32&       size
                    )
  {
    size = colorsUsed.size ();
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
  map<RGBQUAD, int32, RGBQUAD_Pred>            colorsUsed;
  map<RGBQUAD, int32, RGBQUAD_Pred>::iterator  idx;
};  /* PalletBuilder */






BmpImage::PalletBuilderPtr  BmpImage::BuildPalletFromRasterData ()
{
  PalletBuilderPtr  palletBuilder = new PalletBuilder ();

  int32  row, col;
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

  uint32  bmpRowWidthInBytes = bmh.biWidth / 8;

  while  ((bmpRowWidthInBytes % 2) > 0)
  {
    bmpRowWidthInBytes++;
  }

  Bmp1BitRec*  packedRowData = new Bmp1BitRec [bmpRowWidthInBytes];
  uchar*       rowData       = new uchar [bmpRowWidthInBytes * 8];

  Bmp1BitRec  b;
  uint32      byteNum;
  int32       col;
  int32       row;
  size_t      x;

  for  (row = bmh.biHeight - 1; row >= 0; row--)
  {
    x = fread (packedRowData, 1, bmpRowWidthInBytes, inFile);
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
      x = paletteMap[rowData[col]];
      SetPixelValue (row, col, paletteMap[rowData[col]]);
    }
  }

  ReAllocateForBiggerScreen ();

  delete  rowData;
  delete  packedRowData;
}  /* Load1BitColor */




void  BmpImage::ReAllocateForBiggerScreen ()
{
  int32  oldRow;

  int32  newRow;

  int32  newHeight = bmh.biHeight + 6;
  int32  newWidth  = bmh.biWidth  + 6;


  uchar**  newImage = new uchar*[newHeight];
  uchar**  newRed = NULL;
  uchar**  newBlue = NULL;
  if  (color)
  {
     newRed  = new uchar*[newHeight];
     newBlue = new uchar*[newHeight];
  }

  for  (newRow = 0; newRow < newHeight; newRow++)
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

  newRow = 3;
  for  (oldRow = 0; oldRow < bmh.biHeight; oldRow++)
  {
    memcpy ((newImage[newRow] + 3), image[oldRow], bmh.biWidth);
    if  (color)
    {
      memcpy ((newRed[newRow]  + 3), red [oldRow], bmh.biWidth);
      memcpy ((newBlue[newRow] + 3), blue[oldRow], bmh.biWidth);
    }
    newRow++;
  }

  for  (oldRow = 0; oldRow < bmh.biHeight; oldRow++)
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
  bmh.biHeight = newHeight;
  bmh.biWidth  = newWidth;
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
  uint32  bmpRowWidthInBytes = (bmh.biWidth + 1) / 2;

  int32  paddingBytes = bmpRowWidthInBytes % 4;
  if  (paddingBytes != 0)
    paddingBytes = 4 - paddingBytes;

  bmpRowWidthInBytes = bmpRowWidthInBytes + paddingBytes;

  size_t x = sizeof (Bmp4BitRecs);

  Bmp4BitRecs*  rowData = new Bmp4BitRecs [bmpRowWidthInBytes];

  int32  row;

  for  (row = bmh.biHeight - 1; row >= 0; row--)
  {
    x = fread (rowData, sizeof (Bmp4BitRecs), bmpRowWidthInBytes, inFile);

    if  (x < bmpRowWidthInBytes)
    {
      successfull = false;
      cerr << "***ERROR***  BmpImage::Load4BitColor  Error Reading File" << std::endl;
      return;
    }

    int32  col;

    for  (col = 0; col < bmh.biWidth; col++)
    {
      int32  offset = col / 2;

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

  delete  rowData;
}  /* Load4BitColor */






void  BmpImage::Load8BitColor (FILE*  inFile,
                               bool&  successfull
                              )
{
  //cout << "BmpImage::Load8BitColor" << std::endl;
  successfull = true;
  color = true;
  numOfColors = 256;
  paletteEntries = numOfColors;
  AllocateRaster ();

  fseek (inFile, sizeof (hdr) + sizeof (bmh), SEEK_SET);
  fread (palette, sizeof (RGBQUAD), paletteEntries, inFile);


/*
  cout << std::endl;
  for  (x = 0;  x < paletteEntries;  x++)
    cout << x << "\t" << (uint32)palette[x].rgbRed
              << "\t" << (uint32)palette[x].rgbGreen
              << "\t" << (uint32)palette[x].rgbBlue
              << std::endl;
  osWaitForEnter ();
*/

  uint32  bmpRowWidthInBytes = bmh.biWidth;
  uint32  paddingBytes = bmpRowWidthInBytes % 4;
  if  (paddingBytes != 0)
    paddingBytes = 4 - paddingBytes;

  bmpRowWidthInBytes = bmpRowWidthInBytes + paddingBytes;

  fseek (inFile, hdr.bfOffBits, SEEK_SET);

  uchar*  rowData = new uchar[bmpRowWidthInBytes];

  int32  row;

  for  (row = bmh.biHeight - 1; row >= 0; row--)
  {
    memset (rowData, 0, bmpRowWidthInBytes);
    size_t buffRead = fread (rowData, 1, bmpRowWidthInBytes, inFile);
    if  (buffRead < bmpRowWidthInBytes)
    {
      cerr << std::endl 
        << "***ERROR*** BmpImage::Load8BitColor, Error Reading File" << std::endl
        << std::endl;
      successfull = false;
      delete  rowData;  rowData = NULL;
      return;
    }

    int32  col;

    for  (col = 0;  col < bmh.biWidth;  col++)
    {
      int32  palletIdx = rowData[col];

      //cout << "RowData" << "\t" << col << "\t" << palletIdx << "\t" << (uint32)(palette[palletIdx].rgbGreen) << std::endl;
      blue  [row][col] = palette[palletIdx].rgbBlue;
      image [row][col] = palette[palletIdx].rgbGreen;
      red   [row][col] = palette[palletIdx].rgbRed;
    }
  }

  successfull = true;

  delete  rowData;   rowData = NULL;

}  /* Load8BitColor */




void  BmpImage::Load4BitColorCompressed (FILE*  inFile,
                                         bool&  successfull
                                        )
{
  successfull = true;
  int32  x;

  size_t  imageBuffSize = bmh.biSizeImage;

  uchar*  imageBuff = new uchar[imageBuffSize];

  size_t  buffRead = fread (imageBuff, sizeof (uchar), imageBuffSize, inFile);

  if  (buffRead < imageBuffSize)
  {
    cerr << "***ERROR*** BmpImage::Load4BitColorCompressed   *** Error ***, Invalid File Format." << std::endl;
    successfull = false;
    return;
  }

  int32  col = 0;
  int32  row = bmh.biHeight - 1;

  size_t  ifIDX = 0;

  while  (ifIDX < imageBuffSize)
  {
    if  (imageBuff[ifIDX] == 0)
    {
      // We have an Escape Sequence.
      ifIDX++;

      if  (imageBuff[ifIDX] == 0)
      {
        // End of Row.
        row--;
        col = 0;
        ifIDX++;
      }

      else if  (imageBuff[ifIDX] == 1)
      {
        // End of BitMap

        ifIDX = imageBuffSize;
      }

      else if  (imageBuff[ifIDX] == 2)
      {
        ifIDX++;
        col = col + imageBuff[ifIDX];

        ifIDX++;
        row = row + imageBuff[ifIDX];

        ifIDX++;
      }

      else
      {
        // We have Absolute Mode
        int32  len = imageBuff[ifIDX];
        ifIDX++;

        for  (x = 0; x < len;)
        {
          uchar  pix1 = imageBuff[ifIDX] / 16;
          uchar  pix2 = imageBuff[ifIDX] % 16;
          SetPixelValue (row, col, paletteMap[pix1]);
          x++;
          col++;

          if  (x < len)
          {
            SetPixelValue (row, col, paletteMap[pix2]);
            x++;
            col++;
          } 

          ifIDX++;
        }

        if (((len + 1) / 2) % 2 != 0)
           ifIDX++;
      }
    }

    else
    {
      // We have a RLE
      int32  len = imageBuff[ifIDX];
      ifIDX++;

      uchar  pix1 = imageBuff[ifIDX] / 16;
      uchar  pix2 = imageBuff[ifIDX] % 16;

      for  (x = 0; x < len;)
      {
        SetPixelValue (row, col, paletteMap[pix1]);
        x++;
        col++;

        if  (x < len)
        {
          SetPixelValue (row, col, paletteMap[pix2]);
          x++;
          col++;
        }
      }

      ifIDX++;
    }
  }

  delete  imageBuff;
}  /* Load4BitColorCompressed */





void  BmpImage::Load8BitColorCompressed (FILE*  inFile,
                                         bool&  successfull
                                        )
{
  int32  x;

  successfull = true;

  uint32  imageBuffSize = bmh.biSizeImage;

  uchar*  imageBuff = new uchar[imageBuffSize];

  size_t  buffRead = fread (imageBuff, sizeof (uchar), imageBuffSize, inFile);

  if  (buffRead < imageBuffSize)
  {
    cerr << "***ERROR***, Load4BitColorCompressed   *** Error ***, Invalid File Format." << std::endl;
    successfull = false;
    return;
  }

  int32  col = 0;
  int32  row = bmh.biHeight - 1;

  uint32  ifIDX = 0;

  while  (ifIDX < imageBuffSize)
  {
    if  (imageBuff[ifIDX] == 0)
    {
      // We have an Escape Sequence.
      ifIDX++;

      if  (imageBuff[ifIDX] == 0)
      {
        // End of Row.
        row--;
        col = 0;
        ifIDX++;
      }

      else if  (imageBuff[ifIDX] == 1)
      {
        // End of BitMap

        ifIDX = imageBuffSize;
      }

      else if  (imageBuff[ifIDX] == 2)
      {
        ifIDX++;
        col = col + imageBuff[ifIDX];

        ifIDX++;
        row = row + imageBuff[ifIDX];

        ifIDX++;
        // Insert Spaces
      }

      else
      {
        // We have Absolute Mode
        int32  len = imageBuff[ifIDX];
        ifIDX++;

        for  (x = 0; x < len;)
        {
          SetPixelValue (row, col, paletteMap[imageBuff[ifIDX]]);
          x++;
          col++;
          ifIDX++;
        }

        if  ((len % 2) != 0)
          ifIDX++;
      }
    }

    else
    {
      // We have a RLE
      int32  len = imageBuff[ifIDX];
      ifIDX++;

      int32  pixelVal = paletteMap[imageBuff[ifIDX]];

      if  (pixelVal == 0)
      {
        col = col + len;
      }
      else
      {
        for  (x = 0; x < len;)
        {
          SetPixelValue (row, col, pixelVal);
          x++;
          col++;
        }
      }

      ifIDX++;
    }
  }

  delete  imageBuff;
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

  uint32  bmpRowWidthInBytes = bmh.biWidth * 3;
  uint32  paddingBytes = bmpRowWidthInBytes % 4;
  if  (paddingBytes != 0)
    paddingBytes = 4 - paddingBytes;

  bmpRowWidthInBytes = bmpRowWidthInBytes + paddingBytes;

  fseek (inFile, hdr.bfOffBits, SEEK_SET);

  uchar*  rowData = new uchar[bmpRowWidthInBytes];

  int32  row;

  for  (row = bmh.biHeight - 1; row >= 0; row--)
  {
    memset (rowData, 0, bmpRowWidthInBytes);
    size_t buffRead = fread (rowData, 1, bmpRowWidthInBytes, inFile);
    if  (buffRead < bmpRowWidthInBytes)
    {
      cerr << std::endl 
        << "***ERROR*** BmpImage::Load24BitColor, Error Reading File" << std::endl
        << std::endl;
      successfull = false;
      delete  rowData;  rowData = NULL;
      return;
    }

    int32  col;

    for  (col = 0;  col < bmh.biWidth;  col++)
    {
      int32  offset = col * 3;
      blue  [row][col] = rowData[offset];
      image [row][col] = rowData[offset + 1];
      red   [row][col] = rowData[offset + 2];
    }
  }

  successfull = true;

  delete  rowData;   rowData = NULL;
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

  int32  row;

  image = new uchar*[bmh.biHeight];
  if  (color)
  {
    red  = new uchar*[bmh.biHeight];
    blue = new uchar*[bmh.biHeight];
  }

  for  (row = 0; row < bmh.biHeight; row++)
  {
    image[row] = new uchar[bmh.biWidth];
    memset (image[row], 0, bmh.biWidth);
    if  (color)
    {
      red[row]  = new uchar[bmh.biWidth];
      blue[row] = new uchar[bmh.biWidth];
      memset (red [row], 0, bmh.biWidth);
      memset (blue[row], 0, bmh.biWidth);
    }
  }
}  /* AllocateRaster */




void  BmpImage::CleanUpMemory ()
{
  int32  x = 0;
  if  (image)
  {
    for  (x = bmh.biHeight - 1; x >= 0; x--)
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

    for  (x = bmh.biHeight - 1; x >= 0; x--)
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
  int32  newCol;
  int32  newHeight = bmh.biHeight / 2;
  int32  newWidth  = bmh.biWidth  / 2;
  int32  newRow;

  int32  oldCol;
  int32  oldRow;

  uchar**  newImage = new uchar*[newHeight];
  for  (newRow = 0; newRow < newHeight; newRow++)
  {
    oldRow = newRow * 2;

    newImage[newRow] = new uchar[newWidth];
   
    for  (newCol = 0; newCol < newWidth;  newCol++)
    {
      oldCol = newCol * 2;
      newImage[newRow][newCol] = image[oldRow][oldCol];
    }
  }

  for  (oldRow = 0; oldRow < bmh.biHeight; oldRow++)
  {
    delete  image[oldRow];
    image[oldRow] = NULL;
  }

  delete  image;

  image = newImage;

  bmh.biHeight = newHeight;
  bmh.biWidth  = newWidth;
}  /* DownSize */




uchar&  BmpImage::Pixel (int32  row, 
                         int32  col)
{
  return  image[row][col];
}




void  BmpImage::SetPixelValue (int32  row, 
                               int32  col, 
                               int32  pixel
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
  image[row][col] = (uchar)pixel;
}  /* SetPixelValue */




bool  BmpImage::AreThereEdgePixels ()
{
  int32  row;
  int32  col;

  int32  height = Height ();
  int32  width  = Width ();

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


  for  (col = 0;  col < width;  col++)
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

  int32  lastCol0 = width - 3;
  int32  lastCol1 = width - 2;
  int32  lastCol2 = width - 1;

  int32  lastRowToCheck = height - 3;

  for  (row = 3;  row < lastRowToCheck;  row++)
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
  int32  col = 0;
  int32  row = 0;
  int32  x;


  bool* potVertLine = new bool[bmh.biWidth];
  bool* pvl = potVertLine;

  for  (col = 0; col < bmh.biWidth; col++)
  {
    *pvl = true;
    for  (row = 0; ((row < bmh.biHeight) && (*pvl));  row++)
    {
      if  (image[row][col] == 7)
        *pvl = false;
    }
    
    pvl++;
  }


  col = 0;

  while  (col < bmh.biWidth)
  {
    if  (potVertLine[col])  
    {
      int32  firstCol = col;
      int32  lastCol  = col;

      if  (col < (bmh.biWidth - 1))
      {
        while  ((col < bmh.biWidth)  &&  (potVertLine[col]))
        {
          col++;
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

      for  (row = 0; row < bmh.biHeight; row++)
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
          for  (x = firstCol; x <= lastCol; x++)
            image[row][x] = 0;
        }
        else
        {
          for  (x = firstCol; x <= lastCol; x++)
            image[row][x] = (leftPixel + rightPixel) / 2;

          // Set up boolean for next loop around.
          leftSideIsClear = true;
          rightSizeIsClear = true;
        }
      }
    }

    col++;
  }

  delete  potVertLine;

} /* EliminateVerticalLines */





void  BmpImage::Print ()
{
  int32  x;
  int32  y;

  for  (x = 0; x < bmh.biHeight; x++)
  {
    for  (y = 0; y < Min ((long)bmh.biWidth, 76L); y++)
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
  int32  row;
  int32  col;

  for  (row = 0; row < bmh.biHeight; row++)
  {
    for  (col = 0; col < bmh.biWidth; col++)
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

  int32  x = 0;
  int32  y = 0;

  CodedPixels pixelData (bmh.biHeight, bmh.biWidth);

  for  (x = (int32)bmh.biHeight - 1; x >= 0; x--)
  {
    for  (y = 0; y < bmh.biWidth; y++)
    {
      if  (color)
      {
        int32  gsVal = (int32)((float)(0.5f + red[x][y]) * 0.30f + (float)(image[x][y]) * 0.59f + (float)(blue[x][y]) * 0.11f);
        pixelData.AddPixel ((uchar)(gsVal >> 4));
      }
      else
      {
        pixelData.AddPixel (image[x][y] >> 4);
      }
    }

    pixelData.EOL ();
  }

  int32   imageBuffLen  = 0;
  uchar*  imageBuff     = NULL;

  numOfColors = 16;
  imageBuff = pixelData.CreatePixelDataStructure4Bit (imageBuffLen);
  bmh.biBitCount    = 4;
  bmh.biCompression = BI_RLE4; // BI_RGB;
  SetUp4BitPallet ();

  bmh.biSizeImage    = imageBuffLen; 
  bmh.biClrUsed      = numOfColors;
  bmh.biClrImportant = numOfColors;

  hdr.bfSize = 14 + 40 + paletteEntries * 4 + bmh.biSizeImage;
  hdr.bfOffBits = 40 + 14 + paletteEntries * 4;

  x = (int32)fwrite (&hdr,   sizeof (hdr), 1, outFile);
  x = (int32)fwrite (&bmh,   sizeof (bmh), 1, outFile);
  x = (int32)fwrite (palette, sizeof (RGBQUAD), paletteEntries, outFile);
  x = (int32)fwrite (imageBuff, 1, imageBuffLen, outFile);

  delete  imageBuff;
  fclose (outFile);
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

  int32  x = 0;
  int32  y = 0;

  CodedPixels pixelData (bmh.biHeight, bmh.biWidth);

  for  (x = (int32)bmh.biHeight - 1; x >= 0; x--)
  {
    for  (y = 0; y < bmh.biWidth; y++)
    {
      if  (color)
      {
        int32  gsVal = (int32)((float)(0.5f + red[x][y]) * 0.30f + (float)(image[x][y]) * 0.59f + (float)(blue[x][y]) * 0.11f);
        pixelData.AddPixel ((uchar)(gsVal));
      }
      else
      {
        pixelData.AddPixel (image[x][y]);
      }
    }

    pixelData.EOL ();
  }

  int32   imageBuffLen = 0;
  uchar*  imageBuff    = NULL;

  numOfColors = 256;
  imageBuff = pixelData.CreatePixelDataStructure8Bit (imageBuffLen);
  bmh.biBitCount    = 8;
  bmh.biCompression = BI_RLE8; // BI_RGB;
  SetUp8BitPallet ();

  bmh.biSizeImage    = imageBuffLen; 
  bmh.biClrUsed      = numOfColors;
  bmh.biClrImportant = numOfColors;

  hdr.bfSize = 14 + 40 + paletteEntries * 4 + bmh.biSizeImage;
  hdr.bfOffBits = 40 + 14 + paletteEntries * 4;

  x = (int32)fwrite (&hdr,   sizeof (hdr), 1, outFile);
  x = (int32)fwrite (&bmh,   sizeof (bmh), 1, outFile);
  x = (int32)fwrite (palette, sizeof (RGBQUAD), paletteEntries, outFile);
  x = (int32)fwrite (imageBuff, 1, imageBuffLen, outFile);

  delete  imageBuff;
  fclose (outFile);
}  /* SaveGrayscaleInverted8Bit */





void  BmpImage::Save (const KKStr&  _fileName)
{
  fileName = _fileName;
  FILE*  outFile = osFOPEN (fileName.Str (), "wb");
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

  fclose (outFile);
}  /* Save */




void  BmpImage::SaveGrayScale (FILE*  outFile)
{
  int32   x;
  int32   y;

  CodedPixels pixelData (bmh.biHeight, bmh.biWidth);

  for  (x = (int32)bmh.biHeight - 1; x >= 0; x--)
  {
    for  (y = 0; y < bmh.biWidth; y++)
    {
      pixelData.AddPixel (image[x][y]);
    }

    pixelData.EOL ();
  }

  int32   imageBuffLen;
  uchar*  imageBuff;

  numOfColors = 256;
  imageBuff = pixelData.CreatePixelDataStructure8Bit (imageBuffLen);
  bmh.biCompression  = BI_RLE8;
  bmh.biBitCount     = 8;

  bmh.biSizeImage    = imageBuffLen; 
  bmh.biClrUsed      = numOfColors;
  bmh.biClrImportant = numOfColors;
  paletteEntries     = numOfColors;

  hdr.bfSize = 14 + 40 + paletteEntries * 4 + bmh.biSizeImage;
  hdr.bfOffBits = 40 + 14 + paletteEntries * 4;

  #ifndef  WIN32
  WriteWORD  (outFile, hdr.bfType);
  WriteDWORD (outFile, hdr.bfSize);
  WriteWORD  (outFile, hdr.bfReserved1);
  WriteWORD  (outFile, hdr.bfReserved2);
  WriteDWORD (outFile, hdr.bfOffBits);
  #else

  x = (int32)fwrite (&hdr,   sizeof (hdr), 1, outFile);
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

  x = (int32)fwrite (&bmh,   sizeof (bmh), 1, outFile);
  #endif


  #ifndef  WIN32
  for  (x = 0; x < paletteEntries; x++)
  {
    WriteBYTE (outFile, palette[x].rgbBlue);
    WriteBYTE (outFile, palette[x].rgbGreen);
    WriteBYTE (outFile, palette[x].rgbRed);
    WriteBYTE (outFile, palette[x].rgbReserved);
  }
  #else
  x = (int32)fwrite (palette, sizeof (RGBQUAD), paletteEntries, outFile);
  #endif

  x = (int32)fwrite (imageBuff, 1, imageBuffLen, outFile);
  delete  imageBuff;
}  /* SaveGrayScale */




void  BmpImage::SaveColor (FILE*  outFile)
{
  /** @todo  Need to finish implementing and testing compressed color.  */
  //PalletBuilderPtr  palletBuilder = BuildPalletFromRasterData ();
  //if  (palletBuilder->NumOfColors () <= 256)
  //  SaveColorCompressed256 (palletBuilder, outFile);
  //else 
  //  SaveColor24BPP (outFile);
  //delete  palletBuilder;

  SaveColor24BPP (outFile);
}  /* SaveColor */


/** @brief  Will write a color compressd BMP file with a maximum of 256 colors. */
void  BmpImage::SaveColorCompressed256 (PalletBuilderPtr  palletBuilder,
                                        FILE*             outFile
                                       )
{
  int32   x;
  int32   y;

  uchar*  redRow   = NULL;
  uchar*  greenRow = NULL;
  uchar*  blueRow  = NULL;

  int32  palletIdx = 0;

  CodedPixels  pixelData (bmh.biHeight, bmh.biWidth);

  for  (x = (int32)bmh.biHeight - 1; x >= 0; x--)
  {
    redRow   = red[x];
    greenRow = image[x];
    blueRow  = blue[x];
    for  (y = 0; y < bmh.biWidth; y++)
    {
      palletIdx = palletBuilder->PalletIndex (redRow[y], greenRow[y], blueRow[y]);

      if  (palletIdx < 0)
        palletIdx = 0;

      else if  (palletIdx > 255)
        palletIdx = 255;

      pixelData.AddPixel ((uchar)palletIdx);
    }

    pixelData.EOL ();
  }

  int32   imageBuffLen;
  uchar*  imageBuff;

  numOfColors = paletteEntries;
  imageBuff = pixelData.CreatePixelDataStructure8Bit (imageBuffLen);
  bmh.biCompression  = BI_RLE8;  /* 'BI_RLE8' is wrere each pixel is reprsented by a 8 but number that indexes into a color palet. */
  bmh.biBitCount     = 8;

  bmh.biSizeImage    = imageBuffLen; 
  bmh.biClrUsed      = numOfColors;
  bmh.biClrImportant = numOfColors;

  hdr.bfSize = 14 + 40 + paletteEntries * 4 + bmh.biSizeImage;
  hdr.bfOffBits = 40 + 14 + paletteEntries * 4;

  #ifndef  WIN32
  WriteWORD  (outFile, hdr.bfType);
  WriteDWORD (outFile, hdr.bfSize);
  WriteWORD  (outFile, hdr.bfReserved1);
  WriteWORD  (outFile, hdr.bfReserved2);
  WriteDWORD (outFile, hdr.bfOffBits);
  #else

  x = (int32)fwrite (&hdr,   sizeof (hdr), 1, outFile);
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

  x = (int32)fwrite (&bmh,   sizeof (bmh), 1, outFile);
  #endif


  #ifndef  WIN32
  for  (x = 0; x < paletteEntries; x++)
  {
    WriteBYTE (outFile, palette[x].rgbBlue);
    WriteBYTE (outFile, palette[x].rgbGreen);
    WriteBYTE (outFile, palette[x].rgbRed);
    WriteBYTE (outFile, palette[x].rgbReserved);
  }
  #else
  x = (int32)fwrite (palette, sizeof (RGBQUAD), paletteEntries, outFile);
  #endif


  x = (int32)fwrite (imageBuff, 1, imageBuffLen, outFile);
  delete  imageBuff;
}  /* SaveColorCompressed256 */



void  BmpImage::SaveColor24BPP (FILE*  outFile)
{
  int32 x = 0;
  int32  bytesPerRow = Width () * 3;
  int32  bufferPerRow = 0;
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
  x = (int32)fwrite (&hdr,   sizeof (hdr), 1, outFile);
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

  x = (int32)fwrite (&bmh,   sizeof (bmh), 1, outFile);
  #endif


  int32  row, col;
  for  (row = (int32)Height () - 1;  row >= 0;  row--)
  {
    for  (col = 0;  col < (int32)Width ();  col++)
    {
      fwrite (&(blue [row][col]), 1, 1, outFile);
      fwrite (&(image[row][col]), 1, 1, outFile);
      fwrite (&(red  [row][col]), 1, 1, outFile);
    }

    uchar  pad = 0;
    for  (x = 0;  x < bufferPerRow;  x++)
      fwrite (&pad, 1, 1, outFile);
  }
}  /* SaveColor24BPP */




void  BmpImage::AddPixel (uint32 row, 
                          uint32 col, 
                          uchar  pixValue
                         )
{
  image[row][col] = pixValue;
}



void BmpImage::ClearImage()
{
	int32 row;
	for  (row = 0; row < bmh.biHeight; row++)
	{
		memset (image[row], 0, bmh.biWidth );
	}
}



const   
uchar* BmpImage::BlueRow (int32 row)  const
{
  if  ((blue == NULL)  ||  (row < 0)  ||  (row >= (int32)Height ()))
  {
    cerr << std::endl
         << std::endl
         << "BmpImage::BlueRow   *** ERROR ***  Invalid Row[" << row << "]." << std::endl
         << std::endl;
    //osWaitForEnter ();
  }

  return  blue[row];
}  /* BlueRow */



const   
uchar* BmpImage::ImageRow (int32 row)  const
{
  if  ((row < 0)  ||  (row >= (int32)Height ()))
  {
    cerr << std::endl
         << std::endl
         << "BmpImage   *** ERROR ***  Invalid Row[" << row << "]." << std::endl
         << std::endl;
    //osWaitForEnter ();
  }

  return  image[row];
}  /* ImageRow */



const   
uchar* BmpImage::RedRow (int32 row)  const
{
  if  ((red == NULL)  ||  (row < 0)  ||  (row >= (int32)Height ()))
  {
    cerr << std::endl
         << std::endl
         << "BmpImage::RedRow   *** ERROR ***  Invalid Row[" << row << "]." << std::endl
         << std::endl;
    //osWaitForEnter ();
  }

  return  red[row];
}  /* RedRow */



bool  BmpImage::FourBitUncompressed () 
{
  return  ((bmh.biBitCount == 4)  &&  (bmh.biCompression == BI_RGB));
}

