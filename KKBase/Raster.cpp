/* Raster.cpp -- Class for one raster image.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"

#include <memory>
#include <math.h>
#include <limits.h>
#include <fstream>
#include <map>
#include <string.h>
#include <string>
#include <iostream>
#include <vector>

#include "MemoryDebug.h"

using namespace std;

#if defined(FFTW_AVAILABLE)
#  include  <fftw3.h>
//#else
//#  include  "kku_fftw.h"
#endif
#  include  "kku_fftw.h"

#include "Raster.h"

#include "KKBaseTypes.h"
#include "Blob.h"
#include "BMPImage.h"
#include "Compressor.h"
#include "ConvexHull.h"
#include "EigenVector.h"
#include "GoalKeeper.h"
#include "Histogram.h"
#include "ImageIO.h"
#include "KKException.h"
#include "kku_fftw.h"
#include "Matrix.h"
#include "MorphOpBinarize.h"
#include "MorphOpDilation.h"
#include "MorphOpErosion.h"
#include "MorphOpReduceByEvenMultiple.h"
#include "MorphOpReduceByFactor.h"
#include "MorphOpStretcher.h"
#include "MorphOpThinContour.h"
#include "OSservices.h"
#include "SimpleCompressor.h"
#include "MorphOpSobel.h"
using namespace KKB;


volatile GoalKeeperPtr  Raster::goalKeeper = NULL;
volatile bool           Raster::rasterInitialized = false;


KKStr  KKB::ColorChannelToKKStr (ColorChannels  c)
{
  switch  (c)  {
  case  ColorChannels::Red:   return "Red";
  case  ColorChannels::Green: return "Green";
  case  ColorChannels::Blue:  return "Blue";
  default:
    return "";
  }
}


ColorChannels  KKB::ColorChannelFromKKStr(const KKStr& s)
{
  uchar  c = (uchar)tolower(s.FirstChar());
  if  (c == 'r')  return ColorChannels::Red;
  if  (c == 'g')  return ColorChannels::Green;
  if  (c == 'b')  return ColorChannels::Blue;
  return ColorChannels::Green;
}



void  Raster::Initialize ()
{
  GoalKeeper::Create ("Raster", goalKeeper);

  goalKeeper->StartBlock ();
  if  (!rasterInitialized)
  {
    rasterInitialized = true;
    atexit (Raster::FinalCleanUp);
  }
  goalKeeper->EndBlock ();
}


void  Raster::FinalCleanUp ()
{
  GoalKeeper::Destroy (goalKeeper);
  goalKeeper = NULL;
}




/** Supports detection of memory leaks in Raster; maintains list of 'Raster' objects created. */
map<RasterPtr, RasterPtr>  Raster::allocatedRasterInstances;

void  Raster::AddRasterInstance (const RasterPtr  r)
{
  if  (!rasterInitialized)
    Initialize ();

  goalKeeper->StartBlock ();

  map<RasterPtr, RasterPtr>::iterator  idx;
  idx = allocatedRasterInstances.find (r);
  if  (idx != allocatedRasterInstances.end ())
  {
    cerr << std::endl << "Raster::AddRasterInstance   ***ERROR***   Raster Instance[" << r << "] Already in list." << std::endl << std::endl;
  }
  else
  {
    allocatedRasterInstances.insert (pair<RasterPtr, RasterPtr> (r, r));
  }
  goalKeeper->EndBlock ();
}


void  Raster::RemoveRasterInstance (const RasterPtr  r)
{
  if  (!rasterInitialized)
    Initialize ();

  goalKeeper->StartBlock ();

  map<RasterPtr, RasterPtr>::iterator  idx;
  idx = allocatedRasterInstances.find (r);
  if  (idx == allocatedRasterInstances.end ())
  {
    cerr << std::endl << "Raster::RemoveRasterInstance   ***ERROR***   Raster Instance[" << r << "] Not Found." << std::endl << std::endl;
  }
  else
  {
    allocatedRasterInstances.erase (idx);
  }
  goalKeeper->EndBlock ();
}  /* RemoveRasterInstance */



void  Raster::PrintOutListOfAllocatedrasterInstances ()
{
  map<RasterPtr, RasterPtr>::iterator  idx;
  for  (idx = allocatedRasterInstances.begin ();  idx != allocatedRasterInstances.end ();  ++idx)
  {
    RasterPtr  r = idx->first;
    cout << r << "\t"
             << r->Height () << "\t"
             << r->Width  () << "\t"
             << r->FileName ()
             << std::endl;
  }
}  /* PrintOutListOfAllocatedrasterInstances */





//****************************************************************************************
//*   Used to help quickly calculate a Intensity Histogram of a image.  The image has 8  *
//*   levels of gray scale, from 0 -> 7.                                                 *
//****************************************************************************************
//                                       1  1  1  1  1  1  1  1  1  1  2  2  2  2  2  2  2  2  2  2  3  3 
//         0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1 
kkint32  freqHistBucketIdx[256] = 
          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //   0  - 31
           1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //  31  - 63
           2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, //  64  - 95
           3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, //  96 - 127
           4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, // 128 - 159
           5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, // 160 - 191
           6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, // 192 - 223
           7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7  // 224 - 255
          };

kkint32  freqHist16BucketIdx[256] = 
          {0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, //   0  - 31
           2,   2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, //  31  - 63
           4,   4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5, //  64  - 95
           6,   6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7, //  96 - 127
           8,   8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9, // 128 - 159
           10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, // 160 - 191
           12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, // 192 - 223
           14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15  // 224 - 255
          };



inline  
kkint32  Min (kkint32  x1,  kkint32  x2)
{
  if  (x1 < x2)
    return  x1;
  else
    return  x2;
}



inline  
kkint32  Max6 (kkint32 x1,  kkint32 x2,  kkint32 x3,  
               kkint32 x4,  kkint32 x5,  kkint32 x6
              )
{
  kkint32  r;
  
  if  (x1 > x2)
    r = x1;
  else 
    r = x2;

  if  (x3 > r)  r = x3;
  if  (x4 > r)  r = x4;
  if  (x5 > r)  r = x5;
  if  (x6 > r)  r = x6;

  return  r;
}  /* Max6 */



inline  
kkint32  Max9 (kkint32 x1,  kkint32 x2,  kkint32 x3,  
               kkint32 x4,  kkint32 x5,  kkint32 x6,  
               kkint32 x7,  kkint32 x8,  kkint32 x9
              )
{
  kkint32  r;
  
  if  (x1 > x2)
    r = x1;
  else 
    r = x2;

  if  (x3 > r)  r = x3;
  if  (x4 > r)  r = x4;
  if  (x5 > r)  r = x5;
  if  (x6 > r)  r = x6;
  if  (x7 > r)  r = x7;
  if  (x8 > r)  r = x8;
  if  (x9 > r)  r = x9;

  return  r;
}  /* Max9 */



Raster::Raster ():
  backgroundPixelValue (0),
  backgroundPixelTH    (31),
  blobIds              (NULL),
  centroidCol          (0.0f),
  centroidRow          (0.0f),
  color                (false),
  fileName             (),
  foregroundPixelCount (0),
  foregroundPixelValue (255),
  fourierMag           (NULL),
  fourierMagArea       (NULL),
  height               (0),
  maxPixVal            (0),
  title                (),
  totPixels            (0),
  weOwnRasterData      (true),
  width                (0),

  redArea              (NULL),
  greenArea            (NULL),
  blueArea             (NULL),

  red                  (NULL),
  green                (NULL),
  blue                 (NULL)
{
  AddRasterInstance (this);
}



Raster::Raster (kkint32  _height,
                kkint32  _width
               ):

  backgroundPixelValue (0),
  backgroundPixelTH    (31),
  blobIds              (NULL),
  centroidCol          (0.0f),
  centroidRow          (0.0f),
  color                (false),
  divisor              (1),
  fileName             (),
  foregroundPixelCount (0),
  foregroundPixelValue (255),
  fourierMag           (NULL),
  fourierMagArea       (NULL),
  height               (_height),
  maxPixVal            (0),
  title                (),
  totPixels            (_height * _width),
  weOwnRasterData      (true),
  width                (_width),

  redArea              (NULL),
  greenArea            (NULL),
  blueArea             (NULL),

  red                  (NULL),
  green                (NULL),
  blue                 (NULL)

{
  AddRasterInstance (this);
  AllocateImageArea ();
}



Raster::Raster (kkint32 _height,
                kkint32 _width,
                bool  _color
               ):

  backgroundPixelValue (0),
  backgroundPixelTH    (31),
  blobIds              (NULL),
  centroidCol          (0.0f),
  centroidRow          (0.0f),
  color                (_color),
  divisor              (1),
  fileName             (),
  foregroundPixelCount (0),
  foregroundPixelValue (255),
  fourierMag           (NULL),
  fourierMagArea       (NULL),
  height               (_height),
  maxPixVal            (0),
  title                (),
  totPixels            (_height * _width),
  weOwnRasterData      (true),
  width                (_width),

  redArea              (NULL),
  greenArea            (NULL),
  blueArea             (NULL),

  red                  (NULL),
  green                (NULL),
  blue                 (NULL)

{
  AddRasterInstance (this);
  if  (color)
  {
    backgroundPixelValue = 255;
    foregroundPixelValue = 0;
    backgroundPixelTH    = 255 - 31;
  }

  AllocateImageArea ();
}


/**
 *@brief  Constructs a Raster from a BMP image loaded from disk.
 *@details If BMP Image is a gray-scale value pixel values will be reversed.  See description of gray-scale constructor.
 */
Raster::Raster (const BmpImage&  _bmpImage):

  backgroundPixelValue  (0),
  backgroundPixelTH     (31),
  blobIds               (NULL),
  centroidCol           (0.0f),
  centroidRow           (0.0f),
  color                 (false),
  divisor               (1),
  fileName              (_bmpImage.FileName ()),
  foregroundPixelCount  (0),
  foregroundPixelValue  (255),
  fourierMag            (NULL),
  fourierMagArea        (NULL),
  height                (_bmpImage.Height ()),
  maxPixVal             (0),
  title                 (),
  totPixels             (_bmpImage.Height () * _bmpImage.Width ()),
  weOwnRasterData       (true),
  width                 (_bmpImage.Width ()),

  redArea               (NULL),
  greenArea             (NULL),
  blueArea              (NULL),

  red                   (NULL),
  green                 (NULL),
  blue                  (NULL)

{
  AddRasterInstance (this);
  color = _bmpImage.Color ();
  AllocateImageArea ();

  kkint32  row;

  for  (row = 0; row < height; row++)
  {
    const uchar* imageRow = _bmpImage.ImageRow (row);
    memcpy (green[row], imageRow, width);
    if  (color)
    {
      const uchar*  blueRow = _bmpImage.BlueRow (row);
      memcpy (blue[row], blueRow, width);

      const uchar*  redRow = _bmpImage.RedRow (row);
      memcpy (red[row], redRow, width);
    }
  }
}





Raster::Raster (const Raster&  _raster):

  backgroundPixelValue  (_raster.backgroundPixelValue),
  backgroundPixelTH     (_raster.backgroundPixelTH),
  blobIds               (NULL),
  centroidCol           (_raster.centroidCol),
  centroidRow           (_raster.centroidRow),
  color                 (_raster.color),
  divisor               (1),
  fileName              (_raster.fileName),
  foregroundPixelCount  (_raster.foregroundPixelCount),
  foregroundPixelValue  (_raster.foregroundPixelValue),
  fourierMag            (NULL),
  fourierMagArea        (NULL),
  height                (_raster.height),
  maxPixVal             (_raster.maxPixVal),
  title                 (),
  totPixels             (_raster.totPixels),
  weOwnRasterData       (true),
  width                 (_raster.width),

  redArea          (NULL),
  greenArea        (NULL),
  blueArea         (NULL),

  red              (NULL),
  green            (NULL),
  blue             (NULL)

{
  AddRasterInstance (this);

  AllocateImageArea ();
  if  (!greenArea)  throw KKException ("Raster::Raster  Failed to allocate 'greenArea'.");
  memcpy (greenArea, _raster.greenArea, totPixels);

  if  (color)
  {
    if  (!redArea)   throw KKException("Raster::Raster  Allocation of 'redArea'  failed.");
    if  (!blueArea)  throw KKException("Raster::Raster  Allocation of 'blueArea' failed.");
    memcpy (redArea,  _raster.redArea,  totPixels);
    memcpy (blueArea, _raster.blueArea, totPixels);
  }

  if  (_raster.fourierMagArea)
  {
    AllocateFourierMagnitudeTable ();
    if  (!fourierMagArea)  throw KKException ("Raster::Raster  Allocation of 'fourierMagArea' failed.");
    memcpy (fourierMagArea, _raster.fourierMagArea, sizeof (float) * totPixels);
  }
}



Raster::Raster (const Raster&  _raster,
                kkint32        _row,
                kkint32        _col,
                kkint32        _height,
                kkint32        _width
               ):

  backgroundPixelValue (_raster.backgroundPixelValue),
  backgroundPixelTH    (_raster.backgroundPixelTH),
  blobIds              (NULL),
  centroidCol          (-1.0f),
  centroidRow          (-1.0f),
  color                (_raster.color),
  divisor              (1),
  fileName             (),
  foregroundPixelCount (0),
  foregroundPixelValue (_raster.foregroundPixelValue),
  fourierMag           (NULL),
  fourierMagArea       (NULL),
  height               (_height),
  maxPixVal            (0),
  title                (),
  totPixels            (_height * _width),
  weOwnRasterData      (true),
  width                (_width),

  redArea   (NULL),
  greenArea (NULL),
  blueArea  (NULL),

  red       (NULL),
  green     (NULL),
  blue      (NULL)

{
  AddRasterInstance (this);

  green = _raster.GetSubSet (_raster.green, _row, _col, _height, _width);
  greenArea = green[0];

  if  (color)
  {
    red = _raster.GetSubSet (_raster.red, _row, _col, _height, _width);
    redArea = red[0];

    blue = _raster.GetSubSet (_raster.blue, _row, _col, _height, _width);
    blueArea = blue[0];
  }
}



Raster::Raster (const Raster& _raster,
                MaskTypes     _mask,
                kkint32       _row,
                kkint32       _col
               ):

  backgroundPixelValue (_raster.backgroundPixelValue),
  backgroundPixelTH    (_raster.backgroundPixelTH),
  blobIds              (NULL),
  centroidCol          (-1),
  centroidRow          (-1),
  color                (false),
  divisor              (1),
  fileName             (),
  foregroundPixelCount (0),
  foregroundPixelValue (_raster.foregroundPixelValue),
  fourierMag           (NULL),
  fourierMagArea       (NULL),
  height               (MorphOp::Biases (_mask) * 2 + 1),
  maxPixVal            (0),
  title                (),
  totPixels            (0),
  weOwnRasterData      (true),
  width                (MorphOp::Biases (_mask) * 2 + 1),

  redArea   (NULL),
  greenArea (NULL),
  blueArea  (NULL),

  red       (NULL),
  green     (NULL),
  blue      (NULL)

{
  AddRasterInstance (this);

  totPixels = height * width;

  kkint32  r = _row - MorphOp::Biases (_mask);
  kkint32  c = _col - MorphOp::Biases (_mask);

  green = _raster.GetSubSet (_raster.green, r, c, height, width);  
  greenArea = green[0];
  if  (color)
  {
    red = _raster.GetSubSet (_raster.red, r, c, height, width);
    redArea = red[0];

    blue = _raster.GetSubSet (_raster.blue, r, c, height, width);
    blueArea = blue[0];
  }
}




Raster::Raster (const KKStr&  _fileName,
                bool&          validFile
               ):

  backgroundPixelValue (0),
  backgroundPixelTH    (31),
  blobIds              (NULL),
  centroidCol          (-1),
  centroidRow          (-1),
  color                (false),
  divisor              (1),
  fileName             (_fileName),
  foregroundPixelCount (0),
  foregroundPixelValue (255),
  fourierMag           (NULL),
  fourierMagArea       (NULL),
  height               (0),
  maxPixVal            (0),
  title                (),
  totPixels            (0),
  weOwnRasterData      (true),
  width                (0),

  redArea   (NULL),
  greenArea (NULL),
  blueArea  (NULL),

  red       (NULL),
  green     (NULL),
  blue      (NULL)

{
  AddRasterInstance (this);

  validFile = true;
  
  RasterPtr r = ReadImage (fileName);

  if  (!r)
  {
    validFile = false;
    delete  r;
    r = NULL;
    return;
  }

  red       = r->Red   ();
  green     = r->Green ();
  blue      = r->Blue  ();
  height    = r->Height ();
  width     = r->Width ();
  redArea   = r->redArea;
  greenArea = r->greenArea;
  blueArea  = r->blueArea;
  maxPixVal = r->MaxPixVal ();
  totPixels = r->totPixels;
  color     = r->color;
  foregroundPixelCount = r->ForegroundPixelCount ();

  r->weOwnRasterData = false;
  weOwnRasterData = true;
  delete  r;
}




Raster::Raster (kkint32  _height,
                kkint32  _width,
                uchar*   _grayScaleData,
                uchar**  _grayScaleRows
               ):
  backgroundPixelValue (0),
  backgroundPixelTH    (31),
  blobIds              (NULL),
  centroidCol          (0.0f),
  centroidRow          (0.0f),
  color                (false),
  divisor              (1),
  fileName             (),
  foregroundPixelCount (0),
  foregroundPixelValue (255),
  fourierMag           (NULL),
  fourierMagArea       (NULL),
  height               (_height),
  maxPixVal            (0),
  title                (),
  totPixels            (_height * _width),
  weOwnRasterData      (false),
  width                (_width),

  redArea              (NULL),
  greenArea            (_grayScaleData),
  blueArea             (NULL),

  red                  (NULL),
  green                (_grayScaleRows),
  blue                 (NULL)

{
  AddRasterInstance (this);
}



Raster::Raster (kkint32       _height,
                kkint32       _width,
                const uchar*  _grayScaleData
               ):
  backgroundPixelValue (0),
  backgroundPixelTH    (31),
  blobIds              (NULL),
  centroidCol          (0.0f),
  centroidRow          (0.0f),
  color                (false),
  divisor              (1),
  fileName             (),
  foregroundPixelCount (0),
  foregroundPixelValue (255),
  fourierMag           (NULL),
  fourierMagArea       (NULL),
  height               (_height),
  maxPixVal            (0),
  title                (),
  totPixels            (_height * _width),
  weOwnRasterData      (true),
  width                (_width),

  redArea              (NULL),
  greenArea            (NULL),
  blueArea             (NULL),

  red                  (NULL),
  green                (NULL),
  blue                 (NULL)

{
  AddRasterInstance (this);
  AllocateImageArea ();
  if  (!greenArea)  throw KKException ("Raster::Raster   Failed to allocate 'greenArea'.");
  memcpy (greenArea, _grayScaleData, totPixels);
}


Raster::Raster (kkint32       _height,
                kkint32       _width,
                const uchar*  _redChannel,
                const uchar*  _greenChannel,
                const uchar*  _blueChannel
               ):
  backgroundPixelValue (0),
  backgroundPixelTH    (31),
  blobIds              (NULL),
  centroidCol          (0.0f),
  centroidRow          (0.0f),
  color                (true),
  divisor              (1),
  fileName             (),
  foregroundPixelCount (0),
  foregroundPixelValue (255),
  fourierMag           (NULL),
  fourierMagArea       (NULL),
  height               (_height),
  maxPixVal            (0),
  title                (),
  totPixels            (_height * _width),
  weOwnRasterData      (true),
  width                (_width),

  redArea              (NULL),
  greenArea            (NULL),
  blueArea             (NULL),

  red                  (NULL),
  green                (NULL),
  blue                 (NULL)

{
  AddRasterInstance (this);
  AllocateImageArea ();
  if  ((!_redChannel)  ||  (!_greenChannel)  ||  (!_blueChannel))
  {
    KKB::KKStr errMsg;
    errMsg << "Raster::Raster   ***ERROR***   One of the provided channels is 'NULL'.";
    cerr << std::endl << std::endl << errMsg << std::endl << std::endl;
    throw KKException (errMsg);
  }

  if ((!redArea) || (!greenArea) || (!blueArea))
  {
    KKB::KKStr errMsg;
    errMsg << "Raster::Raster   ***ERROR***   Not all channels were allocated.";
    cerr << std::endl << std::endl << errMsg << std::endl << std::endl;
    throw KKException(errMsg);
  }

  memcpy (redArea,   _redChannel,   totPixels);
  memcpy (greenArea, _greenChannel, totPixels);
  memcpy (blueArea,  _blueChannel,  totPixels);
}




Raster::~Raster ()
{
  RemoveRasterInstance (this);
  CleanUpMemory ();
}



void  Raster::CleanUpMemory ()
{
  if  (weOwnRasterData)
  {
    delete  green;
    delete  greenArea;
    delete  red;  
    delete  redArea;
    delete  blue; 
    delete  blueArea;
  }

  green     = NULL;
  greenArea = NULL;
  red       = NULL;
  redArea   = NULL;
  blue      = NULL;
  blueArea  = NULL;

  if  (fourierMagArea)
  {
    delete  fourierMagArea;  fourierMagArea = NULL;
    delete  fourierMag;      fourierMag     = NULL;
  }

  DeleteExistingBlobIds ();
}



kkMemSize  Raster::MemoryConsumedEstimated ()  const
{
  kkMemSize  blobIdsMem = 0;
  if  (blobIds)
    blobIdsMem = (kkint32)sizeof(kkint32) * totPixels + (kkint32)sizeof (kkint32*) * height;

  kkMemSize  fourierMem = 0;
  if  (fourierMagArea)
    fourierMem = (kkint32)sizeof (float) * totPixels + (kkint32)sizeof (float*) * height;

  kkMemSize  pixelMem = totPixels + (kkint32)sizeof (uchar*) * height;
  if  (color)
    pixelMem = pixelMem * 3;

  kkMemSize  memoryConsumedEstimated = (kkMemSize)(
    sizeof (uchar)     * 4   +
    sizeof (float)     * 2   +
    sizeof (bool)      * 2   +
    sizeof (kkint32)   * 4   +
    sizeof (kkint32**) * 1   +
    sizeof (uchar*)    * 3   + 
    sizeof (uchar**)   * 3   +
    sizeof (float*)    * 1   +
    sizeof (float**)   * 1   +
    fileName.MemoryConsumedEstimated () +
    blobIdsMem                          +
    fourierMem                          +
    pixelMem);

  return  memoryConsumedEstimated;
}



void  Raster::ReSize (kkint32  _height, 
                      kkint32  _width,
                      bool     _color
                     )
{
  CleanUpMemory ();
  height = _height;
  width  = _width;
  color  = _color;
  AllocateImageArea ();
}








void  Raster::Initialize (kkint32    _height,
                          kkint32    _width,
                          uchar*     _grayScaleData,
                          uchar**    _grayScaleRows,
                          bool       _takeOwnership
                         )
{
  CleanUpMemory ();
  height = _height;
  width  = _width;
  totPixels = height * width;
  color = false;

  if  (_grayScaleData == NULL)
    throw KKException ("Raster::Initialize    _grayScaleData == NULL");

  if  (_grayScaleRows == NULL)
    throw KKException ("Raster::Initialize    _grayScaleRows == NULL");

  greenArea = _grayScaleData;
  green = _grayScaleRows;

  weOwnRasterData = _takeOwnership;
}



void  Raster::Initialize (kkint32  _height,
                          kkint32  _width,
                          uchar*   _redArea,
                          uchar**  _red,
                          uchar*   _greenArea,
                          uchar**  _green,
                          uchar*   _blueArea,
                          uchar**  _blue,
                          bool     _takeOwnership
                         )
{
  CleanUpMemory ();
  height = _height;
  width  = _width;
  color  = true;

  totPixels = height * width;

  if  ((_red   == NULL)  ||  (_redArea   == NULL)  ||
       (_green == NULL)  ||  (_greenArea == NULL)  ||
       (_blue  == NULL)  ||  (_blueArea  == NULL)
      )
    throw KKException ("Raster::Initialize    One or more of the Color channels == NULL");

  redArea   = _redArea;     red    = _red;
  greenArea = _greenArea;   green  = _green;
  blueArea  = _blueArea;    blue   = _blue;
 
  weOwnRasterData = _takeOwnership;
}




void  Raster::TakeOwnershipOfAnotherRastersData (Raster&  otherRaster)
{
  CleanUpMemory ();
  backgroundPixelValue    = otherRaster.backgroundPixelValue;
  backgroundPixelTH       = otherRaster.backgroundPixelTH;
  blobIds                 = otherRaster.blobIds;
  centroidCol             = otherRaster.centroidCol;
  centroidRow             = otherRaster.centroidRow;
  color                   = otherRaster.color;
  divisor                 = otherRaster.divisor;
  fileName                = otherRaster.fileName;
  foregroundPixelCount    = otherRaster.foregroundPixelCount;
  foregroundPixelValue    = otherRaster.foregroundPixelValue;
  fourierMag              = otherRaster.fourierMag;
  fourierMagArea          = otherRaster.fourierMagArea;
  height                  = otherRaster.height;
  maxPixVal               = otherRaster.maxPixVal;
  title                   = otherRaster.title;
  totPixels               = otherRaster.totPixels;
  weOwnRasterData         = otherRaster.weOwnRasterData;
  width                   = otherRaster.width;
  redArea                 = otherRaster.redArea;
  greenArea               = otherRaster.greenArea;
  blueArea                = otherRaster.blueArea;
  red                     = otherRaster.red;
  green                   = otherRaster.green;
  blue                    = otherRaster.blue;
  otherRaster.weOwnRasterData = false;
  otherRaster.fourierMagArea = NULL;
  otherRaster.fourierMag     = NULL;
  otherRaster.blobIds        = NULL;
}  /* TakeOwnershipOfAnotherRastersData */




RasterPtr  Raster::AllocateARasterInstance (kkint32  _height,
                                            kkint32  _width,
                                            bool     _color
                                           )  const
{
  return new Raster (_height, _width, _color);
}



RasterPtr  Raster::AllocateARasterInstance (const Raster& r)  const
{
  return new Raster (r);
}



RasterPtr  Raster::AllocateARasterInstance (const Raster& _raster,  /**<  Source Raster                             */
                                            kkint32       _row,     /**<  Starting Row in '_raster' to copy from.             */
                                            kkint32       _col,     /**<  Starting Col in '_raster' to copy from.             */
                                            kkint32       _height,  /**<  Height of resultant raster. Will start from '_row'  */
                                            kkint32       _width    /**<  Width of resultant raster.                          */
                                           ) const
{
  return new Raster (_raster, _row, _col, _height, _width);
}




bool  Raster::ForegroundPixel (uchar  pixel)  const
{
  if  (backgroundPixelValue < 125)
     return  (pixel > backgroundPixelTH);
  else
     return  (pixel < backgroundPixelTH);
}  /* ForegroundPixel */



bool  Raster::ForegroundPixel (kkint32  row,
                               kkint32  col
                              )  const
{
  return (ForegroundPixel (green[row][col]));
}  /* ForegroundPixel */



bool  Raster::BackgroundPixel (uchar  pixel)  const
{
  if  (backgroundPixelValue < 125)
     return  (pixel <= backgroundPixelTH);
  else
     return  (pixel >= backgroundPixelTH);
}  /* ForegroundPixel */




bool  Raster::BackgroundPixel (kkint32  row,
                               kkint32  col
                              )  const
{
  if  (backgroundPixelValue < 125)
     return (green[row][col] <= backgroundPixelTH);
  else
     return (green[row][col] >= backgroundPixelTH);
}  /* ForegroundPixel */



kkint32 Raster::TotalBackgroundPixels () const
{
  kkint32  totalBackgroundPixels = 0;
  for  (kkint32 x = 0;  x < totPixels;  ++x)
  {
    if  (greenArea[x] > backgroundPixelTH)
      ++totalBackgroundPixels;
  }
  return  totalBackgroundPixels;
}  /* TotalBackgroundPixels */




float  Raster::CentroidCol ()  const
{
  if  (centroidCol >= 0)
    return  centroidCol;

  float  centroidColWeighted;
  float  centroidRowWeighted;

  kkint32  weight = 0;
  CalcCentroid (totPixels, weight, centroidRow, centroidCol, centroidRowWeighted, centroidColWeighted);

  return  centroidCol;
} 



float    Raster::CentroidRow ()  const
{
  if  (centroidRow >= 0)
    return  centroidRow;

  float  centroidColWeighted;
  float  centroidRowWeighted;
  kkint32  weight = 0;
  CalcCentroid (totPixels, weight, centroidRow, centroidCol, centroidRowWeighted, centroidColWeighted);
  return  centroidRow;
}




RasterPtr  Raster::CreatePaddedRaster (BmpImage&  image,
                                       kkint32    padding
                                      )
{
  kkint32  oldWidth  = image.Width  ();
  kkint32  oldHeight = image.Height ();

  kkint32  newWidth  = oldWidth  + 2 * padding;
  kkint32  newHeight = oldHeight + 2 * padding;

  RasterPtr  paddedRaster = new Raster (newHeight, newWidth, false);

  //const uchar**  oldRows = image.Image ();

  uchar**  newRows = paddedRaster->green;

  kkint32  newRow = padding;
  kkint32  row;
  kkint32  col;

  kkint32  paddedForgroudPixelCount = 0;

  for  (row = 0;  row < oldHeight;  row++)
  {
    const uchar* oldRow = image.ImageRow (row);
    
    kkint32  newCol = padding;
    for  (col = 0; col < oldWidth;  col++)
    {
      if  (oldRow[col] > 0)
         paddedForgroudPixelCount++;

      newRows[newRow][newCol] = oldRow[col];
      newCol++;
    }

    newRow++;
  }

  paddedRaster->foregroundPixelCount = paddedForgroudPixelCount;

  return  paddedRaster; 
}  /* CreatePaddedRaster */



RasterPtr   Raster::ReversedImage ()
{
  RasterPtr  result = AllocateARasterInstance (*this);
  result->ReverseImage ();
  return  result;
}  /* ReversedImage */




RasterPtr  Raster::StreatchImage (float  rowFactor,
                                  float  colFactor
                                 )  const
{
  MorphOpStretcher  streatcher (rowFactor, colFactor);
  return  streatcher.PerformOperation (this);
}




void   Raster::ReverseImage ()
{
  kkint32 x = 0;

  for  (x = 0;  x < totPixels;  x++)
  {
    greenArea[x] = (uchar)(255 - greenArea[x]);
    if  (color)
    {
      redArea[x]  = (uchar)(255 - redArea[x]);
      blueArea[x] = (uchar)(255 - blueArea[x]);
    }
  }

  uchar  temp = backgroundPixelValue;
  backgroundPixelValue = foregroundPixelValue;
  foregroundPixelValue = temp;
  backgroundPixelTH = (uchar)(255 - backgroundPixelTH);
}  /* ReversedImage */


void  Raster::AllocateImageArea ()
{
  CleanUpMemory ();

  totPixels = height * width;
  greenArea = new uchar [totPixels];
  if  (!greenArea)
  {
    cerr << std::endl << std::endl 
         << "Raster::AllocateImageArea      ***ERROR***    Error allocating memory" << std::endl
         << std::endl;
    osDisplayWarning ("Raster::AllocateImageArea      ***ERROR***    Error allocating memory");

    greenArea = NULL;
    return;
  }

  memset (greenArea, backgroundPixelValue, totPixels);
  green = new uchar*[height];

  if  (color)
  {
    redArea = new uchar [totPixels];
    memset (redArea, backgroundPixelValue, totPixels);

    blueArea = new uchar [totPixels];
    memset (blueArea, backgroundPixelValue, totPixels);

    red  = new uchar*[height];
    blue = new uchar*[height];  
  }

  kkint32  row;
  uchar* greenPtr = greenArea;
  uchar* redPtr   = redArea;
  uchar* bluePtr  = blueArea;
  
  for  (row = 0; row < height; row++)
  {
    green[row] = greenPtr;
    greenPtr = greenPtr + width;

    if (color)
    {
      red[row] = redPtr;
      redPtr = redPtr + width;

      blue[row] = bluePtr;
      bluePtr = bluePtr + width;
    }
  }
}  /* AllocateImageArea */


void  Raster::AllocateFourierMagnitudeTable ()
{
  fourierMagArea = new float[totPixels];
  fourierMag     = new float*[height];

  float* rowPtr = fourierMagArea;

  for  (kkint32 row = 0;  row < height;  row++)
  {
    fourierMag[row] = rowPtr;
    rowPtr          = rowPtr + width;
  }
}  /* AllocateFourierMagnitudeTable */



uchar  Raster::GetPixelValue (kkint32 row,  kkint32 col)  const

{
  if  ((row <  0)      ||  
       (row >= height) ||
       (col <  0)      ||
       (col >= width))
  {
    cerr << "Raster::GetPixelValue   *** ERROR ***,  Raster Dimensions Exceeded."       << std::endl;
    cerr << "                        Height[" << height << "]  Width[" << width << "]." << std::endl;
    cerr << "                        Row["    << row    << "]  Col["   << col   << "]." << std::endl;
    exit (-1);
  }

  return  green[row][col];
}  /* GetPixelValue */



void  Raster::GetPixelValue (kkint32 row,
                             kkint32 col,
                             uchar&  r,
                             uchar&  g,
                             uchar&  b
                            )  const
{
  if  ((row <  0)      ||  
       (row >= height) ||
       (col <  0)      ||
       (col >= width))
  {
    cerr << "Raster::GetPixelValue   *** ERROR ***,  Raster Dimensions Exceeded."       << std::endl;
    cerr << "                        Height[" << height << "]  Width[" << width << "]." << std::endl;
    cerr << "                        Row["    << row    << "]  Col["   << col   << "]." << std::endl;
    exit (-1);
  }

  g = green [row][col];

  if  (color)
  {
    r = red   [row][col];
    b = blue  [row][col];
  }
  else
  {
    r = 0;
    b = 0;
  }
}  /* GetPixelValue */




void  Raster::GetPixelValue (kkint32     row,
                             kkint32     col,
                             PixelValue& p
                            )  const
{
  GetPixelValue (row, col, p.r, p.g, p.b);
}  /* GetPixelValue */






uchar  Raster::GetPixelValue (ColorChannels  channel,
                              kkint32        row,
                              kkint32        col
                             )  const
{
  if  ((row <  0)      ||  
       (row >= height) ||
       (col <  0)      ||
       (col >= width))
  {
    cerr << "Raster::GetPixelValue   *** ERROR ***,  Raster Dimensions Exceeded."       << std::endl;
    cerr << "                        Height[" << height << "]  Width[" << width << "]." << std::endl;
    cerr << "                        Row["    << row    << "]  Col["   << col   << "]." << std::endl;
    exit (-1);
  }

  if  (channel == ColorChannels::Green)
    return  green [row][col];

  if  (!color)
  {
    cerr << "***ERROR*** Raster::GetPixelValue   *** ERROR ***,  Not a Color Raster."  << std::endl;
    exit (-1);
  }

  if  (channel == ColorChannels::Red)
    return red [row][col];
  else
    return blue[row][col];
}  /* GetPixelValue */




void   Raster::SetPixelValue (kkint32  row,
                              kkint32  col,
                              uchar  pixVal
                             )
{
  if  ((row <  0)      ||  
       (row >= height) ||
       (col <  0)      ||
       (col >= width))
  {
    cerr << "***ERROR*** Raster::SetPixelValue   *** ERROR ***,  Raster Dimensions Exceeded."       << std::endl;
    cerr << "                        Height[" << height << "]  Width[" << width << "]." << std::endl;
    cerr << "                        Row["    << row    << "]  Col["   << col   << "]." << std::endl;
    return;
  }

  green[row][col] = pixVal;
}  /* SetPixelValue */





void   Raster::SetPixelValue (kkint32            row,
                              kkint32            col,
                              const PixelValue&  pixVal
                             )
{
  if  ((row <  0)      ||  
       (row >= height) ||
       (col <  0)      ||
       (col >= width))
  {
    cerr << "***ERROR*** Raster::SetPixelValue   *** ERROR ***,  Raster Dimensions Exceeded."       << std::endl;
    cerr << "                        Height[" << height << "]  Width[" << width << "]." << std::endl;
    cerr << "                        Row["    << row    << "]  Col["   << col   << "]." << std::endl;
    return;
  }

  green[row][col] = pixVal.g;
  if  (color)
  {
    red  [row][col] = pixVal.r;
    blue [row][col] = pixVal.b;
  }
}  /* SetPixelValue  */




void   Raster::SetPixelValue (const  Point&       point,
                              const  PixelValue&  pixVal
                             )
{
  SetPixelValue (point.Row (), point.Col (), pixVal);
} /* SetPixelValue */




void   Raster::SetPixelValue (kkint32  row,
                              kkint32  col,
                              uchar  r,
                              uchar  g,
                              uchar  b
                             )
{
  if  ((row <  0)      ||  
       (row >= height) ||
       (col <  0)      ||
       (col >= width))
  {
    cerr << "***ERROR*** Raster::SetPixelValue   *** ERROR ***,  Raster Dimensions Exceeded."       << std::endl;
    cerr << "                        Height[" << height << "]  Width[" << width << "]." << std::endl;
    cerr << "                        Row["    << row    << "]  Col["   << col   << "]." << std::endl;
    return;
  }

  green[row][col] = g;
  if  (color)
  {
    red  [row][col] = r;
    blue [row][col] = b;
  }
}  /* SetPixelValue  */




void  Raster::SetPixelValue (ColorChannels  channel,
                             kkint32        row,
                             kkint32        col,
                             uchar          pixVal
                            )
{
  if  ((row <  0)      ||  
       (row >= height) ||
       (col <  0)      ||
       (col >= width))
  {
    cerr << "***ERROR*** Raster::SetPixelValue   *** ERROR ***,  Raster Dimensions Exceeded."       << std::endl;
    cerr << "                        Height[" << height << "]  Width[" << width << "]." << std::endl;
    cerr << "                        Row["    << row    << "]  Col["   << col   << "]." << std::endl;
    return;
  }

  if  (channel == ColorChannels::Green)
  {
    green[row][col] = pixVal;
    return;
  }

  if  (!color)
  {
    cerr << "***ERROR*** Raster::SetPixelValue   *** ERROR ***,  Not a Color Raster."  << std::endl;
  }

  if  (channel == ColorChannels::Red)
    red  [row][col]  = pixVal;
  else
    blue [row][col] = pixVal;

}  /* SetPixelValue  */



bool  Raster::AreThereEdgePixels (kkint32 edgeWidth)
{
  for  (kkint32 edgeIdx = 0;  edgeIdx < edgeWidth;  ++edgeIdx)
  {
    kkint32  topRow = edgeIdx;
    kkint32  botRow = height - (edgeIdx + 1);

    for  (int c = 0;  c < width;  ++c)
    {
      if  (ForegroundPixel (topRow, c)  ||  ForegroundPixel (botRow, c))
        return true;
    }

    kkint32  leftCol = edgeIdx;
    kkint32  rightCol = width - (1 + edgeIdx);

    for  (int r = 0;  r < height;  ++r)
    {
      if  (ForegroundPixel (r, leftCol)  ||  ForegroundPixel (r, rightCol))
        return true;
    }
  }
  return  false;
}  /* AreThereEdgePixels */



void  Raster::DrawGrid (float              pixelsPerMinor,
                        kkuint32           minorsPerMajor,
                        const PixelValue&  hashColor,
                        const PixelValue&  gridColor
                       )
{
  int  x = 0;

  int  hashLen = 4;

  // Horizontal Hash Marks

  //kkint32  horzOffset = (width - (kkint32)(((float)width / pixelsPerMinor) * pixelsPerMinor) / 2);
  while  (true)
  {
    int  hashPos = (int)((float)x * pixelsPerMinor + 0.5f); // + horzOffset;
    if  (hashPos >= (height - hashLen))
      break;

    if  ((x % minorsPerMajor) == 0)
      hashLen = 10;
    else
      hashLen = 6;

    FillRectangle (hashPos,     0,               hashPos + 2, hashLen - 1, hashColor);
    FillRectangle (hashPos,     width - hashLen, hashPos + 2, width - 1,   hashColor);
    DrawLine      (hashPos + 1, 0,               hashPos + 1, width - 1,   gridColor, 0.2f);
    x++;
  }

  x = 0;

  // Vertical Hash Marks
  // kkint32  vertOffset = (height - (kkint32)((height / pixelsPerMinor) * pixelsPerMinor) / 2);
  while  (true)
  {
    int  hashPos = (int)((float)x * pixelsPerMinor + 0.5f); // + vertOffset;
    if  (hashPos >= width)
      break;

    if  ((x % minorsPerMajor) == 0)
      hashLen = 8;
    else
      hashLen = 4;

    FillRectangle (0,                 hashPos,     hashLen - 1, hashPos + 2, hashColor);
    FillRectangle (height - hashLen,  hashPos,     height - 1,  hashPos + 2, hashColor);
    DrawLine      (0,                 hashPos + 1, height - 1,  hashPos + 1, gridColor, 0.2f);

    x++;
  }
}  /* DrawGrid */





inline
void Raster::CalcDialatedValue (kkint32 row,
                                kkint32 col,
                                kkint32&  totVal,
                                uchar&  numNeighbors
                               )  const
{
  if  ((row < 0)  ||  (row >= height))
    return;

  if  ((col < 0)  ||  (col >= width))
    return;

  if  (ForegroundPixel (green[row][col]))
  {
    numNeighbors++;
    totVal = totVal + green[row][col];
  }
}  /* CalcDialatedValue */




RasterPtr  Raster::CreateDilatedRaster ()  const
{
  kkint32  row;
  kkint32  col;

  kkint32  resultForegroundPixelCount = 0;

  RasterPtr  result = AllocateARasterInstance (*this);

  uchar** resultRows = result->green;

  kkint32  totValue     = 0;
  uchar  numNeighbors = 0;

  uchar*  resultRow = NULL;

  for  (row = 0; row < height; row++)
  {
    resultRow = resultRows[row];
    for  (col = 0; col < width; col++)
    {
      if  (BackgroundPixel (resultRow[col]))
      {
        // Since we are a blank cell we want our value to be set to the average value of our occupied neighbors.
        totValue     = 0;
        numNeighbors = 0;

        CalcDialatedValue (row - 1, col - 1, totValue, numNeighbors);
        CalcDialatedValue (row - 1, col,     totValue, numNeighbors);
        CalcDialatedValue (row - 1, col + 1, totValue, numNeighbors);

        CalcDialatedValue (row    , col - 1, totValue, numNeighbors);
        CalcDialatedValue (row    , col + 1, totValue, numNeighbors);

        CalcDialatedValue (row + 1, col - 1, totValue, numNeighbors);
        CalcDialatedValue (row + 1, col    , totValue, numNeighbors);
        CalcDialatedValue (row + 1, col + 1, totValue, numNeighbors);

        if  (numNeighbors > 0)
        {
          resultRow[col] = (uchar)(totValue / numNeighbors);
          resultForegroundPixelCount++;
        }
      }
      else
      {
        resultForegroundPixelCount++;
      }
    }
  }

  result->foregroundPixelCount = resultForegroundPixelCount;
  return  result;
}  /* CreateDilatedRaster */




void  Raster::Dilation ()
{
  RasterPtr  tempRaster = CreateDilatedRaster ();

  delete  greenArea;
  delete  green;

  greenArea = tempRaster->greenArea;
  green     = tempRaster->green;
 
  tempRaster->greenArea = NULL;
  tempRaster->green     = NULL;

  foregroundPixelCount = tempRaster->foregroundPixelCount;

  delete  tempRaster;
}  /* Dilation */

  


RasterPtr  Raster::CreateDilatedRaster (MaskTypes  mask)  const
{
  kkint32 row;
  kkint32 col;

  RasterPtr  result = AllocateARasterInstance (*this);

  uchar** resultRows = result->green;

  //uchar  numNeighbors = 0;

  //for  (row = 1; row < (height - 1); row++)
  //{
  //  for  (col = 1; col < (width - 1); col++)
  //  {
  //    if  (resultRows[row][col] == 0)
  //    {
  //      resultRows[row][col] = Hit (mask, row, col);
  //    }
  //  }
  //}

  kkint32  resultForegroundPixelCount = 0;
  uchar*  resultRow = NULL;
  uchar   pixelValue;
  for  (row = 0; row < height; row++)
  {
    resultRow = resultRows[row];
    for  (col = 0; col < width; col++)
    {
      if  (BackgroundPixel (resultRow[col]))
      {
        pixelValue = Hit (mask, row, col);
        resultRow[col] = pixelValue;
        if  (!BackgroundPixel (pixelValue))
          resultForegroundPixelCount++;
      }
      else
      {
        resultForegroundPixelCount++;
      }
    }
  }

  result->foregroundPixelCount = resultForegroundPixelCount;

  return  result;
}  /* CreateDilatedRaster */





void  Raster::Dilation (MaskTypes  mask)
{    
  RasterPtr tempRaster = CreateDilatedRaster (mask);

  delete  greenArea;
  delete  green;

  foregroundPixelCount = tempRaster->foregroundPixelCount;

  greenArea = tempRaster->greenArea;
  green     = tempRaster->green;

  tempRaster->greenArea = NULL;
  tempRaster->green     = NULL;

  delete  tempRaster;
} /* Dilation */




void  Raster::Dilation (MorphOp::StructureType  _structure,
                        kkuint16                _structureSize,
                        kkint32                 _foregroundCountTH
                       )
{
  MorphOpDilation  dialator (_structure, _structureSize);
  dialator.ForegroundCountTH (_foregroundCountTH);
  RasterPtr tempRaster = dialator.PerformOperation (this);

  delete  greenArea;
  delete  green;

  foregroundPixelCount = tempRaster->ForegroundPixelCount ();

  greenArea = tempRaster->greenArea;
  green     = tempRaster->green;

  tempRaster->greenArea = NULL;
  tempRaster->green     = NULL;

  delete  tempRaster;
} /* Dilation */






void  Raster::Dilation (RasterPtr  dest)  const
{
  if  ((dest->Height () != height)  ||  (dest->Width () != width)  ||  (dest->Color ()  != color))
    dest->ReSize (height, width, color);

  uchar**  srcRows  = Green     ();

  uchar*   destArea = dest->GreenArea ();
  uchar**  destRows = dest->Green     ();

  memset (destArea, 0, totPixels);
  kkint32  pixelCount = 0;

  int  c, r;

  //  Take care of Top and Bottom rows.
  {
    uchar*  srcRow0    = srcRows[0];
    uchar*  srcRow1    = srcRows[1];
    uchar*  srcRowBot0 = srcRows[height - 1];
    uchar*  srcRowBot1 = srcRows[height - 2];

    uchar*  destRow0 = destRows[0];
    uchar*  destRowBot = destRows[height - 1];
    for (c = 1;  c < (width  - 1);  ++c)
    {
      if  ((srcRow0[c - 1] > backgroundPixelTH)  ||  (srcRow0[c] > backgroundPixelTH)  ||  (srcRow0[c + 1] > backgroundPixelTH)  ||
           (srcRow1[c - 1] > backgroundPixelTH)  ||  (srcRow1[c] > backgroundPixelTH)  ||  (srcRow1[c + 1] > backgroundPixelTH)
          )
      {
        destRow0[c] = 255;
        ++pixelCount;
      }

      if  ((srcRowBot0[c - 1] > backgroundPixelTH)  ||                                           (srcRowBot0[c + 1] > backgroundPixelTH)  ||
           (srcRowBot1[c - 1] > backgroundPixelTH)  ||  (srcRowBot1[c] > backgroundPixelTH)  ||  (srcRowBot1[c + 1] > backgroundPixelTH)
          )
      {
        destRowBot[c] = 255;
        ++pixelCount;
      }
    }
  }


  // Take care of left and right columns
  {
    for  (r = 1;  r < (height - 1);  ++r)
    {
      if  ((srcRows[r - 1][0] > backgroundPixelTH)  ||  (srcRows[r - 1][1] > backgroundPixelTH)  ||
           (srcRows[r    ][0] > backgroundPixelTH)  ||  (srcRows[r    ][1] > backgroundPixelTH)  ||
           (srcRows[r + 1][0] > backgroundPixelTH)  ||  (srcRows[r + 1][1] > backgroundPixelTH)
          )
      {
        destRows[r][0] = 255;
        ++pixelCount;
      }


      if  ((srcRows[r - 1][width - 1] > backgroundPixelTH)  ||  (srcRows[r - 1][width - 2] > backgroundPixelTH)  ||
           (srcRows[r    ][width - 1] > backgroundPixelTH)  ||  (srcRows[r    ][width - 2] > backgroundPixelTH)  ||
           (srcRows[r + 1][width - 1] > backgroundPixelTH)  ||  (srcRows[r + 1][width - 2] > backgroundPixelTH)
          )
      {
        destRows[r][width - 1] = 255;
        ++pixelCount;
      }
    }
  }


  // Take care of main Body
  {
    for  (r = 1;  r < (height - 1);  ++r)
    {
      uchar*  srcRow0 = srcRows[r - 1];
      uchar*  srcRow1 = srcRows[r    ];
      uchar*  srcRow2 = srcRows[r + 1];

      for  (c = 1;  c < (width - 1);  ++c)
      {
        if  ((srcRow0[c - 1] > backgroundPixelTH)  ||  (srcRow0[c] > backgroundPixelTH)  ||  (srcRow0[c + 1] > backgroundPixelTH)  ||
             (srcRow1[c - 1] > backgroundPixelTH)  ||  (srcRow1[c] > backgroundPixelTH)  ||  (srcRow1[c + 1] > backgroundPixelTH)  ||
             (srcRow2[c - 1] > backgroundPixelTH)  ||  (srcRow2[c] > backgroundPixelTH)  ||  (srcRow2[c + 1] > backgroundPixelTH)
            )
        {
          destRows[r][c] = 255;
          ++pixelCount;
        }
      }
    }
  }

  dest->foregroundPixelCount = pixelCount;
}  /* Dilation */





void  Raster::Dilation (RasterPtr  dest,
                        MaskTypes  mask
                       )
                          const
{
  if  ((dest->Height () != height)  ||  (dest->Width () != width)  ||  (dest->Color ()  != color))
    dest->ReSize (height, width, color);

  uchar*   destArea = dest->GreenArea ();
  uchar**  destRows = dest->Green     ();

  memset (destArea, 0, totPixels);
  kkint32  pixelCount = 0;

  int  c, r;


  // Take care of main Body
  {
    for  (r = 0;  r < height;  ++r)
    {
      for  (c = 0;  c < width;  ++c)
      {
        if  (IsThereANeighbor (mask, r, c)) 
        {
          destRows[r][c] = 255;
          ++pixelCount;
        }
      }
    }
  }

  dest->foregroundPixelCount = pixelCount;
}  /* Dilation */



void   Raster::FillRectangle (kkint32            tlRow,
                              kkint32            tlCol,
                              kkint32            brRow,
                              kkint32            brCol,
                              const PixelValue&  fillColor
                             )
{
  tlRow = Min (tlRow, height - 1);
  brRow = Min (brRow, height - 1);

  tlCol = Min (tlCol, width - 1);
  brCol = Min (brCol, width - 1);

  for  (kkint32 row = tlRow;  row <= brRow;  ++row)
  {
    if  (color)
    {
      uchar*  rowRed   = red   [row];
      uchar*  rowGreen = green [row];
      uchar*  rowBlue  = blue  [row];
      for  (kkint32 col = tlCol;  col <= brCol;  ++col)
      {
        rowRed  [col] = fillColor.r;
        rowGreen[col] = fillColor.g;
        rowBlue [col] = fillColor.b;
      }
    }
    else
    {
      uchar*  rowGreen = green[row];
      for  (kkint32 col = tlCol;  col <= brCol;  ++col)
        rowGreen[col] = fillColor.g;
    }
  }
}  /* FillRectangle */




void  Raster::FillHoleGrow (kkint32  _row, 
                            kkint32  _col
                           )
{
  PointList  expandList (true);

  expandList.PushOnBack (new Point (_row, _col));
  
  while  (expandList.QueueSize () > 0)
  {
    PointPtr  nextPixel = expandList.PopFromFront ();
  
    kkint32  r = nextPixel->Row ();
    kkint32  c = nextPixel->Col ();
 
    green[r][c] = foregroundPixelValue;

    if  (r > 0)
    {
      if  (BackgroundPixel (green[r - 1][c]))
      {
        green[r - 1][c] = foregroundPixelValue;
        expandList.PushOnBack (new Point (r - 1, c));
      }
    }

    if  (r < (height - 1))
    {
      if  (BackgroundPixel (green[r + 1][c]))
      {
        green[r + 1][c] = foregroundPixelValue;
        expandList.PushOnBack (new Point (r + 1, c));
      }
    }

    if  (c > 0)
    {
      if  (BackgroundPixel (green[r][c - 1]))
      {
        green[r][c - 1] = foregroundPixelValue;
        expandList.PushOnBack (new Point (r, c - 1));
      }
    }

    if  (c < (width - 1))
    {
      if  (BackgroundPixel (green[r][c + 1]))
      {
        green[r][c + 1] = foregroundPixelValue;
        expandList.PushOnBack (new Point (r, c + 1));
      }
    }

    delete  nextPixel;  nextPixel = NULL;
  }
}  /* FillHoleGrow */



RasterPtr  Raster::CreateFillHole () const  {
  auto holesFilledIn = this->AllocateARasterInstance (*this);
  holesFilledIn->FillHole ();
  return holesFilledIn;
}


void  Raster::FillHole ()
{
  kkint32  r;
  kkint32  c;
  Raster  mask (*this);

  uchar**  maskRows = mask.green;

  kkint32  lastRow = height - 1;
  kkint32  lastCol = width  - 1;

  for  (c = 0; c < width; c++)
  {
    if  (BackgroundPixel (maskRows[0][c]))
      mask.FillHoleGrow (0, c);

    if  (BackgroundPixel (maskRows[lastRow][c]))
      mask.FillHoleGrow (lastRow, c);
  }

  for  (r = 0; r < height; r++)     
  {
    if  (BackgroundPixel (maskRows[r][0]))
      mask.FillHoleGrow (r, 0);

    if  (BackgroundPixel (maskRows[r][lastCol]))
      mask.FillHoleGrow (r, lastCol);
  }

  // Now that we know what pixels are background that are connected to one of the boarders,  any other white pixel
  // must be in a hole inside the image.
  foregroundPixelCount = 0;
  uchar*  curRow = NULL;
  for  (r = 0; r < height; r++)
  {
    curRow = green[r];
    for  (c = 0; c < width; c++)
    {
      if  (BackgroundPixel (curRow[c]))
      {
        if  (BackgroundPixel (maskRows[r][c]))
        {
          curRow[c] = foregroundPixelValue;
          foregroundPixelCount++;
        }
      }
      else
      {
        foregroundPixelCount++;
      }
    }
  }
  
  foregroundPixelCount = 0;
}  /* FillHole */





void  Raster::FillHole (RasterPtr  mask)
{
  if  ((mask->Height () != height)  |  (mask->Width () != width))
    mask->ReSize (height, width, false);

  uchar*   srcArea = this->GreenArea ();

  uchar*   maskArea = mask->GreenArea ();
  uchar**  maskRows = mask->Green ();

  memset (maskArea, 0, totPixels);

  kkint32 c, r, x;

  for  (x = 0;  x < totPixels;  ++x)
  {
    if  (srcArea[x] > backgroundPixelTH)
      maskArea[x] = 255;
  }

  {
    // Check Top and Bottom Mask Rows for background pixels and flag them as having access to the border.
    uchar*  rowTop = maskRows[0];
    uchar*  rowBot = maskRows[height - 1];

    for  (c = 0;  c < width;  ++c)
    {
      if  (rowTop[c] == 0)
        rowTop[c] = 1;
      if  (rowBot[c] == 0)
        rowBot[c] = 1;
    }
  }

  {
    // Check Left and Right columns for background pixels and flag them as having access to the border.
    uchar*  leftCol  = maskArea;
    uchar*  rightCol = maskArea + (width - 1);
    for  (r = 0;  r < height;  ++r)
    {
      if  (*leftCol == 0)
        *leftCol = 1;
      if  (*rightCol == 0)
        *rightCol = 1;

      leftCol  += width;
      rightCol += width;
    }
  }


  // We will not iteratively scan the Mask image for pixels that have access top the edge of the image.
  // We will repeat the following loop until no pixels get flagged.
  bool  fillInFound = false;
  do
  {
    fillInFound = false;

    // Scan from TopLeft  to  BotRight
    uchar*  rowPrev  = maskArea;
    uchar*  rowCur   = rowPrev + width;
    uchar*  rowNext  = rowCur  + width;

    for  (r = 1;  r < (height - 1);  ++r)
    {
      for  (c = 1;  c < (width - 1);  ++c)
      {
        if  (rowCur[c] == 0)
        {
          if  ((rowPrev[c    ] == 1)  ||  
               (rowCur [c - 1] == 1)  ||
               (rowCur [c + 1] == 1)  ||
               (rowNext[c    ] == 1)
              )
          {
            rowCur[c] = 1;
            fillInFound = true;
          }
        }
      }

      rowPrev += width;
      rowCur  += width;
      rowNext += width;
    }


    // Scan from Bot-Right  to  Top-Left
    rowPrev  = maskRows[height - 1];
    rowCur   = rowPrev - width;
    rowNext  = rowCur  - width;

    for  (r = (height - 2);  r > 0;  --r)
    {
      for  (c = (width - 2);  c > 0;  --c)
      {
        if  (rowCur[c] == 0)
        {
          if  ((rowPrev[c    ] == 1)  ||  
               (rowCur [c - 1] == 1)  ||
               (rowCur [c + 1] == 1)  ||
               (rowNext[c    ] == 1)
              )
          {
            rowCur[c] = 1;
            fillInFound = true;
          }
        }
      }

      rowPrev -= width;
      rowCur  -= width;
      rowNext -= width;
    }
  }  while  (fillInFound);


  // At this point the only pixels in the mask image that contain a '0' are the ones that are in holes.
  // We will now fill the corresponding pixel locations in the original image with the ForegroundPixelValue.
  for  (x = 0;  x < totPixels;  ++x)
  {
    if  (maskArea[x] == 0)
    {
      srcArea[x] = foregroundPixelValue;
      ++foregroundPixelCount;
    }
  }
}  /* FillHole */





void  Raster::Erosion ()
{
  Raster  origRaster (*this);

  uchar**  origRows = origRaster.green;

  foregroundPixelCount = 0;

  kkint32  r;
  kkint32  c;

  kkint32  lastRow = height - 1;
  kkint32  lastCol = width - 1;

  uchar*  rowLast = NULL;
  uchar*  rowCur  = NULL;
  uchar*  rowNext = NULL;

  for  (r = 1; r < lastRow; r++)
  {
    rowLast = origRows[r - 1];
    rowCur  = origRows[r    ];
    rowNext = origRows[r + 1];

    for  (c = 1; c < lastCol; c++)
    {
      if  (!BackgroundPixel (green[r][c]))
      {
        if  ((rowLast [c - 1] <= backgroundPixelTH)  ||
             (rowLast [c]     <= backgroundPixelTH)  ||
             (rowLast [c + 1] <= backgroundPixelTH)  ||

             (rowCur  [c - 1] <= backgroundPixelTH)  ||
             (rowCur  [c + 1] <= backgroundPixelTH)  ||

             (rowNext [c - 1] <= backgroundPixelTH)  ||
             (rowNext [c]     <= backgroundPixelTH)  ||
             (rowNext [c + 1] <= backgroundPixelTH))
        {
          green[r][c] = backgroundPixelValue;
        }
        else
        {
          foregroundPixelCount++;
        }
      }
    }
  }
}  /* Erosion */




void  Raster::Erosion (MaskTypes  mask)
{
  kkint32  r;
  kkint32  c;

  kkint32  bias = MorphOp::Biases (mask);

  kkint32  maskRowStart = 0 - bias;
  kkint32  maskRowEnd   = 0 + bias;
  kkint32  maskColStart = 0 - bias;
  kkint32  maskColEnd   = 0 + bias;
  kkint32  maskRow;
  kkint32  maskCol;
  bool   fit = false;

  foregroundPixelCount = 0;
  Raster  tempRaster (*this);
  uchar**     tempGreen = tempRaster.Green ();
  uchar*      tempRowData = NULL;
  StructureType  m = MorphOp::MaskShapes (mask);

  for  (r = 0;  r < height;  r++)
  {
    maskColStart = 0 - bias;
    maskColEnd   = 0 + bias;

    tempRowData = tempGreen[r];

    for  (c = 0; c < width; c++)
    {
      if  (ForegroundPixel (green[r][c]))
      {
        fit = true;
        if  ((maskRowStart < 0)  || (maskRowEnd >= height)  ||  (maskColStart < 0) ||  (maskColStart >= width))
          fit = false;

        else if  (m == StructureType::stSquare)
        {
          for  (maskRow = maskRowStart;  ((maskRow <= maskRowEnd)  &&  fit);  maskRow++)
          {
            tempRowData =  tempGreen[maskRow];
            for  (maskCol = maskColStart;  maskCol <= maskColEnd;  maskCol++)
            {
              if  (BackgroundPixel (tempRowData[maskCol]))
              {
                fit = false;
                break;
              }
            }
          }
        }
        else
        {
          //  Cross Structure
          for  (maskRow = maskRowStart;  maskRow <= maskRowEnd;  maskRow++)
          {
            if  (BackgroundPixel (tempGreen[maskRow][c]))
            {
              fit = false;
              break;
            }
          }

          tempRowData =  tempGreen[maskRow];
          for  (maskCol = maskColStart;  maskCol <= maskColEnd;  maskCol++)
          {
            if  (BackgroundPixel (tempRowData[maskCol]))
            {
              fit = false;
              break;
            }
          }
        }

        if  (!fit)
          green[r][c] = backgroundPixelValue;
        else
          foregroundPixelCount++;
      }

      maskColStart++;
      maskColEnd++;
    }   /* End of for(c) */

    maskRowStart++;
    maskRowEnd++;
  }   /* End of for(r) */

}  /* Erosion */





void  Raster::Erosion (MorphOp::StructureType  _structure,
                       kkuint16                _structureSize,
                       kkint32                 _backgroundCountTH
                      )
{
  MorphOpErosion  eroder (_structure, _structureSize);
  eroder.BackgroundCountTH (_backgroundCountTH);
  RasterPtr tempRaster = eroder.PerformOperation (this);
  delete  greenArea;
  delete  green;
  foregroundPixelCount = tempRaster->ForegroundPixelCount ();

  greenArea = tempRaster->greenArea;
  green     = tempRaster->green;

  tempRaster->greenArea = NULL;
  tempRaster->green     = NULL;

  delete  tempRaster;
} /* Erosion */



void  Raster::Erosion (RasterPtr  dest)  const
{
  if  ((dest->Height () != height)  |  (dest->Width () != width))
    dest->ReSize (height, width, false);

  uchar*   destArea = dest->GreenArea ();
  uchar**  destRows = dest->Green ();

  uchar**  srcRows = green;

  memset (destArea, 0, totPixels);
  kkint32  pixelCount = 0;

  int  c, r;

  //  Take care of Top and Bottom rows.
  {
    uchar*  srcRow0         = srcRows[0];
    uchar*  srcRow1         = srcRows[1];
    uchar*  srcRowBot0      = srcRows[height - 1];
    uchar*  srcRowBot1      = srcRows[height - 2];

    uchar*  destRow0 = destRows[0];
    uchar*  destRowBot = destRows[height - 1];
    for (c = 1;  c < (width  - 1);  ++c)
    {
      if  ((srcRow0[c - 1] > backgroundPixelTH)  &&  (srcRow0[c] > backgroundPixelTH)  &&  (srcRow0[c + 1] > backgroundPixelTH)  &&
           (srcRow1[c - 1] > backgroundPixelTH)  &&  (srcRow1[c] > backgroundPixelTH)  &&  (srcRow1[c + 1] > backgroundPixelTH)
          )
      {
        destRow0[c] = 255;
        ++pixelCount;
      }

      if  ((srcRowBot0[c - 1] > backgroundPixelTH)  &&                                           (srcRowBot0[c + 1] > backgroundPixelTH)  &&
           (srcRowBot1[c - 1] > backgroundPixelTH)  &&  (srcRowBot1[c] > backgroundPixelTH)  &&  (srcRowBot1[c + 1] > backgroundPixelTH)
          )
      {
        destRowBot[c] = 255;
        ++pixelCount;
      }
    }
  }


  // Take care of left and right columns
  {
    for  (r = 1;  r < (height - 1);  ++r)
    {
      if  ((srcRows[r - 1][0] > backgroundPixelTH)  &&  (srcRows[r - 1][1] > backgroundPixelTH)  &&
           (srcRows[r    ][0] > backgroundPixelTH)  &&  (srcRows[r    ][1] > backgroundPixelTH)  &&
           (srcRows[r + 1][0] > backgroundPixelTH)  &&  (srcRows[r + 1][1] > backgroundPixelTH)
          )
      {
        destRows[r][0] = 255;
        ++pixelCount;
      }


      if  ((srcRows[r - 1][width - 1] > backgroundPixelTH)  &&  (srcRows[r - 1][width - 2] > backgroundPixelTH)  &&
           (srcRows[r    ][width - 1] > backgroundPixelTH)  &&  (srcRows[r    ][width - 2] > backgroundPixelTH)  &&
           (srcRows[r + 1][width - 1] > backgroundPixelTH)  &&  (srcRows[r + 1][width - 2] > backgroundPixelTH)
          )
      {
        destRows[r][width - 1] = 255;
        ++pixelCount;
      }
    }
  }


  // Take care of main Body
  {
    for  (r = 1;  r < (height - 1);  ++r)
    {
      uchar*  srcRow0 = srcRows[r - 1];
      uchar*  srcRow1 = srcRows[r    ];
      uchar*  srcRow2 = srcRows[r + 1];

      for  (c = 1;  c < (width - 1);  ++c)
      {
        if  ((srcRow0[c - 1] > backgroundPixelTH)  &&  (srcRow0[c] > backgroundPixelTH)  &&  (srcRow0[c + 1] > backgroundPixelTH)  &&
             (srcRow1[c - 1] > backgroundPixelTH)  &&  (srcRow1[c] > backgroundPixelTH)  &&  (srcRow1[c + 1] > backgroundPixelTH)  &&
             (srcRow2[c - 1] > backgroundPixelTH)  &&  (srcRow2[c] > backgroundPixelTH)  &&  (srcRow2[c + 1] > backgroundPixelTH)
            )
        {
          destRows[r][c] = 255;
          ++pixelCount;
        }
      }
    }
  }

  dest->foregroundPixelCount = pixelCount;
}  /* Erosion*/





void  Raster::Erosion (RasterPtr  dest,
                       MaskTypes  mask
                      )
                        const
{
  if  ((dest->Height () != height)  |  (dest->Width () != width))
    dest->ReSize (height, width, false);

  uchar*   destArea = dest->GreenArea ();
  uchar**  destRows = dest->Green ();

  memset (destArea, 0, totPixels);
  kkint32  pixelCount = 0;

  int  c, r;

  // Take care of main Body
  {
    for  (r = 0;  r < height;  ++r)
    {
      uchar*  rowData = green[r];
      for  (c = 0;  c < width;  ++c)
      {
        if  (ForegroundPixel (rowData[c]))
        {
          if  (Fit (mask, r, c))
          {
            destRows[r][c] = 255;
            ++pixelCount;
          }
        }
      }
    }
  }

  dest->foregroundPixelCount = pixelCount;
}  /* Erosion*/







void  Raster::ErosionChanged (MaskTypes  mask, 
                              kkint32    row, 
                              kkint32    col
                             )
{
  kkint32  r;
  kkint32  c;

  kkint32  bias = MorphOp::Biases (mask);

  kkint32  maskRowStart ;
  kkint32  maskRowEnd  ; 
  kkint32  maskColStart ;
  kkint32  maskColEnd  ;
  maskRowStart = 0 - bias;
  maskRowEnd   = 0+ bias;
  maskColStart = 0 - bias;
  maskColEnd   = 0 + bias;
  
  kkint32  maskRow;
  kkint32  maskCol;
  bool  fit;

  foregroundPixelCount = 0;
  Raster      tempRaster (*this);
  uchar**     tempGreen = tempRaster.Green ();
  uchar*      tempRowData = NULL;
  StructureType  m = MorphOp::MaskShapes (mask);

  for  (r = row- 150;  r < row+150;  r++)
  { 
    if (r<0)
    r =0;
    maskColStart = 0 - bias;
    maskColEnd   = 0 + bias;
    
    tempRowData = tempGreen[r];

    for  (c = col - 10; c < col + 10; c++)
    {
      if  (c < 0)
        c = 0;
      // cout << maskColStart <<" ";
        
      if  (ForegroundPixel (green[r][c]))
      {
        fit = true;
        if  ((maskRowStart <  row - 100)  || 
             (maskRowEnd   >= row + 100)  ||  
             (maskColStart <  col - 10)   ||
             (maskColStart >= col + 10)
            )
        {
          fit = false;
        }

        else if  (m == StructureType::stSquare)
        {
          for  (maskRow = row - 150;  ((maskRow <= row + 150)  &&  fit);  maskRow++)
          {
            tempRowData =  tempGreen[maskRow];
            for  (maskCol = col - 10;  maskCol <= col + 10 ; maskCol++)
            {
              if  (BackgroundPixel (tempRowData[maskCol]))
              {
                fit = false;
                break;
              }
            }
          } 
        }

        else
        {
          //  Cross Structure
          for  (maskRow = row-20;  maskRow <= row+20;  maskRow++)
          {
            if  (BackgroundPixel (tempGreen[maskRow][c]))
            {
              fit = false;
              break;
            }
          }

          tempRowData =  tempGreen[maskRow];
          for  (maskCol = col-20;  maskCol <= col+20;  maskCol++)
          {
            if  (BackgroundPixel (tempRowData[maskCol]))
            {
              fit = false;
              break;
            }
          }
        }

        if  (!fit)
          green[r][c] = backgroundPixelValue;
        else
          foregroundPixelCount++;
      }

      maskColStart++;
      maskColEnd++;
    }   /* End of for(c) */

    maskRowStart++;
    maskRowEnd++;
  }   /* End of for(r) */

}  /* ErosionChanged */



void  Raster::ErosionChanged1 (MaskTypes  mask, 
                               kkint32    row, 
                               kkint32    col
                              )
{
  kkint32  r;
  kkint32  c;

  kkint32  bias = MorphOp::Biases (mask);

  kkint32  maskRowStart = 0;
  kkint32  maskRowEnd   = 0; 
  kkint32  maskColStart = 0;
  kkint32  maskColEnd   = 0;

  maskRowStart = 0 - bias;
  maskRowEnd   = 0 + bias;
  maskColStart = 0 - bias;
  maskColEnd   = 0 + bias;
  
  kkint32  maskRow;
  kkint32  maskCol;
  bool  fit;

  foregroundPixelCount = 0;

  Raster  tempRaster (*this);

  uchar**     tempGreen   = tempRaster.Green ();
  uchar*      tempRowData = NULL;
  StructureType  m = MorphOp::MaskShapes (mask);

  for  (r = row- 20;  r < row+20;  r++)
  { 
    if  (r < 0)
      r =0;
  
    maskColStart = 0 - bias;
    maskColEnd   = 0 + bias;
    
    tempRowData = tempGreen[r];

    for  (c = col - 150; c < col + 150; c++)
    {
      if  (c < 0)
        c=0;
      // cout << maskColStart <<" ";
        
      if  (ForegroundPixel (green[r][c]))
      {
        fit = true;
        if  ((maskRowStart <  row - 50)  || 
             (maskRowEnd   >= row + 50)  ||  
             (maskColStart <  col - 100) ||
             (maskColStart >= col+100)
            )
        {
          fit = false;
        }

        else if  (m == StructureType::stSquare)
        {
          for  (maskRow = row - 20;  ((maskRow <= row + 20)  &&  fit);  maskRow++)
          {
            tempRowData =  tempGreen[maskRow];
            for  (maskCol = col - 150;  maskCol <= col + 150 ; maskCol++)
            {
              if  (BackgroundPixel (tempRowData[maskCol]))
              {
                fit = false;
                break;
              }
            }
          } 
        }

        else
        {
          //  Cross Structure
          for  (maskRow = row-20;  maskRow <= row+20;  maskRow++)
          {
            if  (BackgroundPixel (tempGreen[maskRow][c]))
            {
              fit = false;
              break;
            }
          }

          tempRowData =  tempGreen[maskRow];
          for  (maskCol = col-20;  maskCol <= col+20;  maskCol++)
          {
            if  (BackgroundPixel (tempRowData[maskCol]))
            {
              fit = false;
              break;
            }
          }
        }

        if  (!fit)
          green[r][c] = backgroundPixelValue;
        else
          foregroundPixelCount++;
      }

      maskColStart++;
      maskColEnd++;
    }   /* End of for(c) */

    maskRowStart++;
    maskRowEnd++;
  }   /* End of for(r) */
}  /* ErosionChanged1 */



void  Raster::ErosionBoundary (MaskTypes  mask, 
                               kkint32    blobRowStart,
                               kkint32    blobRowEnd,
                               kkint32    blobColStart,
                               kkint32    blobColEnd
                              )
{
  kkint32  r;
  kkint32  c;

  kkint32  bias = MorphOp::Biases (mask);

  kkint32  maskRowStart = 0 - bias;
  kkint32  maskRowEnd   = 0 + bias;
  kkint32  maskColStart = 0 - bias;
  kkint32  maskColEnd   = 0 + bias;

  kkint32  maskRow = 0;
  kkint32  maskCol = 0;
  bool  fit    = false;

  foregroundPixelCount = 0;
  Raster   tempRaster (*this);

  uchar**  tempGreen   = tempRaster.Green ();
  uchar*   tempRowData = NULL;

  StructureType  m = MorphOp::MaskShapes (mask);

  for  (r = 0;  r < height;  r++)
  {
	  /**@todo  This does not make sense; need to investigate. */
    //if ((r>= blobRowStart+30) && (r<= blobrowend-30))
    // {
    // }
  // else
  // {
    maskColStart = 0 - bias;
    maskColEnd   = 0 + bias;

    tempRowData = tempGreen[r];

    for  (c = 0; c < width; c++)
    {
      if  ((c >= blobColStart + 100)  &&  (c <= blobColEnd - 100))
      {
      }
      else
      {
        if  (ForegroundPixel (green[r][c]))
        {
          fit = true;
          if  ((maskRowStart <  0)       || 
               (maskRowEnd   >= height)  ||  
               (maskColStart <  0)       ||
               (maskColStart >= width)
              )
          {
            fit = false;
          }

          else if  (m == StructureType::stSquare)
          {
            for  (maskRow = maskRowStart;  ((maskRow <= maskRowEnd)  &&  fit);  maskRow++)
            {
              tempRowData =  tempGreen[maskRow];
              for  (maskCol = maskColStart;  maskCol <= maskColEnd;  maskCol++)
              {
                if  (BackgroundPixel (tempRowData[maskCol]))
                {
                  fit = false;
                  break;
                }
              }
            }
          }
          else
          {
            //  Cross Structure
            for  (maskRow = maskRowStart;  maskRow <= maskRowEnd;  maskRow++)
            {
              if  (BackgroundPixel (tempGreen[maskRow][c]))
              {
                fit = false;
                break;
              }
            }

            tempRowData =  tempGreen[maskRow];
            for  (maskCol = maskColStart;  maskCol <= maskColEnd;  maskCol++)
            {
              if  (BackgroundPixel (tempRowData[maskCol]))
              {
                fit = false;
                break;
              }
            }
          }

          if  (!fit)
            green[r][c] = backgroundPixelValue;
          else
            foregroundPixelCount++;
        }

        maskColStart++;
        maskColEnd++;
      }
    }   /* End of for(c) */

    maskRowStart++;
    maskRowEnd++;
    //}
  }   /* End of for(r) */

}  /* ErosionBoundary */




RasterPtr  Raster::CreateErodedImage (MaskTypes  mask)  const
{
  kkint32  r;
  kkint32  c;

  RasterPtr   erodedRaster = AllocateARasterInstance (*this);

  uchar*      srcRow    = NULL;
  uchar**     destGreen = erodedRaster->Green ();
  uchar*      destRow   = NULL;

  kkint32  erodedForegroundPixelCount = foregroundPixelCount;

  for  (r = 0;  r < height;  ++r)
  {
    destRow = destGreen[r];
    srcRow  = green[r];

    for  (c = 0;  c < width;  ++c)
    {
      if  (ForegroundPixel (srcRow[c]))
      {
        if  (!Fit (mask, r, c))
        {
          destRow[c] = backgroundPixelValue;
          erodedForegroundPixelCount--;
        }
      }
    }  /* for (c) */
  }  /* for (r) */

  erodedRaster->foregroundPixelCount = erodedForegroundPixelCount;

  return  erodedRaster;
}  /* Erosion  */



void  Raster::Opening ()
{
  Erosion ();
  Dilation ();
}  /* Opening */



void  Raster::Opening (MaskTypes mask)
{
  Erosion (mask);
  Dilation (mask);
}  /* Open */




void  Raster::Closing ()
{
  Dilation ();
  Erosion ();
}  /* Open */





void  Raster::Closing (MaskTypes mask)
{
  Dilation (mask);
  Erosion (mask);
}  /* Open */





inline
bool  Raster::CompletlyFilled3By3 (kkint32  row, 
                                   kkint32  col
                                  )  const
{
  if  (row > 0)
  {
    if  (col > 0)
    {
      if  (BackgroundPixel (green[row - 1][col - 1]))
        return false;
    }

    if  (BackgroundPixel (green[row - 1][col]))
       return  false;

    if  (col < (width - 1))
    {
      if  (BackgroundPixel (green[row - 1][col + 1]))
        return false;
    }
  }

  if  (col > 0)
  {
    if  (BackgroundPixel (green[row][col - 1]))
      return false;
  }

  if  (BackgroundPixel (green[row][col]))
     return  false;

  if  (col < (width - 1))
  {
    if  (BackgroundPixel (green[row][col + 1]))
        return false;
  }

  if  (row < height - 1)
  {
    if  (col > 0)
    {
      if  (BackgroundPixel (green[row + 1][col - 1]))
        return false;
    }

    if  (BackgroundPixel (green[row + 1][col]))
       return  false;

    if  (row < (width - 1))
    {
      if  (BackgroundPixel (green[row + 1][col + 1]))
        return false;
    }
  }

  return  true;
}  /* CompletlyFilled3By3 */




void  Raster::Edge ()
{
  Raster  orig (*this);
  orig.Edge (this);
}  /*  Edge */




void  Raster::Edge (RasterPtr  dest)
{
  if  ((dest->Height () != height)  |  (dest->Width () != width))
    dest->ReSize (height, width, false);

  kkint32  r;
  kkint32  c;

  kkint32  lastRow = height - 1;
  kkint32  lastCol = width - 1;

  uchar**  origRows = Green ();

  uchar*  origRowLast = NULL;
  uchar*  origRowCur  = NULL;
  uchar*  origRowNext = NULL;
   
  kkint32  pixelCount = 0;

  uchar** destRows = dest->Green ();

  for  (r = 1;  r < lastRow;  ++r)
  {
    origRowLast = origRows[r - 1];
    origRowCur  = origRows[r];
    origRowNext = origRows[r + 1];

    uchar*  destRow = destRows[r];

    for  (c = 1; c < lastCol; c++)
    {
      if  (ForegroundPixel (origRowCur[c]))
      {
        if ((ForegroundPixel (origRowLast [c - 1])) &&
            (ForegroundPixel (origRowLast [c]    )) &&
            (ForegroundPixel (origRowLast [c + 1])) &&
            (ForegroundPixel (origRowCur  [c - 1])) &&
            (ForegroundPixel (origRowCur  [c]    )) &&
            (ForegroundPixel (origRowCur  [c + 1])) &&
            (ForegroundPixel (origRowNext [c - 1])) &&
            (ForegroundPixel (origRowNext [c]    )) &&
            (ForegroundPixel (origRowNext [c + 1]))
           )
        {
          destRow[c] = backgroundPixelValue;
        }
        else
        {
          destRow[c] = foregroundPixelValue;
          ++pixelCount;
        }
      }
    }
  }

  dest->foregroundPixelCount = pixelCount;
}  /*  Edge */





inline
kkint32 Raster::BlobId (kkint32  row,
                      kkint32  col
                     )  const
{
  if  ((row < 0)  ||  (row >= height)  ||
       (col < 0)  ||  (col >= width))
    return  -1;

  if  (!blobIds)
    return -1;

  return  blobIds[row][col];
}



kkint32  Raster::NearestNeighborUpperLeft (kkint32 row,
                                           kkint32 col,
                                           kkint32 dist
                                          )
{
  kkint32  nearestBlob = -1;
  kkint32 c, r, startCol, blobID;

  startCol = col - 1;

  for (r = row - dist;  r < row;  ++r)
  {
    if  (r >= 0)
    {
      for  (c = startCol;  c <= col;  ++c)
      {
        if  (c >= 0)
        {
          blobID = BlobId (r, c);
          if  (blobID > nearestBlob)
            nearestBlob = blobID;
        }
      }
    }

    startCol--;
  }

  return  nearestBlob;
}  /* NearestNeighborUpperLeft */




inline
kkint32  Raster::NearestNeighborUpperRight (kkint32 row,
                                            kkint32 col,
                                            kkint32 dist
                                           )
{
  kkint32  nearestBlob = -1;
  kkint32 r, c, endCol, blobID;

  endCol = col + 1;
  for  (r = row - dist;  r < row;  ++r)
  {
    if  (r >= 0)
    {
      if  (endCol >= width)
        endCol = width - 1;

      for  (c = col + 1;  c <= endCol;  ++c)
      {
        blobID = BlobId (r, c);
        if  (blobID > nearestBlob)
          nearestBlob = blobID;
      }
    }

    endCol++;
  }

  return  nearestBlob;
}  /* NearestNeighborUpperRight */




BlobListPtr   Raster::ExtractBlobs (kkint32  dist)
{
  uchar*   curRow         = NULL;
  kkint32*   curRowBlobIds  = NULL;

  kkint32  col = 2;
  kkint32  row = 2;

  BlobPtr  curBlob    = NULL;
  kkint32  curBlobId  = 0;
  kkint32  nearBlobId = 0;

  kkint32  blankColsInARow = 0;

  AllocateBlobIds ();  

  BlobListPtr blobs = new BlobList (true);

  for  (row = 0;  row < height;  ++row)
  {
    curRow        = green[row];
    curRowBlobIds = blobIds[row];

    curBlob = NULL;

    col = 0;
    while  (col < width)
    {
      if  (ForegroundPixel (curRow[col]))
      {
        blankColsInARow = 0;

        nearBlobId = NearestNeighborUpperLeft (row, col, dist);
        if  (nearBlobId < 0)
          nearBlobId = NearestNeighborUpperRight (row, col, dist);


        if  (curBlob)
        {
          if  (nearBlobId >= 0)
          {
            if  (nearBlobId != curBlobId)
            {
              curBlob = blobs->MergeIntoSingleBlob (curBlob, nearBlobId, blobIds);
              curBlobId = curBlob->Id ();
            }
          }

          curRowBlobIds[col] = curBlobId;
          curBlob->colRight  = Max (curBlob->colRight, col);
          curBlob->rowBot    = Max (curBlob->rowBot,   row);
          curBlob->pixelCount++;
        }
        else
        {
          // No Current Blob
          if  (nearBlobId >= 0)
          {
            curBlob   = blobs->LookUpByBlobId (nearBlobId);
            if  (curBlob)
              curBlobId = curBlob->id;
            else
            {
              // If we get to this point there is something wrong with the 'blobs' data structure; for every blobId
              // specified in blobIds(via nearBlobId) there should be a entry in 'blobs'.
              curBlob = blobs->NewBlob(row, col);
              if  (!curBlob)  throw KKException(" Raster::ExtractBlobs   curBlob == NULL");

              curBlobId = curBlob->id;
            }
          }
          else
          {
            curBlob = blobs->NewBlob (row, col);
            if  (!curBlob)
            {
              KKStr errMsg = "Raster::ExtractBlobs   ***ERROR***   allocation of new 'curBlob' failed.";
              cerr << endl << errMsg << endl << endl;
              throw KKException (errMsg);
            }
            curBlobId = curBlob->id;
          }

          curRowBlobIds[col] = curBlobId;
          curBlob->colLeft   = Min (curBlob->colLeft,  col);
          curBlob->colRight  = Max (curBlob->colRight, col);
          curBlob->rowBot    = Max (curBlob->rowBot,   row);
          curBlob->rowTop    = Min (curBlob->rowTop,   row);
          curBlob->pixelCount++;
        }
      }
      else
      {
        // Background Pixel
        if  (curBlob)
        {
          nearBlobId = NearestNeighborUpperLeft (row, col, dist);

          if  (nearBlobId >= 0)
          {
            if  (nearBlobId != curBlobId)
            {
              curBlob = blobs->MergeIntoSingleBlob (curBlob, nearBlobId, blobIds);
              curBlobId = curBlob->Id ();
            }
          }
        }
        
        blankColsInARow++;
        if  (blankColsInARow > dist)
        {
          curBlob = NULL;
          curBlobId = -1;
        }
      }

      col++;
    }
  }

  return  blobs;
}  /* ExtractBlobs */





/**
 @brief Returns image where each blob is labeled with a different color.
 @details Only useful if 'ExtractBlobs' was performed on this instance.
 */
RasterPtr  Raster::CreateColorWithBlobsLabeldByColor (BlobListPtr  blobs)
{
  RasterPtr  colorImage = AllocateARasterInstance (height, width, true);
  if  (blobs == NULL)
    return colorImage;

  if  (blobIds == NULL)
    return colorImage;

  BlobList::iterator  idx;
  for  (idx = blobs->begin ();  idx != blobs->end ();  ++idx)
  {
    BlobPtr  blob = idx->second;
    kkint32  blobId = blob->Id ();
    kkint32  row = 0, col = 0;

    PixelValue  paintColor;
    switch  (blobId % 8)
    {
    case  0:  paintColor = PixelValue::Red;      break;
    case  1:  paintColor = PixelValue::Green;    break;
    case  2:  paintColor = PixelValue::Blue;     break;
    case  3:  paintColor = PixelValue::Yellow;   break;
    case  4:  paintColor = PixelValue::Orange;   break;
    case  5:  paintColor = PixelValue::Magenta;  break;
    case  6:  paintColor = PixelValue::Purple;   break;
    case  7:  paintColor = PixelValue::Teal;     break;
    }

    kkint32  rowStart = Min (blob->RowTop   (), height - 1);
    kkint32  rowEnd   = Min (blob->RowBot   (), height - 1);
    kkint32  colStart = Min (blob->ColLeft  (), width  - 1);
    kkint32  colEnd   = Min (blob->ColRight (), width  - 1);

    for  (row = rowStart;  row <= rowEnd;  ++row)
    {
      for  (col = colStart;  col <= colEnd;  ++col)
      {
        if  (blobIds[row][col] == blobId)
        {
          colorImage->SetPixelValue (row, col, paintColor);
        }
      }
    }
  }

  return  colorImage;
}  /* CreateColorWithBlobsLabeldByColor*/



RasterPtr  Raster::CreateFromOrginalImageWithSpecifidBlobsOnly (RasterPtr    origImage,
                                                                BlobListPtr  blobs
                                                               )
{
  RasterPtr  resultImage = AllocateARasterInstance (height, width, true);
  if  (blobs == NULL)
    return resultImage;

  if  (blobIds == NULL)
    return resultImage;

  BlobList::iterator  idx;
  for  (idx = blobs->begin ();  idx != blobs->end ();  ++idx)
  {
    BlobPtr  blob = idx->second;
    kkint32  blobId = blob->Id ();
    kkint32  row = 0, col = 0;

    kkint32  rowStart = Min (blob->RowTop   (), height - 1);
    kkint32  rowEnd   = Min (blob->RowBot   (), height - 1);
    kkint32  colStart = Min (blob->ColLeft  (), width  - 1);
    kkint32  colEnd   = Min (blob->ColRight (), width  - 1);

    PixelValue  pixelValue;

    for  (row = rowStart;  row <= rowEnd;  ++row)
    {
      for  (col = colStart;  col <= colEnd;  ++col)
      {
        if  (blobIds[row][col] == blobId)
        {
          origImage->GetPixelValue (row, col, pixelValue);
          resultImage->SetPixelValue (row, col, pixelValue);
        }
      }
    }
  }

  return  resultImage;
}  /* CreateFromOrginalImageWithSpecifidBlobsOnly*/





void  Raster::DeleteExistingBlobIds ()
{
  if  (blobIds != NULL)
  {
    // Already been allocated,  just need to reset then
    for  (kkint32 row = 0; row < height; row++)
    {
      delete  blobIds[row];
      blobIds[row] = NULL;
    }

    delete  blobIds;
    blobIds = NULL;
  }
}  /* DeleteExistingBlobIds */



void  Raster::AllocateBlobIds ()
{
  kkint32  row, col;

  DeleteExistingBlobIds ();

  blobIds = new kkint32*[height];

  for  (row = 0; row < height; row++)
  {
    blobIds[row] = new kkint32[width];
    for  (col = 0; col < width; col++)
    {
      blobIds[row][col] = -1;
    }
  }
}  /* AllocateBlobIds */




void  Raster::ConnectedComponent (uchar connectedComponentDist)
{
  if  (connectedComponentDist < 1)
    connectedComponentDist = 3;

  kkint32  row = 0, col = 0;

  BlobListPtr  blobs = ExtractBlobs (connectedComponentDist);

  BlobPtr largestBlob = blobs->LocateLargestBlob ();
  if  (largestBlob)
  {
    kkint32  blobId = largestBlob->Id ();
     
    uchar*   newImageArea = new uchar[totPixels];
    memset (newImageArea, 0, totPixels);
    uchar*   newImageAreaPtr = newImageArea;

    uchar**  newRows = new uchar*[height];

    for  (row = 0; row < height; row++)
    {
      newRows[row] = newImageAreaPtr;
      
      for  (col = 0; col < width; col++)
      {
        if  (blobIds[row][col] == blobId)
        {
          newRows[row][col] = green[row][col];
        }
      }

      newImageAreaPtr = newImageAreaPtr + width;
    }

    delete  green;
    delete  greenArea;

    greenArea = newImageArea;
    green = newRows;
  }

  for  (row = 0; row < height; row++)
    delete  blobIds[row];
  delete  blobIds;
  blobIds = NULL;

  delete  blobs;  blobs = NULL;

  return;
}  /* ConnectedComponent */





void  Raster::ReduceToMostCompleteBlob (uchar connectedComponentDist)
{
  if  (connectedComponentDist < 1)
    connectedComponentDist = 3;

  kkint32  row = 0, col = 0;

  BlobListPtr  blobs = ExtractBlobs (connectedComponentDist);

  BlobPtr largestBlob = blobs->LocateMostComplete ();
  if  (largestBlob)
  {
    kkint32  blobId = largestBlob->Id ();
     
    uchar*   newImageArea = new uchar[totPixels];
    memset (newImageArea, 0, totPixels);
    uchar*   newImageAreaPtr = newImageArea;

    uchar**  newRows = new uchar*[height];

    for  (row = 0; row < height; row++)
    {
      newRows[row] = newImageAreaPtr;
      
      for  (col = 0; col < width; col++)
      {
        if  (blobIds[row][col] == blobId)
        {
          newRows[row][col] = green[row][col];
        }
      }

      newImageAreaPtr = newImageAreaPtr + width;
    }

    delete  green;
    delete  greenArea;

    greenArea = newImageArea;
    green = newRows;
  }

  for  (row = 0; row < height; row++)
    delete  blobIds[row];
  delete  blobIds;
  blobIds = NULL;

  delete  blobs;  blobs = NULL;

  return;
}  /* ReduceToMostCompleteBlob */




void  Raster::ConnectedComponent8Conected ()
{
  uchar*   curRow         = NULL;
  kkint32*   curRowBlobIds  = NULL;
  kkint32*   prevRowBlobIds = NULL;

  kkint32  col = 2;
  kkint32  row = 2;

  BlobPtr  curBlob    = NULL;
  kkint32  curBlobId  = 0;
  kkint32  nearBlobId = 0;

  kkint32  blankColsInARow = 0;

  // Initialize Blob ID's

  blobIds = new kkint32*[height];

  for  (row = 0; row < height; row++)
  {
    blobIds[row] = new kkint32[width];
    for  (col = 0; col < width; col++)
    {
      blobIds[row][col] = -1;
    }
  }

  BlobListPtr blobs = new BlobList (true);

  for  (row = 1; row < height - 1; row++)
  {
    curRow         = green[row];
    curRowBlobIds  = blobIds[row];
    prevRowBlobIds = blobIds[row - 1];


    curBlob = NULL;

    col = 1;
    while  (col < (width - 1))
    {
      if  (ForegroundPixel (curRow[col]))
      {
        blankColsInARow = 0;

        nearBlobId = Max (prevRowBlobIds[col - 1],
                          prevRowBlobIds[col],
                          prevRowBlobIds[col + 1]
                         );
        if  (curBlob)
        {
          if  (nearBlobId >= 0)
          {
            if  (nearBlobId != curBlobId)
            {
              blobs->MergeBlobIds (curBlob, nearBlobId, blobIds);
            }
          }

          curRowBlobIds[col] = curBlobId;
          curBlob->colRight  = Max (curBlob->colRight, col);
          curBlob->rowBot    = Max (curBlob->rowBot,   row);
          curBlob->pixelCount++;
        }
        else
        {
          // No Current Blob
          if  (nearBlobId >= 0)
          {
            curBlob   = blobs->LookUpByBlobId (nearBlobId);
            curBlobId = curBlob->id;
          }
          else
          {
            curBlob = blobs->NewBlob (row, col);
            curBlobId = curBlob->id;
          }

          curRowBlobIds[col] = curBlobId;
          curBlob->colLeft   = Min  (curBlob->colLeft,  col);
          curBlob->colRight  = Max (curBlob->colRight, col);
          curBlob->rowBot    = Max (curBlob->rowBot,   row);
          curBlob->rowTop    = Min  (curBlob->rowTop,   row);
          curBlob->pixelCount++;
        }
      }
      else
      {
        blankColsInARow++;
        curBlob = NULL;
        curBlobId = -1;
      }

      col++;
    }
  }

  BlobPtr largestBlob = blobs->LocateLargestBlob ();
  if  (largestBlob)
  {
    kkint32  blobId = largestBlob->Id ();
    
    uchar*   newImageArea = new uchar[totPixels];
    memset (newImageArea, 0, totPixels);
    uchar*   newImageAreaPtr = newImageArea;

    uchar**  newRows = new uchar*[height];

    for  (row = 0; row < height; row++)
    {
      newRows[row] = newImageAreaPtr;
      
      for  (col = 0; col < width; col++)
      {
        if  (blobIds[row][col] == blobId)
        {
          newRows[row][col] = green[row][col];
        }
      }

      newImageAreaPtr = newImageAreaPtr + width;
    }

    delete  green;
    delete  greenArea;

    greenArea = newImageArea;
    green = newRows;
  }

  for  (row = 0; row < height; row++)
    delete  blobIds[row];
  delete  blobIds;
  blobIds = NULL;

  delete  blobs;  blobs = NULL;

  return;
}  /* ConnectedComponent8Conected */





RasterPtr  Raster::ExtractABlobTightly (const BlobPtr  blob,
                                        kkint32        padding
                                       )  const
{
  if  (blob == NULL)
    return NULL;

  RasterPtr  blobRaster = AllocateARasterInstance (blob->Height () + 2 * padding, blob->Width () + 2 * padding, color);

  kkint32  blobRow = padding;
  kkint32  blobCol = padding;

  kkint32  row = 0;
  kkint32  col = 0;

  for  (row = blob->rowTop;  row <= blob->rowBot;  row++)
  {
    blobCol = padding;
    for  (col = blob->colLeft;  col <= blob->colRight;  col++)
    {
      if  (blobIds[row][col] == blob->id)
      {
        blobRaster->green[blobRow][blobCol] = green[row][col];
        if  (color)
        {
          blobRaster->red [blobRow][blobCol] = red [row][col];
          blobRaster->blue[blobRow][blobCol] = blue[row][col];
        }
      }
      blobCol++;
    }
    blobRow++;
  }
  return  blobRaster;
}  /* ExtractABlobTightly */




RasterPtr  Raster::ExtractABlob (const BlobPtr  blob)  const
{
  RasterPtr  blobRaster = AllocateARasterInstance (height, width, color);

  kkint32  row;
  kkint32  col;

  for  (row = blob->rowTop;  row <= blob->rowBot;  row++)
  {
    for  (col = blob->colLeft;  col <= blob->colRight;  col++)
    {
      if  (blobIds[row][col] == blob->id)
      {
        blobRaster->green[row][col] = green[row][col];
        if  (color)
        {
          blobRaster->red [row][col] = red [row][col];
          blobRaster->blue[row][col] = blue[row][col];
        }
      }
    }
  }

  return  blobRaster;
}  /* ExtractABlob */



kkint32  Raster::CalcArea ()
{
  kkint32  r, c;

  kkint32  area = 0;

  maxPixVal = 0;

  uchar  pixVal;

  for  (r = 0; r < height; r++)
  {
    for  (c = 0; c < width; c++)
    {
      pixVal = green[r][c];
      
      if  (ForegroundPixel (pixVal))
      {
        area++;

        if  (pixVal > maxPixVal)
          maxPixVal = pixVal;
      }
    }
  }

  foregroundPixelCount = area;

  return  area;
}  /* CalcArea */






void  Raster::CalcAreaAndIntensityHistogramWhite (kkint32&  area,
                                                  kkuint32  intensityHistBuckets[8]
                                                 )
{
  kkint32  c;
  kkint32  r;
  kkint32  x;

  // We first need to determine what background pixels are not part of the image.  We assume
  // that any background pixels that are connected to the border of the image are not part
  // of the image.
  Raster  mask (*this);
  mask.FillHole ();


  area = 0;
  for  (x = 0;  x < 8;  x++)
   intensityHistBuckets[x] = 0;


  // Now that we know what pixels are background that are connected to
  // one of the boarders,  any other white pixel must be in a hole inside
  // the image.
  uchar*  curRow     = NULL;
  uchar*  curMaskRow = NULL;
  uchar   pixVal;
  for  (r = 0; r < height; r++)
  {
    curMaskRow = mask.green[r];
    curRow = green[r];
    for  (c = 0; c < width; c++)
    {
      if  (ForegroundPixel (curMaskRow[c]))
      {
        area++;
        pixVal = curRow[c];
        if  (pixVal > maxPixVal)
          maxPixVal = pixVal;
        intensityHistBuckets[freqHistBucketIdx[pixVal]]++;
      }
    }
  }
}  /* CalcAreaAndIntensityHistogramWhite */




void  Raster::CalcAreaAndIntensityHistogram (kkint32&  area,
                                             kkuint32  intensityHistBuckets[8]
                                            )
                                              const
{
  kkint32  r, c;

  area = 0;

  for  (kkint32 x = 0; x < 8; x++)
    intensityHistBuckets[x] = 0;

  maxPixVal = 0;

  uchar  pixVal;

  for  (r = 0; r < height; r++)
  {
    for  (c = 0; c < width; c++)
    {
      pixVal = green[r][c];
      
      if  (ForegroundPixel (pixVal))
      {
        area++;

        if  (pixVal > maxPixVal)
          maxPixVal = pixVal;

        intensityHistBuckets[freqHistBucketIdx[pixVal]]++;
      }
    }
  }

  foregroundPixelCount = area;
}  /* CalcAreaAndIntensityHistogram */




void   Raster::CalcAreaAndIntensityFeatures (kkint32&  area,
                                             float&    weightedSize,
                                             kkuint32  intensityHistBuckets[8],
                                             kkint32&  areaWithWhiteSpace,
                                             kkuint32  intensityHistBucketsWhiteSpace[8]
                                            )  
                                              const
{
  kkint32  x;

  // We first need to determine what background pixels are not part of the image.  We assume
  // that any background pixels that are connected to the border of the image are not part
  // of the image.
  Raster  mask (*this);
  mask.FillHole ();
  
  long  totalPixelValues = 0;

  area               = 0;
  weightedSize       = 0.0f;
  areaWithWhiteSpace = 0;


  for  (x = 0;  x < 8;  x++)
  {
   intensityHistBuckets[x] = 0;
   intensityHistBucketsWhiteSpace[x] = 0;
  }


  maxPixVal = 0;
  uchar  pixVal;

  uchar*  maskGreenArea = mask.GreenArea ();

  for  (x = 0;  x < totPixels;  x++)
  {
    pixVal = greenArea [x];
    if  (ForegroundPixel (maskGreenArea[x]))
    {
      areaWithWhiteSpace++;
      if  (pixVal > maxPixVal)
        maxPixVal = pixVal;
      intensityHistBucketsWhiteSpace[freqHistBucketIdx[pixVal]]++;
      
      if  (ForegroundPixel (pixVal))
      {
        area++;
        intensityHistBuckets[freqHistBucketIdx[pixVal]]++;
        totalPixelValues += pixVal;
      }
    }
  }

  foregroundPixelCount = area;

  weightedSize = (float)totalPixelValues / (float)maxPixVal;
}  /* CalcAreaAndIntensityFeatures */





void   Raster::CalcAreaAndIntensityFeatures16 (kkint32&  area,
                                               float&    weighedSize,
                                               kkuint32  intensityHistBuckets[16]
                                              )
{
  kkint32  x;

  long  totalPixelValues = 0;

  area               = 0;
  weighedSize        = 0.0f;
  maxPixVal          = 0;

  for  (x = 0;  x < 16;  x++)
  {
   intensityHistBuckets[x] = 0;
  }

  maxPixVal = 0;
  uchar  pixVal;

  for  (x = 0;  x < totPixels;  x++)
  {
    pixVal = greenArea [x];
    if  (pixVal > maxPixVal)
      maxPixVal = pixVal;
    if  (ForegroundPixel (pixVal))
    {
      area++;
      intensityHistBuckets[freqHist16BucketIdx[pixVal]]++;
      totalPixelValues += pixVal;
    }
  }

  foregroundPixelCount = area;

  weighedSize = (float)totalPixelValues / (float)maxPixVal;
}  /* CalcAreaAndIntensityFeatures16 */





void   Raster::CalcAreaAndIntensityFeatures (kkint32&  area,
                                             float&    weightedSize,
                                             kkuint32  intensityHistBuckets[8]
                                            )  
                                              const
{
  kkint32  x;

  long  totalPixelValues = 0;

  area          = 0;
  weightedSize  = 0.0f;

  for  (x = 0;  x < 8;  x++)
   intensityHistBuckets[x] = 0;

  maxPixVal = 0;
  uchar  pixVal;

  for  (x = 0;  x < totPixels;  x++)
  {
    pixVal = greenArea [x];
    if  (pixVal > 0)
    {
      if  (pixVal > maxPixVal)
        maxPixVal = pixVal;
      area++;
      intensityHistBuckets[freqHistBucketIdx[pixVal]]++;
      totalPixelValues += pixVal;
    }
  }

  foregroundPixelCount = area;

  weightedSize = (float)totalPixelValues / (float)255.0f;
}  /* CalcAreaAndIntensityFeatures */






float  Raster::CalcWeightedArea ()  const
{
  kkint32  r, c;

  float  area = 0;

  uchar  pixVal = 0;
  kkint32  maxPixValFound = 0;

  //foregroundPixelCount = 0;

  for  (r = 0; r < height; r++)
  {
    for  (c = 0; c < width; c++)
    {
      pixVal = green[r][c];
      if  (ForegroundPixel (pixVal))
      {
        area = area + (float)pixVal;
        if  (pixVal > maxPixValFound)
          maxPixValFound = pixVal;
      }
    }
  }

  area = area / (float)maxPixValFound;

  return  area;
}  /* CalcWeightedArea */


       

  
/**
 *@brief returns in 'features' the 8 central moments as defined by Hu plus eccentricity in the eight bucket.
 *@details See M. K. Hu, Visual pattern recognition by moment invariants  IRE Trans; Inform. Theory, vol. IT, no. 8, pp. 179�187, 1962.
 *@param[in] features A array with 9 elements (0 through 8) that will receive the 8 central moments as defined by HU plus eccentricity 
 * in the eighth element.
 */
void  Raster::CentralMoments (float  features[9])  const
{
  kkint64  m00, m10, m01;
  Moment (m00, m10, m01);

  centroidCol = (float)m10 / (float)m00;   // Center Col
  centroidRow = (float)m01 / (float)m00;   // Center Row

  foregroundPixelCount = (kkint32)m00;

  float  cm00   = (float)m00;
  float  gamma2 = (float)m00 * (float)m00;
  float  gamma3 = gamma2 * (float)sqrt ((float)m00);
        
  float  cm20 = 0.0;
  float  cm02 = 0.0;
  float  cm11 = 0.0;

  float  cm30 = 0.0;
  float  cm03 = 0.0;
  float  cm12 = 0.0;
  float  cm21 = 0.0;

  {
    kkint32 col;
    kkint32 row;

    float   deltaCol = 0.0f;
    float   deltaRow = 0.0f;

    for  (row = 0;  row < height;  ++row)
    {
      deltaRow = (float)row - centroidRow;
      float  rowPow0 = 1.0;
      float  rowPow1 = deltaRow;
      float  rowPow2 = deltaRow * deltaRow;
      float  rowPow3 = rowPow2  * deltaRow;
      uchar*  rowData = green[row];
      for  (col = 0;  col < width;  ++col)
      {
        if  (rowData[col] > backgroundPixelTH)
        {
          deltaCol = (float)col - centroidCol;
          float  colPow0 = 1.0f;
          float  colPow1 = deltaCol;
          float  colPow2 = deltaCol * deltaCol;
          float  colPow3 = colPow2  * deltaCol;

          cm20 += colPow2 * rowPow0;
          cm02 += colPow0 * rowPow2;
          cm11 += colPow1 * rowPow1;
          cm30 += colPow3 * rowPow0;
          cm03 += colPow0 * rowPow3;

          cm12 += colPow1 * rowPow2;
          cm21 += colPow2 * rowPow1;
        }
      }
    }
  }

  cm20 = cm20 / gamma2;
  cm02 = cm02 / gamma2;
  cm11 = cm11 / gamma2;

  cm30 = cm30 / gamma3;
  cm03 = cm03 / gamma3;
  cm12 = cm12 / gamma3;
  cm21 = cm21 / gamma3;

  features[0] = cm00;

  features[1] = cm20 + cm02;
 
  features[2] = (cm20 - cm02) * (cm20 - cm02) + 
                4.0f * cm11 * cm11;

  features[3] = (cm30 - 3.0f * cm12) * (cm30 - 3.0f * cm12) + 
                (3.0f * cm21 - cm03) * (3.0f * cm21 - cm03);

  features[4] = (cm30 + cm12) * (cm30 + cm12) + (cm21 + cm03) * (cm21 + cm03);

  features[5] = (cm30 - 3.0f * cm12) * (cm30 + cm12) * ((cm30 + cm12) * (cm30 + cm12) - 
                3.0f * (cm21 + cm03) * (cm21 + cm03))  +  (3.0f * cm21 - cm03) * (cm21 + cm03) * 
                (3.0f * (cm30 + cm12) * (cm30 + cm12) - (cm21 + cm03) * (cm21 + cm03));

  features[6] = (cm20 - cm02) * 
                ((cm30 + cm12) * (cm30 + cm12) - (cm21 + cm03) * (cm21 + cm03))
                + 4.0f * cm11 * (cm30 + cm12) * (cm21 + cm03);

  features[7] = (3.0f * cm21 - cm03) * (cm30 + cm12) * 
                ((cm30 + cm12) * (cm30 + cm12) - 3.0f * (cm21 + cm03) * (cm21 + cm03) )
               -(cm30 - 3.0f * cm12) * (cm21 + cm03) * 
               (3.0f * (cm30 + cm12) * (cm30 + cm12) - (cm21 + cm03) * (cm21 + cm03) );

  //added by baishali to calculate eccentricity
  features[8] = (((cm20 - cm02) * (cm20 - cm02)) - 
                (4.0f * cm11 * cm11))/((cm20 + cm02) * (cm20 + cm02));


  return;
}  /* CentralMoments */
    



void  Raster::Moment (kkint64& m00,
                      kkint64& m10,
                      kkint64& m01
                     )  const
{
  m00 = 0;
  m10 = 0;
  m01 = 0;

  kkint32  col;
  kkint32  row;

  for  (row = 0;  row < height;  ++row)
  {
    uchar*  rowData = green[row];
    for  (col = 0;  col < width;  ++col)
    {
      if  (rowData[col] > backgroundPixelTH)
      {
        ++m00;
        m10 = m10 + col;
        m01 = m01 + row;
      }
    }
  }
}  /* Moment */




/**
 *@brief Similar to 'CentralMoments' except each pixel position is weighted by its intensity value.
 *@details See M. K. Hu, Visual pattern recognition by moment invariants IRE Trans; Inform. Theory, vol. IT, no. 8, pp. 179, 187, 1962.
 *@param[in] features A array with 9 elements (0 through 8) that will receive the 8 central moments as defined by HU plus eccentricity 
 * in the eighth element.
 */
void  Raster::CentralMomentsWeighted (float  features[9])  const
{

  float  m00, m10, m01;
  MomentWeighted (m00, m10, m01);

  float ew  = m10 / m00;
  float eh  = m01 / m00;
        
  float cm00   = m00;
  float gamma2 = m00 * m00;
  float gamma3 = gamma2 * (float)sqrt (m00);
        
  float cm20 = 0.0f;
  float cm02 = 0.0f;
  float cm11 = 0.0f;

  float cm30 = 0.0f;
  float cm03 = 0.0f;
  float cm12 = 0.0f;
  float cm21 = 0.0f;

  {
    kkint32  col;
    kkint32  row;
   
    maxPixVal = 0;

    for  (row = 0;  row < height;  ++row)
    {
      uchar*  rowData = green[row];
      float  deltaRow = float(row) - eh;

      float  rowPow0 = 1;
      float  rowPow1 = deltaRow;
      float  rowPow2 = deltaRow * deltaRow;
      float  rowPow3 = rowPow2  * deltaRow;

      for  (col = 0;  col < width;  ++col)
      {
        uchar pv = rowData[col];
        if  (pv > backgroundPixelTH)
        {
          if  (pv > maxPixVal)
            maxPixVal = pv;

          float  deltaCol = float(col) - ew;
          float  colPow0 = 1;
          float  colPow1 = deltaCol;
          float  colPow2 = deltaCol * deltaCol;
          float  colPow3 = colPow2  * deltaCol;

          float  pvFloat = (float)pv;

          cm20 += pvFloat * colPow2 * rowPow0;
          cm02 += pvFloat * colPow0 * rowPow2;
          cm30 += pvFloat * colPow3 * rowPow0;
          cm03 += pvFloat * colPow0 * rowPow3;
          cm12 += pvFloat * colPow1 * rowPow2;
          cm21 += pvFloat * colPow2 * rowPow1;
        }
      }
    }

    cm20 = cm20 / (float)maxPixVal;
    cm02 = cm02 / (float)maxPixVal;
    cm30 = cm30 / (float)maxPixVal;
    cm03 = cm03 / (float)maxPixVal;
    cm12 = cm12 / (float)maxPixVal;
    cm21 = cm21 / (float)maxPixVal;
  }

  cm20 = cm20 / gamma2;
  cm02 = cm02 / gamma2;
  cm11 = cm11 / gamma2;

  cm30 = cm30 / gamma3;
  cm03 = cm03 / gamma3;
  cm12 = cm12 / gamma3;
  cm21 = cm21 / gamma3;

  features[0] = cm00;

  features[1] = cm20 + cm02;
 
  features[2] = (cm20 - cm02) * (cm20 - cm02) + (float)4.0 * cm11 * cm11;

  features[3] = (cm30 - (float)3.0 * cm12) * 
                (cm30 - (float)3.0 * cm12) + 
                ((float)3.0 * cm21 - cm03) * 
                ((float)3.0 * cm21 - cm03);

  features[4] = (cm30 + cm12) * (cm30 + cm12) + (cm21 + cm03) * (cm21 + cm03);

  features[5] = (cm30 - (float)3.0 * cm12) * (cm30 + cm12) * 
                ((cm30 + cm12) * (cm30 + cm12) - 
                (float)3.0 * (cm21 + cm03) * (cm21 + cm03)) +
                ((float)3.0 * cm21 - cm03) * (cm21 + cm03) * 
                ((float)3.0 * (cm30 + cm12) * (cm30 + cm12) - 
                (cm21 + cm03) * (cm21 + cm03));

  features[6] = (cm20 - cm02) * 
                (
                 (cm30 + cm12) * (cm30 + cm12) - 
                 (cm21 + cm03) * (cm21 + cm03)
                ) +
                (float)4.0 * cm11 * (cm30 + cm12) * (cm21 + cm03);

  features[7] = ((float)3.0 * cm21 - cm03) * 
                (cm30 + cm12) * ((cm30 + cm12) * 
                (cm30 + cm12) - (float)3.0 * 
                (cm21 + cm03) * (cm21 + cm03)) -
                (cm30 - (float)3.0 * cm12) * 
                (cm21 + cm03) * ((float)3.0 * 
                (cm30 + cm12) * (cm30 + cm12) - 
                (cm21 + cm03) * (cm21 + cm03));

//added by Baishali to calculate eccentricity
  features[8] = (float)((((cm20 - cm02) * (cm20 - cm02)) - 
                (4.0 * cm11 * cm11))/((cm20 + cm02) * (cm20 + cm02)));

  return;
}  /* CentralMomentsWeighted */






void  Raster::MomentWeighted (float& m00,
                              float& m10,
                              float& m01
                             )  const
{
  m00 = 0.0f;
  m10 = 0.0f;
  m01 = 0.0f;

  kkint64  m00Int = 0;
  kkint64  m10Int = 0;
  kkint64  m01Int = 0;

  kkint32  col;
  kkint32  row;

  maxPixVal = 0;

  for  (row = 0;  row < height;  ++row)
  {
    uchar*  rowData = green[row];
    for  (col = 0;  col < width;  ++col)
    {
      uchar pv = rowData[col];
      if  (pv > backgroundPixelTH)
      {
        m00Int += pv;
        m10Int = m10Int + col * pv;
        m01Int = m01Int + row * pv;

        if  (pv > maxPixVal)
          maxPixVal = pv;
      }
    }
  }
 
  m00 = (float)m00Int / (float)maxPixVal;
  m10 = (float)m10Int / (float)maxPixVal;
  m01 = (float)m01Int / (float)maxPixVal;

  return;
}  /* MomentWeighted */



void    Raster::ComputeCentralMoments (kkint32&  _foregroundPixelCount,
                                       float&    weightedPixelCount,
                                       float     centralMoments[9],
                                       float     centralMomentsWeighted[9]
                                      )  
                                       const
{
  kkint64  m00,  m10,  m01;
  float    mw00, mw10, mw01;
  Moments (m00, m10, m01, mw00, mw10, mw01);

  foregroundPixelCount =  (kkint32)m00;
  _foregroundPixelCount = foregroundPixelCount;

  weightedPixelCount   = mw00;

  centroidCol = (float)m10 / (float)m00;
  centroidRow = (float)m01 / (float)m00;

  float centroidColW  = mw10 / mw00;
  float centroidRowW  = mw01 / mw00;
        
  float  cm00   = (float)m00;
  float  gamma2 = (float)m00 * (float)m00;
  float  gamma3 = gamma2 * (float)sqrt ((float)m00);
        
  float  cm20 = 0.0;
  float  cm02 = 0.0;
  float  cm11 = 0.0;

  float  cm30 = 0.0;
  float  cm03 = 0.0;
  float  cm12 = 0.0;
  float  cm21 = 0.0;

  float cmw00   = mw00;
  float gammaW2 = (float)mw00 * (float)mw00;
  float gammaW3 = (float)gammaW2 * (float)sqrt ((float)mw00);
        
  float cmw20 = 0.0f;
  float cmw02 = 0.0f;
  float cmw11 = 0.0f;

  float cmw30 = 0.0f;
  float cmw03 = 0.0f;
  float cmw12 = 0.0f;
  float cmw21 = 0.0f;

  {
    kkint32  col;
    kkint32  row;

    maxPixVal = 0;

    for  (row = 0;  row < height;  ++row)
    {
      uchar*  rowData = green[row];

      float  deltaRow  = float (row) - centroidRow;
      float  deltaRowW = float (row) - centroidRowW;

      float  rowPow0 = 1.0;
      float  rowPow1 = deltaRow;
      float  rowPow2 = deltaRow * deltaRow;
      float  rowPow3 = rowPow2  * deltaRow;

      float  rowPowW0 = 1;
      float  rowPowW1 = deltaRowW;
      float  rowPowW2 = deltaRowW * deltaRowW;
      float  rowPowW3 = rowPowW2  * deltaRowW;

      for  (col = 0;  col < width;  ++col)
      {
        uchar pv = rowData[col];
        if  (pv > backgroundPixelTH)
        {
          float  deltaCol = float (col) - centroidCol;

          float  colPow0 = 1.0f;
          float  colPow1 = deltaCol;
          float  colPow2 = deltaCol * deltaCol;
          float  colPow3 = colPow2  * deltaCol;

          cm20 += colPow2 * rowPow0;
          cm02 += colPow0 * rowPow2;
          cm11 += colPow1 * rowPow1;
          cm30 += colPow3 * rowPow0;
          cm03 += colPow0 * rowPow3;

          cm12 += colPow1 * rowPow2;
          cm21 += colPow2 * rowPow1;

          if  (pv > maxPixVal)
            maxPixVal = pv;

          float  deltaColW = float(col) - centroidColW;
          float  colPowW0 = 1.0f;
          float  colPowW1 = deltaColW;
          float  colPowW2 = deltaColW * deltaColW;
          float  colPowW3 = colPowW2  * deltaColW;

          cmw20 += colPowW2 * rowPowW0;
          cmw02 += colPowW0 * rowPowW2;
          cmw30 += colPowW3 * rowPowW0;
          cmw03 += colPowW0 * rowPowW3;
          cmw12 += colPowW1 * rowPowW2;
          cmw21 += colPowW2 * rowPowW1;
        }
      }
    }

    cmw20 = cmw20 / (float)maxPixVal;
    cmw02 = cmw02 / (float)maxPixVal;
    cmw30 = cmw30 / (float)maxPixVal;
    cmw03 = cmw03 / (float)maxPixVal;
    cmw12 = cmw12 / (float)maxPixVal;
    cmw21 = cmw21 / (float)maxPixVal;
  }

  cm20  = cm20  / gamma2;
  cm02  = cm02  / gamma2;
  cm11  = cm11  / gamma2;

  cm30  = cm30  / gamma3;
  cm03  = cm03  / gamma3;
  cm12  = cm12  / gamma3;
  cm21  = cm21  / gamma3;

  cmw20 = cmw20 / gammaW2;
  cmw02 = cmw02 / gammaW2;
  cmw11 = cmw11 / gammaW2;

  cmw30 = cmw30 / gammaW3;
  cmw03 = cmw03 / gammaW3;
  cmw12 = cmw12 / gammaW3;
  cmw21 = cmw21 / gammaW3;


  centralMoments[0] = cm00;

  centralMoments[1] = cm20 + cm02;
 
  centralMoments[2] = (cm20 - cm02) * (cm20 - cm02) + 
                4.0f * cm11 * cm11;

  centralMoments[3] = (cm30 - 3.0f * cm12) * (cm30 - 3.0f * cm12) + 
                (3.0f * cm21 - cm03) * (3.0f * cm21 - cm03);

  centralMoments[4] = (cm30 + cm12) * (cm30 + cm12) + (cm21 + cm03) * (cm21 + cm03);

  centralMoments[5] = (cm30 - 3.0f * cm12) * (cm30 + cm12) * ((cm30 + cm12) * (cm30 + cm12) - 
                3.0f * (cm21 + cm03) * (cm21 + cm03))  +  (3.0f * cm21 - cm03) * (cm21 + cm03) * 
                (3.0f * (cm30 + cm12) * (cm30 + cm12) - (cm21 + cm03) * (cm21 + cm03));

  centralMoments[6] = (cm20 - cm02) * 
                ((cm30 + cm12) * (cm30 + cm12) - (cm21 + cm03) * (cm21 + cm03))
                + 4.0f * cm11 * (cm30 + cm12) * (cm21 + cm03);

  centralMoments[7] = (3.0f * cm21 - cm03) * (cm30 + cm12) * 
                ((cm30 + cm12) * (cm30 + cm12) - 3.0f * (cm21 + cm03) * (cm21 + cm03) )
               -(cm30 - 3.0f * cm12) * (cm21 + cm03) * 
               (3.0f * (cm30 + cm12) * (cm30 + cm12) - (cm21 + cm03) * (cm21 + cm03) );

  //added by baishali to calculate eccentricity
  centralMoments[8] = (((cm20 - cm02) * (cm20 - cm02)) - 
                (4.0f * cm11 * cm11))/((cm20 + cm02) * (cm20 + cm02));



  centralMomentsWeighted[0] = cmw00;

  centralMomentsWeighted[1] = cmw20 + cmw02;
 
  centralMomentsWeighted[2] = (cmw20 - cmw02) * (cmw20 - cmw02) + (float)4.0 * cmw11 * cmw11;

  centralMomentsWeighted[3] = (cmw30 - (float)3.0 * cmw12) * 
                (cmw30 - (float)3.0 * cmw12) + 
                ((float)3.0 * cmw21 - cmw03) * 
                ((float)3.0 * cmw21 - cmw03);

  centralMomentsWeighted[4] = (cmw30 + cmw12) * (cmw30 + cmw12) + (cmw21 + cmw03) * (cmw21 + cmw03);

  centralMomentsWeighted[5] = (cmw30 - (float)3.0 * cmw12) * (cmw30 + cmw12) * 
                ((cmw30 + cmw12) * (cmw30 + cmw12) - 
                 (float)3.0 * (cmw21 + cmw03) * (cmw21 + cmw03)) +
                ((float)3.0 * cmw21 - cmw03) * (cmw21 + cmw03) * 
                ((float)3.0 * (cmw30 + cmw12) * (cmw30 + cmw12) - 
                (cmw21 + cmw03) * (cmw21 + cmw03));

  centralMomentsWeighted[6] = (cmw20 - cmw02) * 
                (
                 (cmw30 + cmw12) * (cmw30 + cmw12) - 
                 (cmw21 + cmw03) * (cmw21 + cmw03)
                ) +
                (float)4.0 * cmw11 * (cmw30 + cmw12) * (cmw21 + cmw03);

  centralMomentsWeighted[7] = ((float)3.0 * cmw21 - cmw03) * 
                (cmw30 + cmw12) * ((cmw30 + cmw12) * 
                (cmw30 + cmw12) - (float)3.0 * 
                (cmw21 + cmw03) * (cmw21 + cmw03)) -
                (cmw30 - (float)3.0 * cmw12) * 
                (cmw21 + cmw03) * ((float)3.0 * 
                (cmw30 + cmw12) * (cmw30 + cmw12) - 
                (cmw21 + cmw03) * (cmw21 + cmw03));

//added by baishali to calculate eccentricity
  centralMomentsWeighted[8] = (float)((((cmw20 - cmw02) * (cmw20 - cmw02)) - 
                (4.0 * cmw11 * cmw11))/((cmw20 + cmw02) * (cmw20 + cmw02)));

  return;
}  /* CentralMoments */









void  Raster::Moments (kkint64&  m00,
                       kkint64&  m10,
                       kkint64&  m01,
                       float&    mw00,
                       float&    mw10,
                       float&    mw01
                      )  const
{
  m00  = 0;
  m10  = 0;
  m01  = 0;

  mw00 = 0.0f;
  mw10 = 0.0f;
  mw01 = 0.0f;

  kkint64  m00Int = 0;
  kkint64  m10Int = 0;
  kkint64  m01Int = 0;

  kkint32  col = 0;
  kkint32  row = 0;

  maxPixVal = 0;

  for  (row = 0;  row < height;  ++row)
  {
    uchar*  rowData = green[row];
    for  (col = 0;  col < width;  ++col)
    {
      uchar pv = rowData[col];
      if  (pv > backgroundPixelTH)
      {
        m00Int += pv;
        m10Int = m10Int + col * pv;
        m01Int = m01Int + row * pv;

        if  (pv > maxPixVal)
          maxPixVal = pv;

        ++m00;
        m10 = m10 + col;
        m01 = m01 + row;
      }
    }
  }
 
  mw00 = (float)m00Int / (float)maxPixVal;
  mw10 = (float)m10Int / (float)maxPixVal;
  mw01 = (float)m01Int / (float)maxPixVal;

  return;
}  /* Moments */





/**
 *@brief Returns back a two dimension array that is a copy of the specified region in the image.
 *@details The caller will take ownership of the two dimensional array created.
 */
uchar**  Raster::GetSubSet (uchar**  _src,
                            kkint32  _row,
                            kkint32  _col,
                            kkint32  _height,
                            kkint32  _width
                           )  const
{
  kkint32  endR = _row + _height - 1;
  kkint32  endC = _col + _width - 1;

  if  ((_row < 0)  ||  (endR >= height) ||
       (_col < 0)  ||  (endC >= width))
  {
    cerr << "***ERROR***, Raster::Raster  *** ERROR ***,  Index's Exceed Raster Bounds" << std::endl;
    cerr << "       Raster Dimensions["       << width    << ", "  << height << "]."     << std::endl;
    cerr << "       Requested Coordinates [" << _row     << ", "  << _col      << "], "
         << "       Height["  << _height << "],  Width[" << _width << "]."
         << std::endl;
    //WaitForEnter ();
    exit (-1);
  }

  kkint32  row = 0;
  kkint32  col = 0;

  kkint32  totalPixelArea = _height * _width;
  uchar* subSetArea = new uchar[totalPixelArea];

  uchar** subSet = new uchar*[_height];
  
  for  (row = 0; row < _height; row++)
  {
    subSet[row] = subSetArea;
    for  (col = 0; col < _width; col++)
    {
      subSet[row][col] = _src[_row + row][_col + col];
    }
    subSetArea = subSetArea + _width;
  }

  return  subSet;
}  /* GetSubSet */




/**
 *@brief Returns true if all the pixels covered by the specified mask are Foreground pixels.
 *@see  Erosion, Dilation, Closing, Opening, MaskType
 */
bool  Raster::Fit (MaskTypes  mask,
                   kkint32    row, 
                   kkint32    col
                  )  const
{
  kkint32  bias = MorphOp::Biases (mask);
  kkint32  r, c;
  kkint32  rStart = row - bias;
  kkint32  rEnd   = row + bias;
  kkint32  cStart = col - bias;
  kkint32  cEnd   = col + bias;

  if  (rStart  < 0)        rStart = 0;
  if  (rEnd    >= height)  rEnd = height - 1;
  if  (cStart  < 0)        cStart = 0;
  if  (cEnd    >= width)   cEnd = width - 1;

  if  (MorphOp::MaskShapes (mask) == StructureType::stSquare)
  {
    for  (r = rStart;  r <= rEnd;  r++)
    {
      uchar*  rowData = green[r];
      for  (c = cStart;  c <= cEnd;  c++)
      {
        if  (BackgroundPixel (rowData[c]))
          return  false;
      }
    }
  }

  else
  {  
    for  (r = rStart;  r <= rEnd;  r++)
    {
      if  (BackgroundPixel (green[r][col]))
        return  false;
    }

    uchar*  rowData = green[row];
    for  (c = cStart;  c <= cEnd;  c++)
    {
      if  (BackgroundPixel (rowData[c]))
        return  false;
    }
  }

  return  true;
}  /* Fit */






/**
 *@brief Returns true if any one of the pixels covered by the specified mask are Foreground pixels.
 *@see  Erosion, Dilation, Closing, Opening, MaskType
 */
bool  Raster::IsThereANeighbor (MaskTypes  mask,
                                kkint32    row, 
                                kkint32    col
                               )  const
{
  kkint32  bias = MorphOp::Biases (mask);
  kkint32  r, c;
  kkint32  rStart = row - bias;
  kkint32  rEnd   = row + bias;
  kkint32  cStart = col - bias;
  kkint32  cEnd   = col + bias;

  if  (rStart  < 0)        rStart = 0;
  if  (rEnd    >= height)  rEnd = height - 1;
  if  (cStart  < 0)        cStart = 0;
  if  (cEnd    >= width)   cEnd = width - 1;

  if  (MorphOp::MaskShapes (mask) == StructureType::stSquare)
  {
    for  (r = rStart;  r <= rEnd;  r++)
    {
      uchar*  rowData = green[r];
      for  (c = cStart;  c <= cEnd;  c++)
      {
        if  (this->ForegroundPixel (rowData[c]))
          return  true;
      }
    }
  }

  else
  {  
    for  (r = rStart;  r <= rEnd;  r++)
    {
      if  (ForegroundPixel (green[r][col]))
        return  true;
    }

    uchar*  rowData = green[row];
    for  (c = cStart;  c <= cEnd;  c++)
    {
      if  (ForegroundPixel (rowData[c]))
        return  true;
    }
  }

  return  false;
}  /* IsThereANeighbor */




/**
 *@brief  Used by morphological operators to determine the average pixel value of the foreground pixels that the specifies mask covers.
 *@see  Erosion, Dilation, Closing, Opening, MaskType
 */
uchar  Raster::Hit  (MaskTypes  mask,
                     kkint32    row,
                     kkint32    col
                    )  const
{
  kkint32  maskBias = MorphOp::Biases (mask);
  if ((row            <  maskBias)   || 
      (row + maskBias >= height)        ||
      (col            <  maskBias)   || 
      (col + maskBias >= width))
  {
    //return 0;
  }

  uchar    pixVal    = 0;
  kkint32  totPixVal = 0;
  kkint16  numOfHits = 0;

  kkint32  startRow = Max (row - maskBias, (kkint32)0);
  kkint32  endRow   = Min (row + maskBias, height - 1);
  kkint32  startCol = Max (col - maskBias, (kkint32)0);
  kkint32  endCol   = Min (col + maskBias, width - 1);

  kkint32  r, c;

  if  (MorphOp::MaskShapes (mask) == StructureType::stSquare)
  {
    for  (r = startRow; r <= endRow; r++)
    {
      for  (c = startCol; c < endCol; c++)
      {
        pixVal = green[r][c];
 
        if  (ForegroundPixel (pixVal))
        {
          numOfHits++;
          totPixVal = totPixVal + pixVal;
        }
      }
    }
  }

  else
  {
    for  (r = startRow, c = startCol; r <= endRow; r++, c++)
    {
      pixVal = green[r][col];
      if  (ForegroundPixel (pixVal))
      {
        numOfHits++;
        totPixVal = totPixVal + pixVal;
      }

      pixVal = green[row][c];
      if  (ForegroundPixel (pixVal))
      {
        numOfHits++;
        totPixVal = totPixVal + pixVal;
      }
    }
  }


  if  (numOfHits == 0)
    return 0;
  else
    return  (uchar)(totPixVal / numOfHits);
}  /* Hit */




RasterPtr  Raster::FastFourierKK ()  const
{

  KK_DFT2D_Float  plan (height, width, true);
  KK_DFT2D_Float::DftComplexType**  dest;
  KK_DFT2D_Float::DftComplexType*   destArea;
  plan.AllocateArray (destArea, dest);

  if  (destArea == NULL)
  {
    std::cerr 
        << std::endl << std::endl
        << "Raster::FastFourierKK   ***ERROR***    Allocation of 'dest' failed'" << std::endl
        << "              totPixels[" << totPixels << "]"    << std::endl
        << "              FileName[" << fileName   << "]"    << std::endl
        << std::endl;
    return NULL;
  }

  plan.Transform (green, dest);

  RasterPtr fourierImage = AllocateARasterInstance (height, width, false);

  uchar*  destData = fourierImage->greenArea;

  fourierImage->AllocateFourierMagnitudeTable ();
  float* fourierMagArray = fourierImage->fourierMagArea;

  float  mag = 0.0f;

  float  maxAmplitude = 0.0f;

  kkint32 idx = 0;
  for  (kkint32 row = 0; row < height; row++ )
  {
    for (kkint32 col = 0; col < width; col++ )
    {
      double  r = dest[row][col].real ();
      double  i = dest[row][col].imag ();

      mag = (float)(sqrt (r * r + i * i));
      if  (mag > maxAmplitude)
        maxAmplitude = mag;

      fourierMagArray[idx] = mag;  // kk 2004-May-18
      ++idx;
    }
  }

  float  maxAmplitudeLog = log (maxAmplitude);

  idx = 0;
  for (idx = 0; idx < totPixels; idx++ )
  {
    //  mag = (float)sqrt (dest[idx].re * dest[idx].re + dest[idx].im * dest[idx].im);  // kk 2004-May-18
    mag = fourierMagArray[idx];                                                         // kk 2004-May-18

    // destData[idx] = (uchar)(dest[idx].re * maxPixVal / maxAmplitude);

    // kk  2004-May-18
    // Changed the above line to use the constant 255 instead of maxPixVal,  
    // If we have an image who's maxPixVal is less than 255 then the values
    // being calculated for the Fourier features will not be consistent.
    // destData[idx] = (uchar)(dest[idx].re * 255 / maxAmplitude);
    destData[idx] = (uchar)(log (fourierMagArray[idx]) * 255.0f / maxAmplitudeLog);
  }

  delete  dest;      dest     = NULL;
  delete  destArea;  destArea = NULL;

  return  fourierImage;
}  /* FastFourierKK */





RasterPtr  Raster::FastFourier ()  const
{
  #if  defined(FFTW_AVAILABLE)
    fftwf_complex*   src  = NULL;
    fftwf_complex*   dest = NULL;
    fftwf_plan       plan = NULL;
  #else
    KK_DFT2D_Float  plan (height, width, true);

    KK_DFT2D_Float::DftComplexType*   srcArea = NULL;
    KK_DFT2D_Float::DftComplexType**  src     = NULL;

    KK_DFT2D_Float::DftComplexType*   destArea = NULL;
    KK_DFT2D_Float::DftComplexType**  dest     = NULL;
  #endif

  if  (totPixels <= 0)
  {
    cerr << std::endl << std::endl
      << "Raster::FastFourier   ***ERROR***      totPixels == 0." << std::endl
      << "              FileName[" << this->FileName () << "]"    << std::endl
      << std::endl;
    return NULL;
  }

  kkint32  col;
  kkint32  row;

  kkint32  idx = 0;

  #if  defined(FFTW_AVAILABLE)
    src  = (fftwf_complex*)fftwf_malloc (sizeof (fftwf_complex) * totPixels);
    dest = (fftwf_complex*)fftwf_malloc (sizeof (fftwf_complex) * totPixels);
  #else
    plan.AllocateArray (srcArea,  src);
    plan.AllocateArray (destArea, dest);
  #endif

  if  (src == NULL)
  {
    std::cerr 
        << std::endl << std::endl 
        << "Raster::FastFourier   ***ERROR***    Allocation of 'src' failed'" << std::endl
        << "              totPixels[" << totPixels << "]"    << std::endl
        << "              FileName[" << fileName   << "]"    << std::endl
        << std::endl;
    return NULL;
  }

  if  (dest == NULL)
  {
    std::cerr 
        << std::endl << std::endl
        << "Raster::FastFourier   ***ERROR***    Allocation of 'dest' failed'" << std::endl
        << "              totPixels[" << totPixels << "]"    << std::endl
        << "              FileName[" << fileName   << "]"    << std::endl
        << std::endl;
	return NULL;
  }

  // float scalingFact = (float)255.0 / maxPixVal;   // kk  2004-May-18

  for  (row = 0; row < height; row++ )
  {
    for (col = 0; col < width; col++ )
    {     
      // src[idx].re = (float)green[row][col] * scalingFact;
      #if  defined(FFTW_AVAILABLE)
        src[idx][0] = (float)green[row][col];
        src[idx][1] = 0.0;
      #else
        srcArea[idx].real ((float)greenArea[idx]);
        srcArea[idx].imag (0.0);
      #endif
      idx++;
    }
  }
  
  #if  defined(FFTW_AVAILABLE)
    plan = fftwCreateTwoDPlan (height, width, src, dest, FFTW_FORWARD, FFTW_ESTIMATE);
    fftwf_execute (plan);
    fftwDestroyPlan (plan);
  #else
    plan.Transform (src, dest);
  #endif


  RasterPtr fourierImage = new Raster (height, width);

  uchar*  destData = fourierImage->greenArea;

  fourierImage->AllocateFourierMagnitudeTable ();
  float* fourierMagArray = fourierImage->fourierMagArea;

  float  mag = (float)0;

  float  maxAmplitude = (float)0;

  for  (idx = 0; idx < totPixels; idx++)
  {
    #if  defined(FFTW_AVAILABLE)
      float real = dest[idx][0];
      float imag = dest[idx][1];
    #else
      float real = destArea[idx].real ();
      float imag = destArea[idx].imag ();
    #endif

    // mag = (float)log10 (sqrt (real * real + imag * imag));
    mag = (float)(sqrt (real * real + imag * imag));

    if  (mag > maxAmplitude)
      maxAmplitude = mag;

    fourierMagArray[idx] = mag;  // kk 2004-May-18
  }

  float  maxAmplitudeLog = log (maxAmplitude);

  idx = 0;
  for (idx = 0; idx < totPixels; idx++ )
  {
    //  mag = (float)sqrt (dest[idx].re * dest[idx].re + dest[idx].im * dest[idx].im);  // kk 2004-May-18
    mag = fourierMagArray[idx];                                                          // kk 2004-May-18

    // destData[idx] = (uchar)(dest[idx].re * maxPixVal / maxAmplitude);

    // kk  2004-May-18
    // Changed the above line to use the constant 255 instead of maxPixVal,  
    // If we have an image who's maxPixVal is less than 255 then the values
    // being calculated for the Fourier features will not be consistent.
    // destData[idx] = (uchar)(dest[idx].re * 255 / maxAmplitude);
    destData[idx] = (uchar)(log (fourierMagArray[idx]) * 255.0f / maxAmplitudeLog);
  }            

  #if  defined(FFTW_AVAILABLE)
    fftwf_free (src);   src  = NULL;
    fftwf_free (dest);  dest = NULL;
  #else
    plan.DestroyArray (srcArea,  src);
    plan.DestroyArray (destArea, dest);
  #endif

  return  fourierImage;
}  /* FastFourier */




RasterPtr  Raster::SwapQuadrants ()  const
{
  RasterPtr  result = AllocateARasterInstance (*this);

  kkint32  leftStart   = (kkint32)(width  / 2);
  kkint32  bottomStart = (kkint32)(height / 2);

  kkint32  r1, c1;
  kkint32  r2, c2;

  for  (r1 = 0, r2 = bottomStart;  (r1 < bottomStart)  &&  (r2 < height);  r1++, r2++)
  {
    for  (c1 = 0,  c2 = leftStart;  (c1 < leftStart)   &&  (c2 < width);   c1++, c2++)
    {
      result->green[r1][c1] = green[r2][c2];
      result->green[r2][c2] = green[r1][c1];
      result->green[r1][c2] = green[r2][c1];
      result->green[r2][c1] = green[r1][c2];

      if  (result->red)
      {
        result->red[r1][c1] = red[r2][c2];
        result->red[r2][c2] = red[r1][c1];
        result->red[r1][c2] = red[r2][c1];
        result->red[r2][c1] = red[r1][c2];
      }

      if  (result->blue)
      {
        result->blue[r1][c1] = blue[r2][c2];
        result->blue[r2][c2] = blue[r1][c1];
        result->blue[r1][c2] = blue[r2][c1];
        result->blue[r2][c1] = blue[r1][c2];
      }

      if  (result->fourierMag)
      {
        result->fourierMag[r1][c1] = fourierMag[r2][c2];
        result->fourierMag[r2][c2] = fourierMag[r1][c1];
        result->fourierMag[r1][c2] = fourierMag[r2][c1];
        result->fourierMag[r2][c1] = fourierMag[r1][c2];
      }
    }
  }

  return  result;
}  /* SwapQuadrants  */




void  Raster::FourierExtractFeatures (float  fourierFeatures[5])  const
{
  if  (!fourierMagArea)
  {
     cerr << std::endl
          << "*** ERROR ***    This Raster image is not the result of a fast Fourier" << std::endl
          << std::endl;
     osWaitForEnter ();
     exit (-1);
  }

  float  cr = (float)height / (float)2.0;
  float  cw = (float)width  / (float)2.0;

  float  crSqr = cr * cr;
  float  cwSqr = cw * cw;

  //float  diagLen = crSqr + cwSqr;
  float  r1Len = crSqr / (float)4.0 + cwSqr / (float)4.0;

  float  r2Len = (float)((9.0   /  16.0) * crSqr + (9.0   /  16.0) * cwSqr);  //  3/ 4 ths Len

  float  r3Len = (float)((81.0  / 100.0) * crSqr + (81.0  / 100.0) * cwSqr);  //  9/10 ths Len

  float  r4Len = (float)((361.0 / 400.0) * crSqr + (361.0 / 400.0) * cwSqr);  // 19/20 ths Len

  fourierFeatures[0] = (float)0.0;
  fourierFeatures[1] = (float)0.0;
  fourierFeatures[2] = (float)0.0;
  fourierFeatures[3] = (float)0.0;
  fourierFeatures[4] = (float)0.0;

  kkint32  count[5];

  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  count[3] = 0;
  count[4] = 0;

  kkint32  row;
  kkint32  col;

  float  deltaRow;
  float  deltaRowSqr = (float)0.0;

  float  deltaCol;
  float  deltaColSqr = (float)0.0;

  float  distFromCent;

  for  (row = 0 ; row < height; row++)
  {
    deltaRow = cr - (float)row;
    deltaRowSqr = deltaRow * deltaRow;

    for  (col = 0; col < (kkint32)cw; col++)
    {
      deltaCol = cw - (float)col;
      deltaColSqr = deltaCol * deltaCol;
      
      distFromCent = deltaRowSqr + deltaColSqr;

      if  (distFromCent < r1Len)
      {
        fourierFeatures[0] = fourierFeatures[0] + fourierMag[row][col];
        count[0]++;
      }

      else if  (distFromCent < r2Len)
      {
        fourierFeatures[1] = fourierFeatures[1] + fourierMag[row][col];
        count[1]++;
      }
   
      else if  (distFromCent < r3Len)
      {
        fourierFeatures[2] = fourierFeatures[2] + fourierMag[row][col];
        count[2]++;
      }
   
      else if  (distFromCent < r4Len)
      {
        fourierFeatures[3] = fourierFeatures[3] + fourierMag[row][col];
        count[3]++;
      }
      else
      {
        fourierFeatures[4] = fourierFeatures[4] + fourierMag[row][col];
        count[4]++;
      }
    }
  }

  kkint32  x;

  for  (x = 0; x < 5; x++)
  {
    fourierFeatures[x] = fourierFeatures[x] / (float)count[x];
  }
}  /* ExtractFourierFeatures */



/**
 *@brief Returns the difference between two angles in degrees.
 *@details  Given two angles, will determine the angular distance
 * between them, going from the ang1 to ang2.  A positive
 * delta indicates that ang2 is in front of ang1, negative
 * means it is behind ang1.  The result can be from -180
 * to +180 degrees.
 */
float  DeltaAngle (float  ang1,
                   float  ang2
                  )
{
  float  deltaAng = ang2 - ang1;

  while  (deltaAng > 180.0)
    deltaAng  = deltaAng - (float)360.0;

  if  (deltaAng < -180.0)
    deltaAng = deltaAng + (float)360;

  return  deltaAng;
}  /* DeltaAngle */



const 
MovDir   movements[8]  = {{-1,  0}, // Up
                          {-1,  1}, // Up-Right
                          { 0,  1}, // Right
                          { 1,  1}, // Down-Right
                          { 1,  0}, // Down
                          { 1, -1}, // Down-Left
                          { 0, -1}, // Left,
                          {-1, -1}  // Up-Left
                         };


void  Raster::FollowContour (float  countourFreq[5])  const
{
  // startRow and startCol is assumed to come from the left (6)

  kkint32  startRow = 0;
  kkint32  startCol = 0;

  kkint32  lastDir = 1; // Last Movement was Up-Right

  kkint32  nextRow = 0;
  kkint32  nextCol = 0;

  kkint32  fromDir = 0;

  kkint32  edgeLen = 0;

  kkint32  maxNumOfAngles = 2 * height * width;
  kkint32  numOfAngles = 0;


  kkint32  count[5];

  for  (kkint32 x = 0; x < 5; x++)
  {
    countourFreq[x] = 0.0;
    count[x] = 0;
  }



  #if defined(FFTW_AVAILABLE)
    fftw_complex*  src = (fftw_complex*)fftw_malloc (sizeof (fftw_complex) * maxNumOfAngles);
  #else
    KK_DFT1D_Float::DftComplexType*  src = new KK_DFT1D_Float::DftComplexType[maxNumOfAngles];
  #endif

  startRow = height / 2;
  startCol = 0;

  while  ((startCol < width)  &&  (BackgroundPixel (green[startRow][startCol])))
  {
    startCol++;
  }

  // if  (green[startRow][startCol] == 0)
  if  (startCol >= width)
  {
    // We did not find a Pixel in the Middle Row(height / 2) so now we will
    // scan the image row, by row, Left to Right until we find an occupied 
    // pixel.

    bool  found = false;
    kkint32  row;
    kkint32  col;

    for  (row = 0; ((row < height)  &&  (!found));  row++)
    {
      for  (col = 0; ((col < width)  &&  (!found)); col++)
      {
        if  (ForegroundPixel (green[row][col]))
        {
          found = true;
          startRow = row;
          startCol = col;
        }
      }
    }

    if  (!found)
    {
      return;
    }
  }

  kkint32  curRow = startRow;
  kkint32  curCol = startCol;

  bool  nextPixelFound = false;

  kkint32  nextDir;

  do  
  {
    edgeLen++;

    fromDir = lastDir + 4;
    if  (fromDir > 7)
      fromDir = fromDir - 8;

    nextPixelFound = false;

    nextDir = fromDir + 2;
    while  (!nextPixelFound)
    {
      if  (nextDir > 7)
        nextDir = nextDir - 8;

      nextRow = curRow + movements[nextDir].row;
      nextCol = curCol + movements[nextDir].col;

      if  ((nextRow <  0)       ||  
           (nextRow >= height)  ||
           (nextCol <  0)       ||
           (nextCol >= width) 
          )
      {
        nextDir++;
      }
      else if  (ForegroundPixel (green[nextRow][nextCol]))
      {
        nextPixelFound = true;
      }
      else
      {
        nextDir++;
      }
    }

    if  (numOfAngles >= maxNumOfAngles)
    {
      kkint32  newMaxNumOfAngles = maxNumOfAngles * 2;

      #if  defined(FFTW_AVAILABLE)
        fftw_complex*  newSrc = (fftw_complex*)fftw_malloc (sizeof (fftw_complex) * newMaxNumOfAngles);
      #else
        KK_DFT1D_Float::DftComplexType*  newSrc = new KK_DFT1D_Float::DftComplexType[newMaxNumOfAngles];
      #endif

      for  (kkint32 x = 0; x < maxNumOfAngles; ++x)
      {
        #if  defined(FFTW_AVAILABLE)
          newSrc[x][0] = src[x][0];
          newSrc[x][1] = src[x][1];
        #else
          newSrc[x].real (src[x].real ());
          newSrc[x].imag (src[x].imag ());
        #endif
      }
        
      #ifdef  FFTW3_H
        fftw_free (src);
      #else
        delete src;
      #endif

      src = newSrc;
      maxNumOfAngles = newMaxNumOfAngles;
    }
  
    #if  defined(FFTW_AVAILABLE)
      src[numOfAngles][0] = nextRow; 
      src[numOfAngles][1] = nextCol;
    #else
      src[numOfAngles].real ((float)nextRow);
      src[numOfAngles].imag ((float)nextCol);
    #endif


    // sqrt ((float)(nextRow * nextRow + nextCol * nextCol));
    numOfAngles++;
 
    lastDir = nextDir;
    curRow = nextRow;
    curCol = nextCol;
  }  while  ((curRow != startRow)  ||  (curCol != startCol));


  #if  defined(FFTW_AVAILABLE)
    fftw_complex*   dest = (fftw_complex*)fftw_malloc (sizeof (fftw_complex) * maxNumOfAngles);
    fftw_plan       plan;
    plan = fftw_plan_dft_1d (numOfAngles, src, dest, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute (plan);
    fftw_destroy_plan(plan);  
  #else
    KK_DFT1D_Float  plan(numOfAngles, true);
    KK_DFT1D_Float::DftComplexType*  dest = new KK_DFT1D_Float::DftComplexType[maxNumOfAngles];
    plan.Transform(src, dest);
  #endif


  float  middle = (float)(numOfAngles / 2.0);
  float  r1 = (float)(middle / 2.0);
  float  r2 = (float)(middle * ( 3.0  /   4.0));
  float  r3 = (float)(middle * ( 7.0  /   8.0));
  float  r4 = (float)(middle * (15.0  /  16.0));

  float  deltaX;
  float  mag;

  for  (kkint32 x = 0; x < numOfAngles; x++)
  {
    #if  defined(FFTW_AVAILABLE)
      mag = (float)log (sqrt (dest[x][0] * dest[x][0] + dest[x][1] * dest[x][1]));
    #else
      mag = (float)log (sqrt (dest[x].real () * dest[x].real () + dest[x].imag () * dest[x].imag ()));
    #endif

    deltaX = (float)fabs ((float)x - middle);

    if  (deltaX < r1)
    {
      countourFreq[0] = countourFreq[0] + mag;
      count[0]++;
    }

    else if  (deltaX < r2)
    {
      countourFreq[1] = countourFreq[1] + mag;
      count[1]++;
    }

    else if  (deltaX < r3)
    {
      countourFreq[2] = countourFreq[2] + mag;
      count[2]++;
    }

    else if  (deltaX < r4)
    {
      countourFreq[3] = countourFreq[3] + mag;
      count[3]++;
    }

    else
    {
      countourFreq[4] = countourFreq[4] + mag;
      count[4]++;
    }
  }


  for  (kkint32 x = 0; x < 5; x++)
    countourFreq[x] = countourFreq[x] / (float)count[x];

  #if  defined(FFTW_AVAILABLE)
    fftw_free (src);
    fftw_free (dest);
  #else
    delete[]  src;
    delete[]  dest;
  #endif
}  /* FollowContour */







void  Raster::CalcOrientationAndEigerRatio (float&  eigenRatio,
                                            float&  orientationAngle
                                           )  
{
  float  centroidColWeighted;
  float  centroidRowWeighted;

  kkint32  size;
  kkint32  weight;

  CalcCentroid (size, weight, centroidRow, centroidCol, centroidRowWeighted, centroidColWeighted);

  double  cov[2][2];

  cov[0][0] = 0.0;
  cov[0][1] = 0.0;
  cov[1][0] = 0.0;
  cov[1][1] = 0.0;

  kkint32  col;
  uchar    pixVal;
  kkint32  row;

  double  colOffset = 0.0;
  double  rowOffset = 0.0;
  double  rowOffsetSquare = 0.0;

  for  (row = 0; row < height;  row++)
  {
    rowOffset = double (row) - centroidRow;
    rowOffsetSquare = rowOffset * rowOffset;

    for  (col = 0; col < width; col++)
    {
      pixVal = green[row][col];

      if  (ForegroundPixel (pixVal))
      {
        colOffset = double (col) - centroidCol;
        cov[0][0] += (colOffset * colOffset);
        cov[1][1] += (rowOffsetSquare);
        cov[0][1] += (colOffset * rowOffset);
      }
    }
  }

  cov[1][0] = cov[0][1];
  cov[0][0] /= size;
  cov[0][1] /= size;
  cov[1][0] /= size;
  cov[1][1] /= size;

  double  origCov00 = cov[0][0];
  double  origCov11 = cov[1][1];
  double  origCov10 = cov[1][0];

  double  d[2];
  double  e[2];

  d[0] = 0.0;
  d[1] = 0.0;
  e[0] = 0.0;
  e[1] = 0.0;

  Tred2 (2, cov, d, e);

  tqli (2, d, e, cov);

  double  ang0;

  if  (fabs (d[0]) > fabs (d[1]))
  {
    ang0 = atan2 (cov[0][0], cov[1][0]);
    eigenRatio = (d[0] == 0.0) ? 9999.0f : float (d[1] / d[0]);
  }
  else
  {
    ang0 = atan2 (cov[0][1], cov[1][1]);
    eigenRatio = (d[1] == 0.0) ? 9999.0f : float (d[0] / d[1]);
  }
  //double  ang1 = RadToDeg * atan2 (cov[0][1], cov[1][1]);

  orientationAngle = float (-ang0 - 1.57079632679f);
  if  (fabs (orientationAngle) > 1.57079632679f)
  {
    if  (orientationAngle < 0.0)
      orientationAngle += float (PIE);
    else
      orientationAngle -= float (PIE);
  }


  //if  (d[0] == 0.0)
  //  eigenRatio = (float)9999.0;
  //else
  //  eigenRatio = (float) (d[1] / d[0]);


  if  (origCov10 == 0)
  {
    // Appears we have a image that is standing up Straight, 
    // but may be fatter than it is tall.
    if  (origCov00 > origCov11)
    {
      orientationAngle = 0.0f;
    }
    else
    {
      orientationAngle = 1.57079632679f;
    }
  }

  return;
} /* CalcOrientation */




RasterPtr  Raster::Rotate (float  turnAngle)
{

  kkint32  diag = (kkint32)sqrt ((float)(Height () * Height () + Width () * Width ())) + 10;
  kkint32  halfDiag = (diag + 1) / 2;

  //  J( x+width(J)/2, y+height(J)/2 )= I( A11x+A12y+b1, A21x+A22y+b2 ),
  //  C:\Program Files\OpenCV\docs\ref\OpenCVRef_ImageProcessing.htm
  //      Example. Using cvGetQuadrangeSubPix for image rotation


  RasterPtr  rotatedImage = AllocateARasterInstance (diag, diag, false);

  float  a11 = (float)(cos (-turnAngle));
  float  a12 = (float)(sin (-turnAngle));
  float  b1  = (float)width  * 0.5f;

  float  a21 = -a12;
  float  a22 = a11;
  float  b2  = (float)height * 0.5f;

  kkint32  centDestCol = 0;
  kkint32  centDestRow = 0;

  kkint32  srcCol;
  kkint32  srcRow;

  kkint32  destX;
  kkint32  destY;


  for  (centDestRow = -halfDiag; centDestRow <  halfDiag; centDestRow++)
  {
    for  (centDestCol = -halfDiag; centDestCol <  halfDiag; centDestCol++)
    {
      srcRow = (kkint32)((float)(a21 * (float)centDestCol) + (float)(a22 * (float)centDestRow) + b2 + 0.5);
      if  ((srcRow >= 0)  &&  (srcRow < height))
      {
        srcCol = (kkint32)((float)(a11 * (float)centDestCol) + (float)(a12 * (float)centDestRow) + b1 + 0.5);
   
        if  ((srcCol >= 0)  &&  (srcCol < width))
        {
          uchar  pixVal = GetPixelValue (srcRow, srcCol);
          if  (pixVal > 0)
          {
            destY = centDestRow + halfDiag;
            destX = centDestCol + halfDiag;
            rotatedImage->SetPixelValue (destY, destX, pixVal);
          }
        }
      }
    }
  }

  return  rotatedImage;
}  /* Rotate */




Point  Raster::RotateDerivePreRotatedPoint (kkint32  height,
                                            kkint32  width,
                                            Point&   rotatedPoint, 
                                            float    turnAngle
                                           )
                                           const
{
  kkint32  diag = (kkint32)sqrt ((float)(height * height + width * width)) + 10;

  float  a11 = (float)(cos (-turnAngle));
  float  a12 = (float)(sin (-turnAngle));
  float  b1  = (float)width  * 0.5f;

  float  a21 = -a12;
  float  a22 = a11;
  float  b2  = (float)height * 0.5f;

  kkint32  halfDiag = (diag + 1) / 2;

  kkint32  centDestRow = rotatedPoint.Row () - halfDiag;
  kkint32  centDestCol = rotatedPoint.Col () - halfDiag;

  kkint32  srcY = (kkint32)((float)(a21 * centDestCol) + (a22 * (float)centDestRow) + b2 + 0.5f);
  kkint32  srcX = (kkint32)((float)(a11 * centDestCol) + (a12 * (float)centDestRow) + b1 + 0.5f);

  return  Point (srcY, srcX);
}  /* RotateDerivePreRotatedPoint */




void  Raster::FindBoundingBox (kkint32&  tlRow,
                               kkint32&  tlCol,
                               kkint32&  brRow,
                               kkint32&  brCol
                              )  const
{
  tlRow = INT_MAX;
  tlCol = INT_MAX;

  brRow = INT_MIN;
  brCol = INT_MIN;

  kkint32  col = 0;
  kkint32  row = 0;

  bool  firstPixelFound = false;

  // lets 1st find the very 1st Pixel Used.
  while  (row < height) 
  {
    while  (col < width)
    {
      if  (ForegroundPixel (green[row][col]))
      {
        tlRow = row;
        tlCol = col;
        brRow = row;
        brCol = col;
        firstPixelFound = true;
        break;
      }

      col++;
    }

    if  (firstPixelFound)
      break;
 
    row++; col = 0;
  }


  if  (!firstPixelFound)
  {
    // We have a Blank Image.
    tlRow = tlCol = brRow = brCol = -1;
    return;
  }

  while  (row < height)
  {
    // Lets find 1st Pixel used in Row.
    while  ((col < width)  &&  (BackgroundPixel (green[row][col])))
    {
      col++;
    }
 
    if  (col < width)
    {
      // We have some Data on this row.

      brRow = row;

      if  (col < tlCol)
        tlCol = col;

      kkint32  lastColUsed = 0;

      while  (col < width)
      {
        if  (ForegroundPixel (green[row][col]))
        {
          lastColUsed = col;
        }

        col++;
      }

      if  (lastColUsed > brCol)
        brCol = lastColUsed;
    }

    row++;  col = 0;
  }

  return;

}  /* FindBoundingBox */  


uchar  Raster::DeltaMagnitude (uchar c1, uchar c2)
{
  if  (c1 > c2)
    return (uchar)(c1 - c2);
  else
    return (uchar)(c2 - c1);
}




RasterPtr   Raster::FindMagnitudeDifferences (const Raster&  r)
{
  kkint32  resultHeight = Max (height, r.Height ());
  kkint32  resultWidth  = Max (width,  r.Width  ());

  RasterPtr  result = new Raster (resultHeight, resultWidth, color);

  uchar**  otherRed   = r.Red   ();
  uchar**  otherGreen = r.Green ();
  uchar**  otherBlue  = r.Blue  ();

  uchar**  resultRed   = result->Red   ();
  uchar**  resultGreen = result->Green ();
  uchar**  resultBlue  = result->Blue  ();

  kkint32  minHeight = Min (height, r.Height ());
  kkint32  minWidth  = Min (width,  r.Width  ());

  kkint32  resultTotPixels = resultHeight * resultWidth;

  memset (result->GreenArea (), 255, resultTotPixels);
  if  (color)
  {
    memset (result->RedArea  (), 255, resultTotPixels);
    memset (result->BlueArea (), 255, resultTotPixels);
  }

  for  (kkint32  row = 0;  row < minHeight;  ++row)
  {
    for  (kkint32  c = 0;  c < minWidth;  ++c)
    {
      // resultGreen[r][c] = DeltaMagnitude (green[r][c], otherGreen[r][c]);
      int  deltaC = DeltaMagnitude (green[row][c], otherGreen[row][c]);
      if  (deltaC > 0)
        deltaC = Min (255, deltaC + 64);
      resultGreen[row][c] = deltaC;
      
      if  (color)
      {
        resultRed [row][c] = DeltaMagnitude (red [row][c], otherRed [row][c]);
        resultBlue[row][c] = DeltaMagnitude (blue[row][c], otherBlue[row][c]);
      }
    }
  }

  return  result;
}  /* FindMagnitudeDifferences */






void  Raster::CalcCentroid (kkint32&  size,
                            kkint32&  weight,
                            float&  rowCenter,  
                            float&  colCenter,
                            float&  rowCenterWeighted,
                            float&  colCenterWeighted
                           )  const
{
  size              = 0;
  weight            = 0;
  rowCenter         = 0;
  colCenter         = 0;
  rowCenterWeighted = 0;
  colCenterWeighted = 0;

  kkint32  row;  
  kkint32  col;

  uchar  intensity;

  for  (row = 0; row < height;  row++)
  {
    for  (col = 0; col < width; col++)
    {
      intensity = green[row][col];

      if  (intensity > 0)
      {
        rowCenter += row;
        colCenter += col;
        size++;

        rowCenterWeighted += row * intensity;
        colCenterWeighted += col * intensity;
        weight += intensity;
      }
    }
  }

  rowCenter /= size;
  colCenter /= size; 

  this->centroidCol = colCenter;
  this->centroidRow = rowCenter;
  
  rowCenterWeighted /= weight;
  colCenterWeighted /= weight; 
}  /* CalcCentroid */



RasterPtr  Raster::CreateColor ()  const
{
  if  (color)
    return new Raster (*this);

  RasterPtr  colorImage = new Raster (height, width, true);

  uint  x = 0;
  PixelValue  pv[256];
  for  (x = 0;  x < 256;  ++x)
  {
    pv[x] = PixelValue::FromHSI ((float)x, 1.0f, 1.0f);
  }

  for  (int r = 0;  r < height;  ++r)
  {
    uchar*  rowData = green[r];
    for  (int c = 0;  c < width;  ++c)
    {
      uchar  intensity = rowData[c];
      if  (intensity == 0)
        colorImage->SetPixelValue (r, c, PixelValue::White);
      else
        colorImage->SetPixelValue (r, c, pv[rowData[c]]);
    }
  }

  return   colorImage;
}



RasterPtr  Raster::CreateGrayScale ()  const
{
  if  (!color)
  {
    // Already a gray-scale image,  Just return copy of self.
    return  (AllocateARasterInstance (*this));
  }

  kkint32  h = height;
  kkint32  w = width;

  RasterPtr  grayRaster = AllocateARasterInstance (h, w, false);
  uchar* grayPtr  = grayRaster->GreenArea ();
  uchar* redPtr   = redArea;
  uchar* greenPtr = greenArea;
  uchar* bluePtr  = blueArea;

  for  (kkint32 x = 0;  x < totPixels;  ++x, ++grayPtr, ++redPtr, ++greenPtr, ++bluePtr)
  {
    *grayPtr = (uchar)((kkuint32)((0.30f) * (float)(*redPtr)    +  
                              (0.59f) * (float)(*greenPtr)  +  
                              (0.11f) * (float)(*bluePtr)
                             )
                      );
  }
  return  grayRaster;
}  /* CreateGrayScale */




HistogramPtr  Raster::HistogramGrayscale ()  const
{
  HistogramPtr histogram = new KKB::Histogram (0, 256, 1.0, false);

  kkint32  row, col;
 
  for  (row = 0;  row < height;  row++)
  {
    for  (col = 0;  col < width;  col++)
    {
      histogram->Increment (green[row][col]);
    }
  }

  return  histogram;
}  /* HistogramGrayscale */



HistogramPtr  Raster::Histogram (ColorChannels  channel)  const
{
  uchar*  data = NULL;
  if  (color)
  {
    switch  (channel)
    {
    case  ColorChannels::Red:   data = redArea;    break;
    case  ColorChannels::Green: data = greenArea;  break;
    case  ColorChannels::Blue:  data = blueArea;   break;
    }
  }

  if  (!data)
  {
    KKStr  errMsg (128);
    errMsg << "Raster::Histogram   ***ERROR***    channel: " << ColorChannelToKKStr (channel) << "  Is not defined.";
    cerr << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  HistogramPtr histogram = new KKB::Histogram (0, 256, 1.0, false);
  for  (kkint32 x = 0;  x < totPixels;  x++)
  {
    histogram->Increment (*data);
    ++data;
  }

  return  histogram;
}  /* Histogram */



RasterPtr  Raster::HistogramEqualizedImage ()  const
{
  HistogramPtr  grayScaleHistogram = HistogramGrayscale ();
  HistogramPtr  equalizedHistogram = grayScaleHistogram->Equalized ();
  delete  grayScaleHistogram;
  grayScaleHistogram = NULL;
  RasterPtr equalizedImage = HistogramEqualizedImage (equalizedHistogram);
  delete  equalizedHistogram;

  return  equalizedImage;
}  /* HistogramEqualizedImage */





RasterPtr  Raster::HistogramEqualizedImage (HistogramPtr  equalizedHistogram)  const
{
  if  (equalizedHistogram->NumOfBuckets () != 256)
  {
    cerr << std::endl
         << "**** ERROR ****    Histogram Does not have 256 Buckets[" 
         << equalizedHistogram->NumOfBuckets () << "]"  << std::endl
         << std::endl;
    osWaitForEnter ();
    exit (-1);
  }


  kkint32*  equalizedMapTable = equalizedHistogram->EqualizedMapTable ();

  if  (!equalizedMapTable)
  {
    cerr << std::endl
         << "**** ERROR ****    Histogram Does not have EqualizeMapTable."  << std::endl
         << std::endl;
    osWaitForEnter ();
    exit (-1);
  }

  RasterPtr  equalizedImage = AllocateARasterInstance (height, width, false);

  uchar**  dest = equalizedImage->Rows ();

  kkint32  row, col;

  for  (row = 0;  row < height;  row++) 
  {
    for  (col = 0;  col < width;  col++)
    {
      dest[row][col] = (uchar)(equalizedMapTable[green[row][col]]);
    }
  }

  return  equalizedImage;
}  /* HistogramEqualizedImage */




RasterPtr  Raster::HistogramGrayscaleImage ()  const
{
  HistogramPtr histogram = NULL;

  if  (color)
  {
    RasterPtr  gsImage = this->CreateGrayScale ();
    histogram = gsImage->HistogramGrayscale ();
    delete  gsImage;
    gsImage = NULL;
  }
  else
  {
    histogram = HistogramGrayscale ();
  }

  RasterPtr histogramImage = histogram->CreateGraph ();
  delete histogram;
  histogram = NULL;
  return  histogramImage;
}  /* HistogramGrayscaleImage */



RasterPtr  Raster::HistogramImage (ColorChannels  channel)  const
{
  HistogramPtr histogram = Histogram (channel);
  RasterPtr histogramImage = histogram->CreateGraph ();
  delete histogram;
  histogram = NULL;
  return  histogramImage;
}  /* HistogramImage */




void  Raster::DrawLine (kkint32 bpRow,    kkint32 bpCol,
                        kkint32 epRow,    kkint32 epCol
                       )
{
  DrawLine (bpRow, bpCol,  
            epRow, epCol,
            255
           );
}





void  Raster::DrawLine (kkint32 bpRow,    kkint32 bpCol,
                        kkint32 epRow,    kkint32 epCol,
                        uchar pixelVal
                       )
{
  DrawLine (bpRow, bpCol, epRow, epCol, pixelVal, pixelVal, pixelVal);
}  /* DrawLine */




void  Raster::DrawLine (kkint32 bpRow,    kkint32 bpCol,
                        kkint32 epRow,    kkint32 epCol,
                        uchar  r,
                        uchar  g,
                        uchar  b
                       )
{

  if  ((bpRow < 0)  ||  (bpRow >= height)  ||
       (bpCol < 0)  ||  (bpCol >= width)   ||
       (epRow < 0)  ||  (epRow >= height)  ||
       (epCol < 0)  ||  (epCol >= width)
      )
  {
    cerr << std::endl
         << "*** WARNING ***"
         << "Raster::DrawLine,  Out of Raster Boundaries   Height[" << height << "]  width[" << width << "]."  << std::endl
         << "                   BeginPoint" << Point (bpRow, bpCol) << "   EndPoint" << Point (epRow, epCol) << "." << std::endl
         << std::endl;
    return;
  }

  kkint32  deltaY = epRow - bpRow;
  kkint32  deltaX = epCol - bpCol;

  kkint32  row, col;

  if  (deltaY == 0)
  {
    // We have a Straight Horizontal Line

    row = bpRow;
    
    kkint32  startCol;
    kkint32  endCol;

    if  (bpCol < epCol)
    {
      startCol = bpCol;
      endCol   = epCol;
    }
    else
    {
      startCol = epCol;
      endCol   = bpCol;
    }

    for  (col = startCol;  col <= endCol;  col++)
    {
      green[row][col] = g;
      if  (color)
      {
        red [row][col] = r;
        blue[row][col] = b;
      }
    }

    return;
  }

  if  (deltaX == 0)
  {
    // We have a Straight Vertical Line

    col = bpCol;
    
    kkint32  startRow;
    kkint32  endRow;

    if  (bpRow < epRow)
    {
      startRow = bpRow;
      endRow   = epRow;
    }
    else
    {
      startRow = epRow;
      endRow   = bpRow;
    }

    for  (row = startRow;  row <= endRow;  row++)
    {
      green[row][col] = g;
      if  (color)
      {
        red [row][col] = r;
        blue[row][col] = b;
      }
    }

    return;
  }

  float  m = (float)deltaY / (float)deltaX;

  float  c = bpRow - m * bpCol;

  if  (fabs (m) < 0.5)
  {
    kkint32  startCol;
    kkint32  endCol;

    if  (bpCol < epCol)
    {
      startCol = bpCol;
      endCol   = epCol;
    }
    else
    {
      startCol = epCol;
      endCol   = bpCol;
    }

    for  (col = startCol;  col <= endCol;  col++)
    {
      row = (kkint32)(m * col + c + 0.5);
      green[row][col] = g;
      if  (color)
      {
        red [row][col] = r;
        blue[row][col] = b;
      }
    }
  }
  else
  {
    kkint32  startRow;
    kkint32  endRow;

    if  (bpRow < epRow)
    {
      startRow = bpRow;
      endRow   = epRow;
    }
    else
    {
      startRow = epRow;
      endRow   = bpRow;
    }

    for  (row = startRow;  row <= endRow;  row++)
    {
      col = (kkint32)(((row - c) / m) + 0.5);
      green[row][col] = g;
      if  (color)
      {
        red [row][col] = r;
        blue[row][col] = b;
      }
    }
  }
}  /* DrawLine */



uchar  MergeAlpfaBeta (float alpha,
                       uchar alphaPixel,
                       float beta,
                       uchar betaPixel
                      )
{
  int newPixelValue = (int)(0.5f + alpha * (float)alphaPixel  + beta  * (float)betaPixel);
  if  (newPixelValue > 255)
    newPixelValue = 255;
  return (uchar)newPixelValue;
}


void  Raster::PaintPoint (kkint32            row,
                          kkint32            col,
                          const PixelValue&  pv,
                          float              alpha
                         )
{
  if  ((row < 0)  ||  (row >= height)  ||  (col < 0)  ||  (col >= width))
    return;

  float beta = 1.0f - alpha;

  green[row][col] = Min (255, (int)(0.5f + alpha * (float)pv.g  + beta  * (float)green[row][col]));
  if  (color)
  {
    red [row][col] = Min (255, (int)(0.5f + alpha * (float)pv.r  + beta  * (float)red [row][col]));
    blue[row][col] = Min (255, (int)(0.5f + alpha * (float)pv.b  + beta  * (float)blue[row][col]));
  }
}



void  Raster::PaintFatPoint (kkint32           row,
                             kkint32           col,
                             const PixelValue  pv,
                             float             alpha
                            )
{
  PaintPoint (row - 2, col,     pv, alpha);

  PaintPoint (row - 1, col - 1, pv, alpha);
  PaintPoint (row - 1, col,     pv, alpha);
  PaintPoint (row - 1, col + 1, pv, alpha);

  PaintPoint (row,     col - 2, pv, alpha);
  PaintPoint (row,     col - 1, pv, alpha);
  PaintPoint (row,     col,     pv, alpha);
  PaintPoint (row,     col + 1, pv, alpha);
  PaintPoint (row,     col + 2, pv, alpha);

  PaintPoint (row + 1, col - 1, pv, alpha);
  PaintPoint (row + 1, col,     pv, alpha);
  PaintPoint (row + 1, col + 1, pv, alpha);

  PaintPoint (row + 2, col,     pv, alpha);
}


void  Raster::DrawLine (kkint32 bpRow,    kkint32 bpCol,
                        kkint32 epRow,    kkint32 epCol,
                        uchar  r,
                        uchar  g,
                        uchar  b,
                        float  alpha
                       )
{
  float  beta = 1.0f - alpha;

  if  ((bpRow < 0)  ||  (bpRow >= height)  ||
       (bpCol < 0)  ||  (bpCol >= width)   ||
       (epRow < 0)  ||  (epRow >= height)  ||
       (epCol < 0)  ||  (epCol >= width)
      )
  {
    cerr << std::endl
         << "*** WARNING ***"
         << "Raster::DrawLine,  Out of Raster Boundaries   Height[" << height << "]  width[" << width << "]."  << std::endl
         << "                   BeginPoint" << Point (bpRow, bpCol) << "   EndPoint" << Point (epRow, epCol) << "." << std::endl
         << std::endl;
    return;
  }

  kkint32  deltaY = epRow - bpRow;
  kkint32  deltaX = epCol - bpCol;

  kkint32  row, col;

  if  (deltaY == 0)
  {
    // We have a Straight Horizontal Line

    row = bpRow;
    
    kkint32  startCol;
    kkint32  endCol;

    if  (bpCol < epCol)
    {
      startCol = bpCol;
      endCol   = epCol;
    }
    else
    {
      startCol = epCol;
      endCol   = bpCol;
    }

    for  (col = startCol;  col <= endCol;  col++)
    {
      green[row][col] = MergeAlpfaBeta (alpha, g, beta, green[row][col]);
      if  (color)
      {
        red [row][col] = MergeAlpfaBeta (alpha, r, beta, red [row][col]);
        blue[row][col] = MergeAlpfaBeta (alpha, b, beta, blue[row][col]);
      }
    }

    return;
  }

  if  (deltaX == 0)
  {
    // We have a Straight Vertical Line

    col = bpCol;
    
    kkint32  startRow;
    kkint32  endRow;

    if  (bpRow < epRow)
    {
      startRow = bpRow;
      endRow   = epRow;
    }
    else
    {
      startRow = epRow;
      endRow   = bpRow;
    }

    for  (row = startRow;  row <= endRow;  row++)
    {
      green[row][col] = MergeAlpfaBeta (alpha, g, beta, green[row][col]);
      if  (color)
      {
        red [row][col] = MergeAlpfaBeta (alpha, r, beta, red [row][col]);
        blue[row][col] = MergeAlpfaBeta (alpha, b, beta, blue[row][col]);
      }
    }

    return;
  }

  float  m = (float)deltaY / (float)deltaX;

  float  c = (float)bpRow - m * (float)bpCol;

  if  (fabs (m) < 0.5)
  {
    kkint32  startCol;
    kkint32  endCol;

    if  (bpCol < epCol)
    {
      startCol = bpCol;
      endCol   = epCol;
    }
    else
    {
      startCol = epCol;
      endCol   = bpCol;
    }

    for  (col = startCol;  col <= endCol;  col++)
    {
      row = (kkint32)(m * (float)col + c + 0.5f);
      green[row][col] = MergeAlpfaBeta (alpha, g, beta, green[row][col]);
      if  (color)
      {
        red [row][col] = MergeAlpfaBeta (alpha, r, beta, red [row][col]);
        blue[row][col] = MergeAlpfaBeta (alpha, b, beta, blue[row][col]);
      }
    }
  }
  else
  {
    kkint32  startRow;
    kkint32  endRow;

    if  (bpRow < epRow)
    {
      startRow = bpRow;
      endRow   = epRow;
    }
    else
    {
      startRow = epRow;
      endRow   = bpRow;
    }

    for  (row = startRow;  row <= endRow;  row++)
    {
      col = (kkint32)((((float)row - c) / m) + 0.5f);
      green[row][col] = MergeAlpfaBeta (alpha, g, beta, green[row][col]);
      if  (color)
      {
        red [row][col] = MergeAlpfaBeta (alpha, r, beta, red [row][col]);
        blue[row][col] = MergeAlpfaBeta (alpha, b, beta, blue[row][col]);
      }
    }
  }
}  /* DrawLine */



void  Raster::DrawFatLine (Point       startPoint,
                           Point       endPoint, 
                           PixelValue  pv,
                           float       alpha
                          )
{
  kkint32  bpRow = startPoint.Row ();
  kkint32  bpCol = startPoint.Col ();
  kkint32  epRow = endPoint.Row   ();
  kkint32  epCol = endPoint.Col   ();

  if  ((bpRow < 0)  ||  (bpRow >= height)  ||
       (bpCol < 0)  ||  (bpCol >= width)   ||
       (epRow < 0)  ||  (epRow >= height)  ||
       (epCol < 0)  ||  (epCol >= width)
      )
  {
    cerr << std::endl
         << "*** WARNING ***"
         << "Raster::DrawFatLine,  Out of Raster Boundaries   Height[" << height << "]  width[" << width << "]."  << std::endl
         << "                      BeginPoint" << startPoint << "   EndPoint" << endPoint << "." << std::endl
         << std::endl;
    return;
  }

  kkint32  deltaY = epRow - bpRow;
  kkint32  deltaX = epCol - bpCol;

  kkint32  row, col;

  if  (deltaY == 0)
  {
    // We have a Straight Horizontal Line

    row = bpRow;
    
    kkint32  startCol;
    kkint32  endCol;

    if  (bpCol < epCol)
    {
      startCol = bpCol;
      endCol   = epCol;
    }
    else
    {
      startCol = epCol;
      endCol   = bpCol;
    }

    for  (col = startCol;  col <= endCol;  col++)
      PaintFatPoint (row, col, pv, alpha);

    return;
  }

  if  (deltaX == 0)
  {
    // We have a Straight Vertical Line

    col = bpCol;
    
    kkint32  startRow;
    kkint32  endRow;

    if  (bpRow < epRow)
    {
      startRow = bpRow;
      endRow   = epRow;
    }
    else
    {
      startRow = epRow;
      endRow   = bpRow;
    }

    for  (row = startRow;  row <= endRow;  row++)
      PaintFatPoint (row, col, pv, alpha);

    return;
  }

  float  m = (float)deltaY / (float)deltaX;

  float  c = (float)bpRow - m * (float)bpCol;

  if  (fabs (m) < 0.5)
  {
    kkint32  startCol;
    kkint32  endCol;

    if  (bpCol < epCol)
    {
      startCol = bpCol;
      endCol   = epCol;
    }
    else
    {
      startCol = epCol;
      endCol   = bpCol;
    }

    for  (col = startCol;  col <= endCol;  col++)
    {
      row = (kkint32)(m * (float)col + c + 0.5);
      PaintFatPoint (row, col, pv, alpha);
    }
  }
  else
  {
    kkint32  startRow;
    kkint32  endRow;

    if  (bpRow < epRow)
    {
      startRow = bpRow;
      endRow   = epRow;
    }
    else
    {
      startRow = epRow;
      endRow   = bpRow;
    }

    for  (row = startRow;  row <= endRow;  row++)
    {
      col = (kkint32)((((float)row - c) / m) + 0.5);
      PaintFatPoint (row, col, pv, alpha);
    }
  }
}  /* DrawFatLine */




void   Raster::DrawLine (kkint32  bpRow,    kkint32 bpCol,
                         kkint32  epRow,    kkint32 epCol,
                         PixelValue  pixelVal
                        )
{
  DrawLine (bpRow, bpCol, epRow, epCol, pixelVal.r, pixelVal.g, pixelVal.b);
}  /* DrawLine */




void   Raster::DrawLine (kkint32  bpRow,    kkint32 bpCol,
                         kkint32  epRow,    kkint32 epCol,
                         PixelValue  pixelVal,
                         float       alpha
                        )
{
  DrawLine (bpRow, bpCol, epRow, epCol, pixelVal.r, pixelVal.g, pixelVal.b, alpha);
}  /* DrawLine */




void   Raster::DrawLine (const Point&  beginPoint,
                         const Point&  endPoint,
                         uchar         pixelVal
                        )
{
  DrawLine (beginPoint.Row (), beginPoint.Col (), 
            endPoint.Row (),   endPoint.Col (), 
            pixelVal
           );
}  /* DrawLine */



void   Raster::DrawLine (const Point&       beginPoint,
                         const Point&       endPoint,
                         const PixelValue&  pixelVal
                        )
{
  DrawLine (beginPoint.Row (), beginPoint.Col (), 
            endPoint.Row   (), endPoint.Col   (), 
            pixelVal
           );

}  /* DrawLine */




void  Raster::DrawPointList (const PointList&   borderPixs,
                             const PixelValue&  pixelValue
                            )
{
  DrawPointList (Point ((kkint32)0, (kkint32)0), borderPixs, pixelValue.r, pixelValue.g, pixelValue.b);
}


void  Raster::DrawPointList (Point              offset,
                             const PointList&   borderPixs,
                             const PixelValue&  pixelValue
                            )
{
  DrawPointList (offset, borderPixs, pixelValue.r, pixelValue.g, pixelValue.b);
}




void  Raster::DrawPointList (const PointList&  borderPixs,
                             uchar             redVal,
                             uchar             greenVal,
                             uchar             blueVal
                            )
{
  DrawPointList (Point ((kkint32)0, (kkint32)0), borderPixs, redVal, greenVal, blueVal);
}  /* DrawPointList */




void  Raster::DrawPointList (Point             offset,
                             const PointList&  borderPixs,
                             uchar             redVal,
                             uchar             greenVal,
                             uchar             blueVal
                            )
{
  PointList::const_iterator  pIDX;

  PointPtr  pixel = NULL;

  kkint32  row, col;

  for  (pIDX = borderPixs.begin ();  pIDX != borderPixs.end ();  pIDX++)
  {
    pixel = *pIDX;
    row = pixel->Row () + offset.Row ();
    col = pixel->Col () + offset.Col ();

    if  ((row < height)  &&  
         (row >= 0)      &&
         (col < width)   &&
         (col >= 0)   
        )  
    {
      green[row][col] = greenVal;
      if  (color)
      {
        red [row][col] = redVal;
        blue[row][col] = blueVal;
      }
    }
    else
    {
      continue;
    }
  }
}  /* DrawPointList */



void  Raster::DrawConnectedPointList (Point              offset,
                                      const PointList&   borderPixs,
                                      const PixelValue&  pixelValue,
                                      const PixelValue&  linePixelValue
                                     )
{
  PointList::const_iterator  pIDX;

  PointPtr  pixel = NULL;
  PointPtr  lastPoint = NULL;

  kkint32  row = 0, col = 0;

  for  (pIDX = borderPixs.begin ();  pIDX != borderPixs.end ();  pIDX++)
  {
    pixel = *pIDX;
    if  (lastPoint)
    {
      kkint32  deltaRow = lastPoint->Row () - pixel->Row ();
      kkint32  deltaCol = lastPoint->Col () - pixel->Col ();

      kkint32  distSquared = deltaRow * deltaRow  +  deltaCol * deltaCol;

      if  (distSquared > 5)
      {
        // We need to draw a line between the two points.
        DrawLine (*lastPoint + offset, *pixel + offset, linePixelValue);
      }
    }

    if  (pixel != NULL)
    {
      row = pixel->Row () + offset.Row ();
      col = pixel->Col () + offset.Col ();
    }

    if  ((row <  height)  &&  (col <  width)   &&
         (row >= 0)       &&  (col >= 0)    
        )
    {
      green[row][col] = pixelValue.g;
      if  (color)
      {
        red [row][col] = pixelValue.r;
        blue[row][col] = pixelValue.b;
      }
    }

    lastPoint = pixel;
  }
}  /* DrawPointList */



void  Raster::DrawDot (const Point&       point, 
                       const PixelValue&  paintColor,
                       kkint32            size    // Diameter in Pixels of Dot
                      )
{
  SetPixelValue (point, paintColor);
  
  double  radius = (double)size / 2.0;
  double  radiusSquared = radius * radius;

  kkint32  left  = Max ((kkint32)0,           (kkint32)floor ((double)point.Col () - radius));
  kkint32  right = Min ((kkint32)(width - 1), (kkint32)ceil  ((double)point.Col () + radius));

  kkint32  bot   = Max ((kkint32)0,            (kkint32)floor ((double)point.Row () - radius));
  kkint32  top   = Min ((kkint32)(height - 1), (kkint32)ceil  ((double)point.Row () + radius));

  kkint32  row, col;

  for  (row = bot;  row <= top;  row++)
  {
    kkint32  deltaRow = row - point.Row ();
    kkint32  deltaRowSquared = deltaRow * deltaRow;

    if  (deltaRowSquared <= radiusSquared)
    {
      for  (col = left;  col <= right;  col++)
      {
        kkint32  deltaCol = col - point.Col ();
        double  distFromCenterSquares = deltaRowSquared + deltaCol * deltaCol;

        if  (distFromCenterSquares <= radiusSquared)
        {
          // we are within the dot.
          SetPixelValue (row, col, paintColor);
        }
      }
    }
  }
}  /* DrawDot */




void  Raster::DrawCircle (float              centerRow, 
                          float              centerCol, 
                          float              radius,
                          const PixelValue&  pixelValue
                         )
{
  kkint32  row, col;
  float  x, y;

  float  start = -(float)ceil  (radius);
  float  end   = +(float)floor (radius);

  float  radiusSquare = radius * radius;

  for  (x = start;  x <= end;  x++)
  {
    y = (float)sqrt (radiusSquare - (x * x));
    col = (kkint32)(x + centerCol + 0.5);
    row = (kkint32)(centerRow + 0.5 + y);
    SetPixelValue (row, col, pixelValue);

    row = (kkint32)(centerRow + 0.5 - y);
    SetPixelValue (row, col, pixelValue);


    row = (kkint32)(x + centerRow + 0.5);
    col = (kkint32)(centerCol + 0.5 + y);
    SetPixelValue (row, col, pixelValue);

    col = (kkint32)(centerCol + 0.5 - y);
    SetPixelValue (row, col, pixelValue);

  }
}  /* DrawCircle */





void  Raster::DrawCircle (float              centerRow, 
                          float              centerCol, 
                          float              radius,
                          float              startAngle,
                          float              endAngle,
                          const PixelValue&  pixelValue
                         )
{
  float  row, col;

  while  (startAngle < 0.0f)
    startAngle += (float)TwoPie;

  while  (startAngle >= (float)TwoPie)
    startAngle -= (float)TwoPie;

  while  (endAngle < startAngle)
    endAngle += (float)TwoPie;
  
  while  ((endAngle - startAngle) >= (float)TwoPie)
    endAngle -= (float)TwoPie;

  float  angle = startAngle;
  float  angleIncrement = asin (0.5f / radius);

  while  (angle <= endAngle)
  {
    // 1st Determine Which Quarter we r in

    float  qtrAngle = angle;
    while  (qtrAngle > (float)TwoPie)
      qtrAngle -= (float)TwoPie;

    row = -(radius * cos (angle));
    col =  (radius * sin (angle));

    kkint32  adjRow = (kkint32)(centerRow + row + 0.5f);
    kkint32  adjCol = (kkint32)(centerCol + col + 0.5f);

    if  ((adjRow >= 0)  &&  (adjRow < height)  &&
         (adjCol >= 0)  &&  (adjCol < width))
      SetPixelValue ((kkint32)(centerRow + row + 0.5f), (kkint32)(centerCol + col + 0.5f), pixelValue);

    angle += angleIncrement;
  }
}  /* DrawCircle */





void  Raster::DrawCircle (const Point&       point,
                          kkint32            radius,
                          const PixelValue&  paintColor
                         )
{
  DrawCircle ((float)point.Row (),
              (float)point.Col (),
              (float)radius,
			  paintColor
             );
}  /* DrawCircle */



void  Raster::SmoothImageChannel (uchar**  src,
                                  uchar**  dest,
                                  kkint32  maskSize
                                 )  const
{
  kkint32  row, col;

  kkint32  firstMaskRow, firstMaskCol;
  kkint32  lastMaskRow,  lastMaskCol;

  kkint32  maskRow,  maskCol;

  kkint32  maskOffset = maskSize / 2;
  
  for  (row = 0;  row < height;  row++)
  {
    firstMaskRow = row - maskOffset;
    lastMaskRow  = firstMaskRow + maskSize - 1;

    firstMaskRow = Max ((kkint32)0,    firstMaskRow);
    lastMaskRow  = Min (lastMaskRow, (kkint32)(height - 1));

    for  (col = 0;  col < width;  col++)
    {
      firstMaskCol = col - maskOffset;
      lastMaskCol = firstMaskCol + maskSize - 1;
      firstMaskCol = Max (firstMaskCol, (kkint32)0);
      lastMaskCol  = Min (lastMaskCol,  (kkint32)(width - 1));

      kkint32  total      = 0;
      kkint32  numOfCells = 0;

      for  (maskRow = firstMaskRow;  maskRow <= lastMaskRow;  maskRow++)
      {
        for  (maskCol = firstMaskCol;  maskCol <= lastMaskCol;  maskCol++)
        {
          total += src[maskRow][maskCol];
          numOfCells++;
        }
      }

      dest[row][col] = (uchar)((kkint32)((float)((float)total / (float)numOfCells) + 0.5f));
    }
  }

  return;
} /* SmoothImageChannel */



RasterPtr  Raster::CreateSmoothImage (kkint32 maskSize)  const
{
  RasterPtr  result = AllocateARasterInstance (*this);
  if  (maskSize < 2)
    return result;

  if  (red)
    SmoothImageChannel (red,   result->Red   (), maskSize);

  if  (green)
    SmoothImageChannel (green, result->Green (), maskSize);

  if  (blue)
    SmoothImageChannel (blue,  result->Blue  (), maskSize);

  return  result;
} /* CreateSmoothImage */



template<typename T>
T  FindKthValue (T*    values, 
                 kkint32 arraySize, 
                 kkint32 Kth
                )
{
  T    pv;
  kkint32  left  = 0;
  kkint32  right = arraySize - 1;


  kkint32  pivotIndex = right;

  kkint32  partitionIndex = -1;

  T temp;

  while  (partitionIndex != Kth)
  {
    pv = values[pivotIndex];
    
    partitionIndex = left;
    for  (kkint32 i = left;  i < right;  i++)
    {
      if  (values[i] <= pv)
      {
        if  (i != partitionIndex)
        {
          temp = values[i];
          values[i] = values[partitionIndex];
          values[partitionIndex] = temp;
        }
        partitionIndex = partitionIndex + 1;
      }
    }

    temp = values[partitionIndex];
    values[partitionIndex] = values[right];
    values[right] = temp;

    if  (Kth < partitionIndex)
      right = partitionIndex - 1;
    else
      left  = partitionIndex + 1;

    pivotIndex = right;
  }

  return  values[Kth];
}  /* FindKthValue */



RasterPtr  Raster::CreateSmoothedMediumImage (kkint32 maskSize)  const
{
  RasterPtr  result = AllocateARasterInstance (*this);
  if  (maskSize < 2)
    return result;

  uchar**  destR = result->Red   ();
  uchar**  destG = result->Green ();
  uchar**  destB = result->Blue  ();

  kkint32  maxCandidates = maskSize * maskSize;
  uchar*  candidatesRed   = NULL;
  uchar*  candidatesGreen = new uchar[maxCandidates];
  uchar*  candidatesBlue  = NULL;
  if  (color)
  {
    candidatesRed  = new uchar[maxCandidates];
    candidatesBlue = new uchar[maxCandidates];
  }

  kkint32  numCandidates = 0;
  kkint32  middleCandidate = 0;

  kkint32  row, col;

  kkint32  firstMaskRow, firstMaskCol;
  kkint32  lastMaskRow,  lastMaskCol;

  kkint32  maskRow,  maskCol;

  kkint32  maskOffset = maskSize / 2;

  //uchar*  srcRow = NULL;
  
  for  (row = 0;  row < height;  row++)
  {
    firstMaskRow = row - maskOffset;
    lastMaskRow  = firstMaskRow + maskSize - 1;

    firstMaskRow = Max ((kkint32)0, firstMaskRow);
    lastMaskRow  = Min (lastMaskRow, (kkint32)(height - 1));
    
    for  (col = 0;  col < width;  col++)
    {
      firstMaskCol = col - maskOffset;
      lastMaskCol = firstMaskCol + maskSize - 1;
      firstMaskCol = Max (firstMaskCol, (kkint32)0);
      lastMaskCol  = Min (lastMaskCol,  (kkint32)(width - 1));

      //if  (ForegroundPixel (row, col))
      //{
        numCandidates = 0;

        for  (maskRow = firstMaskRow;  maskRow <= lastMaskRow;  maskRow++)
        {
          for  (maskCol = firstMaskCol;  maskCol <= lastMaskCol;  maskCol++)
          {
            //if  (ForegroundPixel (maskRow, maskCol))
            //{
              candidatesGreen[numCandidates] = green[maskRow][maskCol];
              if  (color)
              {
                candidatesRed[numCandidates]  = green[maskRow][maskCol];
                candidatesBlue[numCandidates] = green[maskRow][maskCol];
              }
              numCandidates++;
            //}
          }
        }

        middleCandidate = numCandidates / 2;
        uchar  medium = FindKthValue (candidatesGreen, numCandidates, middleCandidate);
        destG[row][col] = medium;

        if  (color)
        {
          medium = FindKthValue (candidatesRed, numCandidates, middleCandidate);
          destR[row][col] = medium;
          medium = FindKthValue (candidatesBlue, numCandidates, middleCandidate);
          destB[row][col] = medium;
        }
        //}
    }
  }

  delete[]  candidatesRed;    candidatesRed   = NULL;
  delete[]  candidatesGreen;  candidatesGreen = NULL;
  delete[]  candidatesBlue;   candidatesBlue  = NULL;

  return  result;
} /* CreateSmoothedMediumImage */



RasterPtr  Raster::HalfSize ()
{
  kkint32  hHeight = kkint32 (height / 2);
  kkint32  hWidth  = kkint32 (width  / 2);

  RasterPtr  halfSize = AllocateARasterInstance (hHeight, hWidth, color);

  kkint32  row = 0;
  kkint32  col = 0;
  kkint32  hRow, hCol;
  for  (hRow = 0;  hRow < hHeight;  hRow++)
  {
    col = 0;
    for  (hCol = 0;  hCol < hWidth;  hCol++)
    {
      halfSize->SetPixelValue (hRow, hCol, green[row][col]);
      if  (color)
      {
        halfSize->SetPixelValue (ColorChannels::Red,  hRow, hCol, red [row][col]);
        halfSize->SetPixelValue (ColorChannels::Blue, hRow, hCol, blue[row][col]);
      }

      col += 2;
    }

    row += 2;
  }

  return  halfSize;
}  /* HalfSize */



RasterPtr  Raster::ReduceByEvenMultiple (kkint32  multiple)  const
{
  MorphOpReduceByEvenMultiple  reducer (multiple);
  return reducer.PerformOperation (this);
}  /* ReduceByEvenMultiple */



RasterPtr  Raster::ReduceByFactor (float factor)  const  //  0 < factor <= 1.0
{
  MorphOpReduceByFactor reducer (factor);
  return reducer.PerformOperation (this);
}  /* ReduceByFactor */



RasterPtr  Raster::SobelEdgeDetector ()  const
{
  MorphOpSobel  sobel;
  return sobel.PerformOperation (this);
}  /* SobelEdgeDetector */



RasterPtr  Raster::BinarizeByThreshold (uchar  min,
                                        uchar  max
                                       )  const
{
  MorphOpBinarize  binarizer (min, max);
  RasterPtr result = binarizer.PerformOperation (this);
  return  result;
}  /* BinarizeByThreshold */




RasterPtr  Raster::ExtractChannel (ColorChannels  channel)
{
  if  (!color)
    return  AllocateARasterInstance (*this);

  RasterPtr  r = AllocateARasterInstance (height, width, false);

  uchar*  src = NULL;

  KKStr  rootName = osGetRootName (FileName ());

  if  (channel == ColorChannels::Red)
  {
    r->FileName (rootName + "_Red.bmp") ;
    src = RedArea ();
  }

  else if  (channel == ColorChannels::Green)
  {
    r->FileName (rootName + "_Green.bmp") ;
    src = GreenArea ();
  }

  else 
  {
    r->FileName (rootName + "_Blue.bmp") ;
    src = BlueArea ();
  }

  uchar*  dest = r->GreenArea ();

  memcpy (dest, src, totPixels);

  return  r;
}  /* ExtractChannel */




/**
 *@brief  Extracts the pixel locations where the 'mask' images pixel location is a foreground pixel. 
 */
RasterPtr  Raster::ExtractUsingMask (RasterPtr  mask)
{
  RasterPtr  result = new Raster (height, width, color);
  result->BackgroundPixelTH    (this->BackgroundPixelTH    ());
  result->BackgroundPixelValue (this->BackgroundPixelValue ());
  result->ForegroundPixelValue (this->ForegroundPixelValue ());

  if  (!mask)
    return  result;

  int  heighToUse = Min (height, mask->Height ());
  int  widthToUse = Min (width,  mask->Width  ());

  uchar  maskBackgroundValue = mask->BackgroundPixelValue ();
  uchar  maskBackgroundTH    = mask->BackgroundPixelTH ();


  for  (int row = 0;  row < heighToUse;  ++row)
  {
    uchar*  maskRow = (mask->Green ())[row];

    uchar*  resultGreenRow = (result->Green ())[row];
    uchar*  srcGreenRow = green[row];

    uchar*  resultRedRow  = NULL;
    uchar*  srcRedRow     = NULL;

    uchar*  resultBlueRow = NULL;
    uchar*  srcBlueRow    = NULL;

    if  (color)
    {
      resultRedRow  = (result->Red  ())[row];
      srcRedRow     = red[row];
      resultBlueRow = (result->Blue ())[row];
      srcBlueRow    = blue[row];
    }

    for  (int col = 0;  col < widthToUse;  ++col)
    {
      bool  backgroundPix = (maskBackgroundValue < 125) ? (maskRow[col] <= maskBackgroundTH) : (maskRow[col] >= maskBackgroundTH);
      bool  usePixel = !backgroundPix;
      if  (usePixel)
      {
        resultGreenRow[col] = srcGreenRow[col];
        if  (color)
        {
          resultRedRow [col] = srcRedRow  [col];
          resultBlueRow[col] = srcBlueRow [col];
        }
      }
    }
  }
  return  result;
}  /* ExtractUsingMask */






//*******************************************************************************
//*  Wrote this method to deal with pollution sample particles, needed to help  *
//* segment out particles from a very noisy background.                         *
//******************************************************************************
RasterPtr   Raster::SegmentImage (bool  save)
{
  KKStr  rootName  = osGetRootName (fileName);
  KKStr  dirName = "c:\\Temp\\PolutionImages\\" + rootName;
  if  (save)
    osCreateDirectoryPath (dirName);
  osAddLastSlash (dirName);

  KKStr  baseName = dirName + rootName;

  if  (save)
    SaveImage (*this, baseName + "_Orig.bmp");

  bool  imageIsWhiteOnBlack = false;

  kkint32  r, c;

  float  threshold;

  float  backgroundValue;
  float  forgroundValue;

  RasterPtr  gsImage = NULL;

  if  (Color ())
  {
    gsImage = CreateGrayScale ();

    RasterPtr  redPart    = ExtractChannel (ColorChannels::Red);
    RasterPtr  greenPart  = ExtractChannel (ColorChannels::Green);
    RasterPtr  bluePart   = ExtractChannel (ColorChannels::Blue);

    delete  redPart;
    delete  greenPart;
    delete  bluePart;
  }
  else
  {
    gsImage = AllocateARasterInstance (*this);
  }

  RasterPtr  mask = NULL;

  RasterPtr  smoothedImage = gsImage->CreateSmoothImage ();
  {
    // Now lets determine if white or black background
 
    uchar**  g = smoothedImage->Rows ();

    kkint32  totalOfPixelVals = 0;
    kkint32  count = 0;
 
    kkint32  lastRow = Height () - 1;
    kkint32  lastCol = Width () - 1;

    for  (c = 0;  c < Width ();  c++)
    {
      totalOfPixelVals += g[0][c];
      totalOfPixelVals += g[lastRow][c];
      count += 2;
    }

    for  (r = 0;  r < Height ();  r++)
    {
      totalOfPixelVals += g[r][0];
      totalOfPixelVals += g[r][lastCol];
      count += 2;
    }

    threshold = (float)totalOfPixelVals / (float)count;

    imageIsWhiteOnBlack = (threshold < (float)175.0);
  }

  imageIsWhiteOnBlack = false;

  if  (imageIsWhiteOnBlack)
  {
    gsImage->BackgroundPixelValue (0);
    gsImage->ForegroundPixelValue (255);
  }
  else
  {
    gsImage->BackgroundPixelValue (255);
    gsImage->ForegroundPixelValue (0);
  }

  if  (save)
  {
    SaveImage (*gsImage, baseName + "_GrayScale.bmp");
    SaveImage (*smoothedImage, baseName + "_Smoothed.bmp");
  }


  {
    HistogramPtr  grayScaleHistogram = smoothedImage->HistogramGrayscale ();

    if  (save)
    {
      grayScaleHistogram->Save           (baseName + "_Histogram.txt");
      grayScaleHistogram->SaveGraphImage (baseName + "_Histogram.bmp");
    }

    HistogramPtr  grayScaleHistogramSmoothed = grayScaleHistogram->Smooth (4);

    if  (save)
    {
      grayScaleHistogramSmoothed->Save           (baseName + "_HistogramSmoothed.txt");
      grayScaleHistogramSmoothed->SaveGraphImage (baseName + "_HistogramSmoothed.bmp");
    }

    uchar**  g = smoothedImage->Rows ();

    if  (imageIsWhiteOnBlack)
    {
      backgroundValue = grayScaleHistogramSmoothed->AverageOfMaxBucketInRange (0, 120);
      forgroundValue  = grayScaleHistogramSmoothed->AverageOfMaxBucketInRange (130, 255);
      //threshold = grayScaleHistogramSmoothed->AverageOfMinBucketInRange (backgroundValue, forgroundValue);
      //threshold = backgroundValue + 10;
      threshold = (float)(backgroundValue + forgroundValue) / 2.0f;
    }
    else
    {
      backgroundValue = grayScaleHistogramSmoothed->AverageOfMaxBucketInRange (130, 255);
      forgroundValue  = grayScaleHistogramSmoothed->AverageOfMaxBucketInRange (0, 120);
      //threshold = grayScaleHistogramSmoothed->AverageOfMinBucketInRange (forgroundValue, backgroundValue);
      //threshold = backgroundValue - 10; 
      threshold = (float)(backgroundValue + forgroundValue) / 2.0f;
    }

    //threshold = (backgroundValue + forgroundValue) / (float)2.0;

    mask = AllocateARasterInstance (Height (), Width (), false);
    mask->BackgroundPixelValue (0);
    mask->ForegroundPixelValue (255);

    for  (r = 0;  r < Height ();  r++)
    {
      for  (c = 0;  c < Width ();  c++)
      {
        if  (imageIsWhiteOnBlack)
        {
          if  (g[r][c] > threshold)
            mask->SetPixelValue (r, c, 255);
          else
            mask->SetPixelValue (r, c, 0);
        }
        else
        {
          // Image is Black on White
          if  (g[r][c] < threshold)
            mask->SetPixelValue (r, c, 255);
          else
            mask->SetPixelValue (r, c, 0);
        }  
      }
    }

    delete  grayScaleHistogramSmoothed;
    delete  grayScaleHistogram;
  }

  if  (save)
    SaveImage (*mask, baseName + "_Mask_" + StrFormatInt ((kkint32)threshold, "zz0") + ".bmp");

  RasterPtr  destRaster = AllocateARasterInstance (Height (), Width (), false);
  destRaster->BackgroundPixelValue (255);
  destRaster->ForegroundPixelValue (0);

  for  (r = 0;  r < Height ();  r++)
  {
    for  (c = 0;  c < Width ();  c++)
    {
      if  (mask->GetPixelValue (r, c) >= 150)
      {
        if  (imageIsWhiteOnBlack)
          destRaster->SetPixelValue (r, c, gsImage->GetPixelValue (r, c));
        else
          destRaster->SetPixelValue (r, c, (uchar)(255 - gsImage->GetPixelValue (r, c)));
      }
      else
      {
        destRaster->SetPixelValue (r, c, 255);
      }
    }
  }


  delete  gsImage;
  delete  mask;

  
  if  (save)
    SaveImage (*destRaster, baseName + "_Segmented.bmp");

  return  destRaster;
}  /* SegmentImage */




RasterListPtr  Raster::SplitImageIntoEqualParts (kkint32 numColSplits,
                                                 kkint32 numRowSplits
                                                )  const
{
  if  ((numColSplits < 1)  ||  (numColSplits >= width))
    return NULL;

  if  ((numRowSplits < 1)  ||  (numRowSplits >= height))  
    return NULL;

  RasterListPtr  parts = new RasterList (true);

  kkint32  partStartingCol = 0;
  kkint32  partStartingRow = 0;
 
  // because the image might not divide evenly we may have to do some
  // adjusting,  possibly loosing some rows and columns off the edges.
  kkint32  partWidth  = (kkint32)((float)(width  / numColSplits) + 0.5);
  kkint32  colsNeeded = numColSplits * partWidth;
  if  (colsNeeded > width)
  {
    partWidth--;
    partStartingCol = (width - (partWidth * numColSplits)) / 2;
  }

  kkint32  partHeight = (kkint32)((float)(height  / numRowSplits) + 0.5);
  kkint32  rowsNeeded = numRowSplits * partHeight;
  if  (rowsNeeded > height)
  {
    partWidth--;
    rowsNeeded = numRowSplits * partHeight;
    partStartingRow = (height -rowsNeeded) / 2;
  }

  kkint32  splitRow;
  kkint32  splitCol;

  kkint32  partStartRow = partStartingRow;

  for  (splitRow = 0;  splitRow < numRowSplits;  splitRow++)
  {
    kkint32  partEndRow = partStartRow + partHeight - 1;

    kkint32  partStartCol = partStartingCol;

    for  (splitCol = 0;  splitCol < numColSplits;  splitCol++)
    {
      kkint32  partEndCol = partStartCol + partWidth - 1;

      RasterPtr  part = new Raster (*this, partStartRow, partStartCol, partHeight, partWidth);

      parts->PushOnBack (part);

      partStartCol = partEndCol + 1;
    }

    partStartRow = partEndRow + 1;
  }

  return  parts;
}  /* SplitImageIntoEqualParts */




RasterPtr  Raster::ThinContour ()  const
{
  MorphOpThinContour  thinner;
  return thinner.PerformOperation (this);
}



RasterPtr  Raster::TightlyBounded (kkuint32 borderPixels)  const
{
  kkint32  tlRow = 0;
  kkint32  tlCol = 0;
  kkint32  brRow = 0;
  kkint32  brCol = 0;

  FindBoundingBox (tlRow, tlCol, brRow, brCol);
  if  ((tlRow > brRow)  ||  (tlCol > brCol))
  {
    if  (borderPixels < 1)
      borderPixels = 1;
    return AllocateARasterInstance (borderPixels * 2, borderPixels * 2, color);
  }

  kkint32  newHeight = (brRow - tlRow) + borderPixels * 2 +  + 1;
  kkint32  newWidth  = (brCol - tlCol) + borderPixels * 2 +  + 1;

  RasterPtr  result = AllocateARasterInstance (newHeight, newWidth, color);

  uchar**  newRed   = result->Red   ();
  uchar**  newGreen = result->Green ();
  uchar**  newBlue  = result->Blue  ();

  kkint32  oldR, oldC;

  kkint32  newR = borderPixels;
  for  (oldR = tlRow;  oldR <= brRow;  oldR++)
  {
    kkint32  newC = borderPixels;
    for  (oldC = tlCol;  oldC < brCol;  oldC++)
    {
      newGreen[newR][newC] = green[oldR][oldC];
      if  (color)
      {
        newRed [newR][newC] = red [oldR][oldC];
        newBlue[newR][newC] = blue[oldR][oldC];
      }
      
      newC++;
    }
    newR++;
  }

  return  result;
}  /* TightlyBounded */



RasterPtr  Raster::Transpose ()  const
{
  RasterPtr  result = new Raster (width, height, color);

  uchar**  resultRed   = result->Red   ();
  uchar**  resultGreen = result->Green ();
  uchar**  resultBlue  = result->Blue  ();

  for  (kkint32  row = 0;  row < height;  ++row)
  {
    for  (kkint32  col = 0;  col < width;  ++col)
    {
      resultGreen[col][row] = green[row][col];
      if  (color)
      {
        resultRed [col][row] = red [row][col];
        resultBlue[col][row] = blue[row][col];
      }
    }
  }

  return  result;
}



RasterPtr  Raster::ToColor ()  const
{
  if  (color)
    return AllocateARasterInstance (*this);

  RasterPtr  r = AllocateARasterInstance (height, width, true);

  if  (this->ForegroundPixelValue () < 100)
  {
    // 
    memcpy (r->RedArea   (), greenArea, totPixels);
    memcpy (r->GreenArea (), greenArea, totPixels);
    memcpy (r->BlueArea  (), greenArea, totPixels);
  }
  else
  {
    uchar* srcGreen    = greenArea;
    uchar* targetRed   = r->RedArea   ();
    uchar* targetGreen = r->GreenArea ();
    uchar* targetBlue  = r->BlueArea  ();
    for  (kkint32 x = 0;  x < this->totPixels;  ++x)
    {
      uchar  pc = (uchar)(255 - *srcGreen);
      *targetRed   = pc;
      *targetGreen = pc;
      *targetBlue  = pc;

      ++srcGreen;
      ++targetRed;
      ++targetGreen;
      ++targetBlue;
    }
  }

  return r;
}  /* ToColor */



RasterPtr  Raster::BandPass (float  lowerFreqBound,    /**< Number's between 0.0 and 1.0  */
                             float  upperFreqBound,    /**< Represent percentage.         */
                             bool   retainBackground
                            )
{
  #if  defined(FFTW_AVAILABLE)
    fftwf_complex*   src  = NULL;
    fftwf_complex*   dest = NULL;
    fftwf_plan       plan = NULL;
    src  = (fftwf_complex*)fftwf_malloc (sizeof (fftwf_complex) * totPixels);
    dest = (fftwf_complex*)fftwf_malloc (sizeof (fftwf_complex) * totPixels);
  #else
    KK_DFT2D_Float*  forwardPlan = new KK_DFT2D_Float (height, width, true);

    KK_DFT2D_Float::DftComplexType*   srcArea = NULL;
    KK_DFT2D_Float::DftComplexType**  src     = NULL;

    KK_DFT2D_Float::DftComplexType*   destArea = NULL;
    KK_DFT2D_Float::DftComplexType**  dest     = NULL;

    forwardPlan->AllocateArray (srcArea,  src);
    forwardPlan->AllocateArray (destArea, dest);
  #endif

  kkint32  col;
  kkint32  row;

  kkint32  idx = 0;


  double  centerCol = (double)width  / 2.0;
  double  centerRow = (double)height / 2.0;

  uchar  smallestPixelVal = 255;
  uchar  largestPixelVal  = 0;
  uchar  pixelVal         = 0;

  // float scalingFact = (float)255.0 / maxPixVal;   // kk  2004-May-18

  for  (row = 0; row < height; row++ )
  {
    for (col = 0; col < width; col++ )
    {     
      pixelVal = green[row][col];
      if  (pixelVal < smallestPixelVal)
        smallestPixelVal = pixelVal;
      if  (pixelVal > largestPixelVal)
        largestPixelVal = pixelVal;

      // src[idx].re = (float)green[row][col] * scalingFact;  // kk 2004-May-18

      #if  defined(FFTW_AVAILABLE)
        src[idx][0] = (float)green[row][col];                   // kk 2004-May-18
        src[idx][1] = 0.0;
      #else
        if  (color)
        {
          srcArea[idx].real ((float)(0.39f * redArea[idx] + 0.59f * greenArea[idx] + 0.11f * blueArea[idx]) / 3.0f);
        }
        else
        {
          srcArea[idx].real ((float)greenArea[idx]);
        }
        srcArea[idx].imag (0.0f);
      #endif

      idx++;
    }            
  }

  double  pixelValRange = largestPixelVal - smallestPixelVal;


  #if  defined(FFTW_AVAILABLE)
    plan = fftwCreateTwoDPlan (height, width, src, dest, FFTW_FORWARD, FFTW_ESTIMATE);
    fftwf_execute (plan);
    fftwDestroyPlan (plan);
  #else
    forwardPlan->Transform (src, dest);
    delete  forwardPlan;
    forwardPlan = NULL;
  #endif
    
  //  Will now perform the BandPass portion;  that is we will ZERO out all 
  // data that does not fall within the frequency range specified by
  // 'lowerFreqBound' and 'upperFreqBound'.
    
  double  deltaRow        = 0.0;
  double  deltaRowSquared = 0.0;
  double  deltaCol        = 0.0;
  double  deltaColSquared = 0.0;


  // Because the lower frequencies are further away from the center than higher frequencies
  // the 'lowerFreqBoundDistFromCenter' will be greater than 'upperFreqBoundDistFromCenter'
  
  double  zzz = sqrt ((double)(centerCol * centerCol + centerRow * centerRow));

  double  lowerFreqBoundDistFromCenter = (1.0 - (double)lowerFreqBound) * (double)zzz;
  double  upperFreqBoundDistFromCenter = (1.0 - (double)upperFreqBound) * (double)zzz;

  double  lowerFreqBoundDistFromCenterSquared = (lowerFreqBoundDistFromCenter * lowerFreqBoundDistFromCenter + 0.1f);
  double  upperFreqBoundDistFromCenterSquared = (upperFreqBoundDistFromCenter * upperFreqBoundDistFromCenter - 0.1f);

  idx = 0;

  for  (row = 0; row < height; row++ )
  {
    deltaRow = (double)row - centerRow;
    deltaRowSquared = deltaRow * deltaRow;

    for (col = 0; col < width; col++ )
    {     
      deltaCol = (double)col - centerCol;
      deltaColSquared = deltaCol* deltaCol;

      double  distFromCenterSquared = deltaRowSquared + deltaColSquared;

      if  ((distFromCenterSquared > lowerFreqBoundDistFromCenterSquared)  ||
           (distFromCenterSquared < upperFreqBoundDistFromCenterSquared)
          )
      {
        // We are out of the band so this data does not get passed through.
        #if  defined(FFTW_AVAILABLE)
          dest[idx][0] = 0.0f;
          dest[idx][1] = 0.0f;
        #else
          destArea[idx].real (0.0f);
          destArea[idx].imag (0.0f);
        #endif
      }
      idx++;
    }
  }

  #if  defined(FFTW_AVAILABLE)
    plan = fftwCreateTwoDPlan (height, width, src, dest, FFTW_BACKWARD, FFTW_ESTIMATE);
    fftwf_execute (plan);
    fftwDestroyPlan (plan);
  #else
    KK_DFT2D_Float*  reversePlan = new KK_DFT2D_Float (height, width, false);
    reversePlan->Transform (dest, src);
  #endif

  // We now need to transform the Fourier results back to GreayScale.
  double  smallestNum;
  double  largestNum;

  #if  defined(FFTW_AVAILABLE)
    smallestNum = largestNum = src[0][0];
  #else
    smallestNum = largestNum = srcArea[0].real ();
  #endif

  {
    double  zed = 0;
    for (idx = 0;  idx < totPixels;  idx++)
    {
      #if  defined(FFTW_AVAILABLE)
        zed = src[idx][0];
      #else
        zed = srcArea[idx].real ();
      #endif

      if  (zed < smallestNum)
        smallestNum = zed;
      else if  (zed > largestNum)
        largestNum = zed;
    }
  }

  double  range = largestNum - smallestNum;


  RasterPtr  result = AllocateARasterInstance (height, width, false);
  uchar*  destData = result->greenArea;
  
  {
    idx = 0;
    double  zed = 0;
    for (idx = 0;  idx < totPixels;  idx++)
    {     
      if  (retainBackground  &&  BackgroundPixel (greenArea[idx]))
      {
        // Since in the original source image this pixel was a background pixel;  we will
        // continue to let it be one.
        destData[idx] = backgroundPixelValue;
      }
      else
      {
        #if  defined(FFTW_AVAILABLE)
          zed = src[idx][0];
        #else
          zed = srcArea[idx].real ();
        #endif
        destData[idx] = (uchar)(smallestPixelVal + Min (largestPixelVal, (uchar)(0.5 + pixelValRange * (zed - smallestNum) / range)));
      }
    }
  }

  #if  defined(FFTW_AVAILABLE)
    fftwf_free (src);
    fftwf_free (dest);
  #else
    reversePlan->DestroyArray (srcArea,  src);
    reversePlan->DestroyArray (destArea, dest);
    delete  reversePlan;
    reversePlan = NULL;
  #endif

  return  result;
}  /* BandPass */



RasterPtr  RasterList::CreateSmoothedFrame ()
{
  if  (QueueSize () < 1)
    return NULL;

  kkint32 x;

  RasterPtr  firstRaster = IdxToPtr (0);
  kkint32 height = firstRaster->Height ();
  kkint32 width  = firstRaster->Height ();
  kkint32 totPixels = height * width;

  kkuint32*  totGreenArea = new kkuint32[totPixels];
  memset (totGreenArea, 0, totPixels * sizeof (kkuint32));

  kkint32 idx = 0;
  kkint32 rastersAdded = 0;

  for  (idx = 0;  idx < QueueSize ();  idx++)
  {
    RasterPtr raster = IdxToPtr (idx);
    if  ((raster->Height () != height)  ||
         (raster->Width  () != width)
        )
    {
      continue;
    }

    if  (raster->Color ())
    {
      uchar*  redArea   = raster->RedArea ();
      uchar*  greenArea = raster->GreenArea ();
      uchar*  blueArea  = raster->BlueArea ();
      for (x = 0;  x < totPixels;  x++)
      {
        totGreenArea[x] += (kkuint32)((float)redArea[x]   * 0.39f +
                                  (float)greenArea[x] * 0.59f +
                                  (float)blueArea[x]  * 0.11f +
                                  0.5f
                                 );
      }
    }
    else
    {
      uchar*  greenArea = raster->GreenArea ();
      for (x = 0;  x < totPixels;  x++)
        totGreenArea[x] += greenArea[x];
    }

    rastersAdded++;
  }

  RasterPtr  smoothedRaster = firstRaster->AllocateARasterInstance (height, width, false);

  uchar*  newGreenArea = smoothedRaster->GreenArea ();
  for  (x = 0;  x < totPixels;  x++)
    newGreenArea[x] = (uchar)  (totGreenArea[x] / rastersAdded);

  delete[]  totGreenArea;

  return  smoothedRaster;
}  /* CreateSmoothedFrame */




uchar*   Raster::SimpleCompression (kkuint32&  buffLen) const // Will create a compress image using 'SimpleCompression'
{
  kkint32  totalPixs = height * width;

  SimpleCompressor  compressor (totalPixs);

  compressor.Add16BitInt (height);
  compressor.Add16BitInt (width);

  kkint32  x;
  for  (x = 0;  x < totalPixs;  x++)
    compressor.AddByte (greenArea[x]);

  return  compressor.CreateCompressedBuffer (buffLen);
}  /* SimpleCompression */
  


// Creates a raster from a compressedBuff created by 'SimpleCompression'
RasterPtr  Raster::FromSimpleCompression (const uchar*  compressedBuff,
                                          kkuint32      compressedBuffLen
                                         )  
{
  // I expect simple run length compressed data to be passed in.  The data was  originally 
  // compressed by SimpleCompressor and we will use the same class to decompress.

  // The format of the uncompressed data is very simple.
  //  Bytes    Description
  //  0 thru 1:    Height   2 byte integer.
  //  2 thru 3:    Width    2 byte integer.
  //
  //  4 thru (Hight * Width) - 1:   Raster Data
  //          Row 0          : (consists of 'Width' bytes.)
  //          Row 1          : (    ""    ""    ""   ""   )
  //                   --     ---     ---     ----   
  //          Row Height - 1 : (Last row of image) 
  //
  //          Each byte in the raster represents one pixel 0 - 255 gray-scale;  where 0=background.

  kkuint32  unCompressedSize = 0;

  if  (compressedBuff == NULL)
  {
    ofstream f ("c:\\Temp\\Raster_FromSimpleCompression.txt", ios_base::app);

    f << std::endl
      << "DateTime"                 << "\t"  <<  osGetLocalDateTime () << "\t"
      << "(compressedBuff==NULL)"
      << std::endl;

    f.flush ();
    f.close ();
    return new Raster (20, 20, false);
  }

  uchar*  uncompressedBuff = SimpleCompressor::Decompress (compressedBuff, compressedBuffLen, unCompressedSize);
  if  (uncompressedBuff == NULL)
  {
    ofstream f ("c:\\Temp\\Raster_FromSimpleCompression.txt", ios_base::app);

    f << std::endl
      << "DateTime"                 << "\t"  <<  osGetLocalDateTime () << "\t"
      << "(uncompressedBuff==NULL)" << "\t" 
      << "unCompressedSize"         << "\t" << unCompressedSize
      << std::endl;
      f.flush ();
      f.close ();
    return new Raster (20, 20, false);
  }

  // The first Four Bytes have the image Height, and Width.

  kkint32  height = uncompressedBuff[0] * 256 +  uncompressedBuff[1];
  kkint32  width  = uncompressedBuff[2] * 256 +  uncompressedBuff[3];

  if  ((height < 1)  ||  (height > 1000)  ||  (width < 1)  ||  (width > 1000))
  {
    ofstream f ("c:\\Temp\\Raster_FromSimpleCompression.txt", ios_base::app);

    f << std::endl
      << "DateTime"         << "\t"  << osGetLocalDateTime () << "\t"
      << "unCompressedSize" << "\t"  << unCompressedSize      << "\t"
      << "Height"           << "\t"  << height                << "\t" 
      << "Width"            << "\t"  << width
      << std::endl;

    f <<  (kkint32)compressedBuff [0] << "\t"
      <<  (kkint32)compressedBuff [1] << "\t"
      <<  (kkint32)compressedBuff [2] << "\t"
      <<  (kkint32)compressedBuff [3] << "\t"
      <<  (kkint32)compressedBuff [4] << "\t"
      <<  (kkint32)compressedBuff [5] << "\t"
      <<  (kkint32)compressedBuff [6] << "\t"
      <<  (kkint32)compressedBuff [7] << "\t"
      <<  (kkint32)compressedBuff [8] << "\t"
      <<  (kkint32)compressedBuff [9] << "\t"
      <<  (kkint32)compressedBuff[10] 
      << std::endl;

      f.flush ();
      f.close ();
    return new Raster (20, 20, false);
  }

  kkuint32  totalPixels = height * width;
  if  (unCompressedSize > (totalPixels + 4))
  {
    ofstream f ("c:\\Temp\\Raster_FromSimpleCompression.txt", ios_base::app);

    f << std::endl
      << "DateTime"         << "\t"  << osGetLocalDateTime () << "\t"
      << "unCompressedSize" << "\t"  << unCompressedSize      << "\t"
      << "totalPixels"      << "\t"  << totalPixels           << "\t"
      << "Height"           << "\t"  << height                << "\t" 
      << "Width"            << "\t"  << width
      << std::endl;

    f <<  (kkint32)compressedBuff [0] << "\t"
      <<  (kkint32)compressedBuff [1] << "\t"
      <<  (kkint32)compressedBuff [2] << "\t"
      <<  (kkint32)compressedBuff [3] << "\t"
      <<  (kkint32)compressedBuff [4] << "\t"
      <<  (kkint32)compressedBuff [5] << "\t"
      <<  (kkint32)compressedBuff [6] << "\t"
      <<  (kkint32)compressedBuff [7] << "\t"
      <<  (kkint32)compressedBuff [8] << "\t"
      <<  (kkint32)compressedBuff [9] << "\t"
      <<  (kkint32)compressedBuff[10] 
      << std::endl;

    f.flush ();
    f.close ();
  }


  RasterPtr  result = new Raster (height, width, false);
  uchar* greenArea = result->GreenArea ();

  kkuint32  nextIdx = 4;
  kkuint32  greanAreaIdx = 0;

  while  ((nextIdx < (kkuint32)unCompressedSize)  &&  (greanAreaIdx < totalPixels))
  {
    greenArea[greanAreaIdx] = uncompressedBuff[nextIdx];
    nextIdx++;
    greanAreaIdx++;
  }

  delete  uncompressedBuff;
  uncompressedBuff = NULL;

  return  result;
}  /* FromSimpleCompression */



uchar*  Raster::ToCompressor (kkuint32&  compressedBuffLen)  const
{
  // Will first write Rater data to a buffer that will be compressed by zlib by the Compressor class.
  // 0 - 3:    Height:  high order to low order
  // 4 - 7:    Width:   high order to low order
  // 8 - 8:    Color    0 = Gray-scale,  1 = Color
  // 9 - 8 + (Height * Width) Green Channel
  // xxxxx                    Red  Channel
  // xxxxx                    Blue Channel
  kkuint32  totalDataNeeded = totPixels + (color ? (2 * totPixels) : 0) + 9;
  uchar*  buff = new uchar[totalDataNeeded];
  if  (!buff)
    return  NULL;

  memset (buff, 0, totalDataNeeded);

  kkuint32  h = (kkuint32)height;
  buff[0] = h % 256;  h = h / 256;
  buff[1] = h % 256;  h = h / 256;
  buff[2] = h % 256;  h = h / 256;
  buff[3] = h % 256;  h = h / 256;


  kkuint32  w = (kkuint32)width;
  
  buff[4] = w % 256;  w = w / 256;
  buff[5] = w % 256;  w = w / 256;
  buff[6] = w % 256;  w = w / 256;
  buff[7] = w % 256;  w = w / 256;

  buff[8] = (color ? 1 : 0);

  kkint32  buffIdx = 9;
  kkint32  x = 0;
  for  (x = 0;  x < totPixels;  x++, buffIdx++)
    buff[buffIdx] = greenArea[x];

  if  (color)
  {
    for  (x = 0;  x < totPixels;  x++, buffIdx++)
      buff[buffIdx] = redArea[x];

    for  (x = 0;  x < totPixels;  x++, buffIdx++)
      buff[buffIdx] = blueArea[x];
  }

  compressedBuffLen = 0;
  uchar*  compressedBuff = (uchar*)Compressor::CreateCompressedBuffer (buff, totalDataNeeded, compressedBuffLen);
  delete[]  buff;  buff = NULL;
  return  compressedBuff;
}  /* ToCompressor */





RasterPtr  Raster::FromCompressor (const uchar*  compressedBuff,    // Creates a raster from a compressedBuff created by 'Compressor'(zlib)
                                   kkuint32      compressedBuffLen
                                  )
{
  kkuint32  unCompressedBuffLen = 0;
  uchar*  unCompressedBuff = (uchar*)Compressor::Decompress (compressedBuff, compressedBuffLen, unCompressedBuffLen);
  if  (!unCompressedBuff)
    return NULL;

  if  (unCompressedBuffLen < 10)
  {
    cerr << std::endl << std::endl << "Raster::FromCompressor  Compressor did not return any data."  << std::endl;
    return NULL;
  }

  // 0 - 3:    Height:  high order to low order
  // 4 - 7:    Width:   high order to low order
  // 8 - 8:    Color    0 = Gray--                                     scale,  1 = Color
  // 9 - 8 + (Height * Width) Green Channel
  // xxxxx                    Red  Channel
  // xxxxx                    Blue Channel

  kkuint32  height = 0;
  kkuint32  width  = 0;
  height = unCompressedBuff[0] + unCompressedBuff[1] * 256 + unCompressedBuff[2] * 256 * 256 + unCompressedBuff[3] * 256 * 256 * 256;
  width  = unCompressedBuff[4] + unCompressedBuff[5] * 256 + unCompressedBuff[6] * 256 * 256 + unCompressedBuff[7] * 256 * 256 * 256;

  bool color = (unCompressedBuff[8] == 1);

  kkuint32  totalPixels = height * width;
  if  ((totalPixels > (100 * 1024 * 1024))  ||  (height < 1)  ||  (width < 1))
  {
    cerr << std::endl << std::endl << "Raster::FromCompressor  Height[" << height << "]  Width[" << width << "]  is not valid." << std::endl << std::endl;
    delete  unCompressedBuff;
    unCompressedBuff = NULL;
    return NULL;
  }

  kkuint32  totalDataNeeded = totalPixels + (color ? (2 * totalPixels) : 0) + 9;
  if  (totalDataNeeded > unCompressedBuffLen)
  {
    cerr << std::endl << std::endl 
         << "Raster::FromCompressor  Height[" << height << "]  Width[" << width << "]  Color[" << (color ? "Yes" : "No") << "]" << std::endl
         << "              requires TotalDataNeeded[" << totalDataNeeded << "]  but UnCompressedBuffLen[" << unCompressedBuffLen << "]" << std::endl
         << std::endl;
    delete  unCompressedBuff;
    unCompressedBuff = NULL;
    return NULL;
  }


  RasterPtr  r = new Raster ((kkint32)height, (kkint32)width, color);

  kkuint32 nextIdx = 9;
  kkuint32 x;

  uchar*  greenArea = r->GreenArea ();
  for  (x = 0;  x < totalPixels;  x++, nextIdx++)
    greenArea[x] = unCompressedBuff[nextIdx];

  if  (color)
  {
    uchar*  redArea = r->RedArea ();
    for  (x = 0;  x < totalPixels;  x++, nextIdx++)
      redArea[x] = unCompressedBuff[nextIdx];

    uchar*  blueArea = r->BlueArea ();
    for  (x = 0;  x < totalPixels;  x++, nextIdx++)
      blueArea[x] = unCompressedBuff[nextIdx];
  }
 
  delete  unCompressedBuff;
  unCompressedBuff = NULL;

  return  r;
}  /* FromCompressor */




RasterPtr   Raster::Padded (kkint32 padding)
{
  kkint32  newHeight = height + padding * 2;
  kkint32  newWidth  = width  + padding * 2;

  kkint32  r, c;

  RasterPtr  paddedRaster = AllocateARasterInstance (newHeight, newWidth, false);

  for  (r = 0;  r < height;  r++)
  {
    for  (c = 0;  c < width; c++)
    {
      paddedRaster->SetPixelValue (r + padding, c + padding, this->GetPixelValue (r, c));
    }
  }

  return  paddedRaster;
}  /* Padded */




MatrixPtr  Raster::BuildGaussian2dKernel (float  sigma)  
{
  kkint32 row, col, x, y;

  double  prefix =  1.0 / (2.0 * PIE * sigma * sigma);
  double  twoSigmaSquared = 2.0 * sigma * sigma;


  // Determine size of kernel
  double z = 100;
  kkint32  delta = 0;
  while (true)
  {
    z = 256.0 * prefix * exp (-(delta * delta / twoSigmaSquared));
    if  (z < 1.0)
    {
      delta--;
      break;
    }
    ++delta;
  }

  kkint32 len = delta * 2 + 1;

  MatrixPtr  kernel = new Matrix (len, len);

  double  total = 0.0;
  x =  -delta;
  for (row = 0;  row < len;  row++, x++)
  {
    y = -delta;
    for  (col = 0;  col < len;  col++, y++)
    {
      double  v = exp (-( (x * x + y * y) / twoSigmaSquared));
      (*kernel)[row][col] = v;
      total += v;
    }
  }

  for (row = 0;  row < len;  row++)
  {
    for  (col = 0;  col < len;  col++)
    {
      (*kernel)[row][col] = ((*kernel)[row][col] / total);
    }
  }

  return  kernel;
}  /* BuildGaussian2dKernel */



void  Raster::SmoothUsingKernel (Matrix&  kernel,
                                 uchar**  src,
                                 uchar**  dest
                                )  const
{
  kkint32  row, col;
  kkint32  kernelSideLen = kernel.NumOfCols ();
  kkint32  delta = kernelSideLen / 2;

  double const * const *  kernalData = kernel.Data ();

  for  (row = 0;  row < height;  row++)
  {
    kkint32  maskTop = row - delta;
    for  (col = 0;  col < width;  col++)
    {
      kkint32  maskLeft = col - delta;
      kkint32  maskRow  = maskTop;
      double  total  = 0.0;
      double kernelTotal = 0.0;
      for  (kkint32 kernelRow = 0;  (kernelRow < kernelSideLen)  &&  (maskRow < height);  kernelRow++, maskRow++)
      {
        if  (maskRow >= 0)
        {
          double const *  kernalRowData = kernalData[kernelRow];

          kkint32  maskCol = maskLeft;
          for  (kkint32 kernelCol = 0;  (kernelCol < kernelSideLen)  &&  (maskCol < width);  kernelCol++, maskCol++)
          {
            if  (maskCol >= 0)
            {
              double  fact = kernalRowData[kernelCol];
              total += fact * (float)(src[maskRow][maskCol]);
              kernelTotal += fact;
            }
          }
        }
      }

      total = total / kernelTotal;

      dest[row][col] = (uchar)((kkint32)(total + 0.5));
    }
  }
}  /* SmoothUsingKernel */





RasterPtr  Raster::CreateGaussianSmoothedImage (float sigma)  const
{
  MatrixPtr  kernel = BuildGaussian2dKernel (sigma);

  RasterPtr  result = AllocateARasterInstance (*this);

  SmoothUsingKernel (*kernel, this->green, result->green);
  if  (color)
  {
    SmoothUsingKernel (*kernel, red,  result->red);
    SmoothUsingKernel (*kernel, blue, result->blue);
  }

  delete  kernel;
  kernel = NULL;

  return  result;
}  /* CreateGaussianSmoothedImage */




RasterPtr  Raster::ThresholdInHSI (float              thresholdH,
                                   float              thresholdS, 
                                   float              thresholdI, 
                                   float              distance,
                                   const PixelValue&  flagValue
                                  )
{
  PixelValue pixelColor (0, 0, 0);
  //Vec<float,3> white(255,255,255);
  float tempH, tempS, tempI;
  float y, x, r, xOriginalPoint, yOriginalPoint;

  // converts from polar coordinates to Cartesian coordinates. Hue is an angle and Saturation is a distance from an origin.
  // xOriginalPoint and yOriginalPoint are the threshold point coordinates (origin)
  xOriginalPoint = thresholdS * cos (thresholdH);
  yOriginalPoint = thresholdS * sin (thresholdH);

  RasterPtr  resultingImage = AllocateARasterInstance (*this);

  for  (kkint32 m = 0;  m < height;  m++)
  {
    for  (kkint32 n = 0;  n < width;  n++)
    {
      GetPixelValue (m, n, pixelColor);
      pixelColor.ToHSI (tempH, tempS, tempI);

      // Convert from Cartesian polar coordinates to Cartesian coordinates. These are the Cartesian coordinates of the
      // current pixel
      x = tempS * cos (tempH);
      y = tempS * sin (tempH);

      // Calculate the distance from the threshold initial point to the current pixelColor using
      // the euclidean distance
      r = (float)(sqrt (pow (xOriginalPoint - x, 2) + pow (yOriginalPoint - y, 2) + pow (thresholdI - tempI, 2)));

      if  (r <= distance)
         resultingImage->SetPixelValue (m, n, pixelColor);
      else
        resultingImage->SetPixelValue (m, n, flagValue);
    }
  }

  return  resultingImage;
}  /* ThresholdingHSI */




RasterPtr  Raster::CreateGrayScaleKLT ()  const
{
  if  (!color)
  {
    // Already a Gray-scale image,  nothing much to do.
    return AllocateARasterInstance (*this);
  }

  kkuint32  x = 0;

  MatrixPtr  cov = new Matrix (3, 3);
  {
    // Build a covariance matrix.
    kkint32  col = 0, row = 0;

    double*   totals       = new double[3];
    double*   means        = new double[3];
    double**  centeredVals = new double*[3];
    for  (col = 0;  col < 3;  ++col)
    {
      totals[col] = 0.0;
      means [col] = 0.0;
      centeredVals[col] = new double[totPixels];
    }

    for  (row = 0;  row < totPixels;  ++row)
    {
      totals[0] += (double)redArea   [row];
      totals[1] += (double)greenArea [row];
      totals[2] += (double)blueArea  [row];
    }

    for  (col = 0;  col < 3;  ++col)
      means[col] = totals[col] / (double)totPixels;

    for  (row = 0;  row < totPixels;  ++row)
    {
      centeredVals[0][row] = (double)redArea   [row] - means[0];
      centeredVals[1][row] = (double)greenArea [row] - means[1];
      centeredVals[2][row] = (double)blueArea  [row] - means[2];
    }
 
    for  (kkint32 varIdxX = 0;  varIdxX < 3;  ++varIdxX)
    {
      double*  varXs = centeredVals[varIdxX];
      for  (kkint32 varIdxY = varIdxX;  varIdxY < 3;  ++varIdxY)
      {
        // Calculate the covariance between chanIdx0 and chanIdx1

        double*  varYs = centeredVals[varIdxY];
        double total = 0.0f;
        for  (row = 0;  row < totPixels;  ++row)
          total += varXs[row] * varYs[row];
        (*cov)[varIdxX][varIdxY] = total / (double)(totPixels - 1);
        (*cov)[varIdxY][varIdxX]  = (*cov)[varIdxX][varIdxY];
      }
    }

    for  (col = 0;  col < 3;  col++)
    {
      delete[]  centeredVals[col];
      centeredVals[col] = NULL;
    }
    delete[]  centeredVals;   centeredVals = NULL;
    delete[]  means;          means  = NULL;
    delete[]  totals;         totals = NULL;
  }
 
  MatrixPtr      eigenVectors = NULL;
  VectorDouble*  eigenValues  = NULL;

  cov->EigenVectors (eigenVectors, eigenValues);
  if  (!eigenVectors)
  {
    cerr << std::endl << std::endl 
      << "Raster::CreateGrayScaleKLT  ***ERROR***   Could not derive Eigen Vectors of covariance matrix." << std::endl
      << std::endl;
    delete  cov;
    cov = NULL;
    return NULL;
  }

  kkint32 eigenValueMaxIdx = 0;
  double  eigenValueMax    = (*eigenValues)[0];
  for  (x = 1;  x < eigenValues->size ();  ++x)
  {
    if  ((*eigenValues)[x] > eigenValueMax)
    {
      eigenValueMaxIdx = x;
      eigenValueMax = (*eigenValues)[x];
    }
  }

  VectorDouble  eigenVector = eigenVectors->GetCol (eigenValueMaxIdx);
  // We are now going to do Matrix multiplication between the original image and the eigenVector
  // but since the eigen vector may have negative values the resultant numbers can be negative for
  // some pixels and a magnitude greater than 255.  We will place temporarily into a floating 
  // array,  after which we will use the adjustment value and scaling factor.
  double  redFact   = eigenVector[0];
  double  greenFact = eigenVector[1];
  double  blueFact  = eigenVector[2];
  double  adjVal    = 0.0;

  double  valMin = DBL_MAX;
  double  valMax = DBL_MIN;
  double*  adjChannel = new double [totPixels];
  for  (kkint32 y = 0;  y < totPixels;  ++y)
  {
    adjVal = (double)redArea  [y] * redFact    +
             (double)greenArea[y] * greenFact  +
             (double)blueArea [y] * blueFact;
    if  (adjVal < valMin)  valMin = adjVal;
    if  (adjVal > valMax)  valMax = adjVal;
    adjChannel[y] = adjVal;
  }

  if  (valMax <= valMin)  valMax = valMin + 1.0;   // This would only happen if the whole image had the same GrayScale value.

  //double  adjScaleFact = 256.0 / (1 + valMax - valMin);
  double  adjScaleFact = 255.0 / (valMax - valMin);

  RasterPtr  result = AllocateARasterInstance (Height (), Width (), false);
  {
    uchar*  resultArea = result->GreenArea ();
    for  (kkint32 y = 0;  y < totPixels;  ++y)
    {
      resultArea[y] = (uchar)Min ((kkint32)((adjChannel[y] - valMin) * adjScaleFact + 0.5), (kkint32)255);
    }
  }
  delete[]  adjChannel;  adjChannel   = NULL;
  delete  cov;           cov          = NULL;
  delete  eigenValues;   eigenValues  = NULL;
  delete  eigenVectors;  eigenVectors = NULL;

  return  result;
}  /* CreateGrayScaleKLT */



RasterPtr  Raster::CreateGrayScaleKLTOnMaskedArea (const Raster&  mask)  const
{
  if  (!color)
  {
    // Already a Gray-scale image,  nothing much to do.
    return AllocateARasterInstance (*this);
  }

  if  ((mask.Height () != height)  ||  (mask.Width () != width))
  {
    cerr << std::endl << std::endl
      << "Raster::CreateGrayScaleKLTOnMaskedArea   ***ERROR***   Mask image dimensions must match !!!!" << std::endl
      << "                    Our[" << height << "," << width << "]  Mask[" << mask.height << "," << mask.width << "]." << std::endl
      << std::endl;
    return NULL;
  }

  uchar*  maskArea = mask.GreenArea ();
  uchar   maskTh   = mask.backgroundPixelTH;

  kkint32  totalMaskPixels = mask.TotalBackgroundPixels ();

  MatrixPtr  cov = new Matrix (3, 3);
  {
    // Build a covariance matrix.
    kkint32  col = 0, row = 0;

    double*   totals       = new double[3];
    double*   means        = new double[3];
    double**  centeredVals = new double*[3];
    for  (col = 0;  col < 3;  ++col)
    {
      totals[col] = 0.0;
      means [col] = 0.0;
      centeredVals[col] = new double[totPixels];
    }

    for  (row = 0;  row < totPixels;  ++row)
    {
      if  (maskArea[row] > maskTh)
      {
        totals[0] += (double)redArea   [row];
        totals[1] += (double)greenArea [row];
        totals[2] += (double)blueArea  [row];
      }
    }

    for  (col = 0;  col < 3;  ++col)
      means[col] = totals[col] / (double)totalMaskPixels;

    for  (row = 0;  row < totPixels;  ++row)
    {
      if  (maskArea[row] > maskTh)
      {
        centeredVals[0][row] = (double)redArea   [row] - means[0];
        centeredVals[1][row] = (double)greenArea [row] - means[1];
        centeredVals[2][row] = (double)blueArea  [row] - means[2];
      }
    }
 
    for  (kkint32 varIdxX = 0;  varIdxX < 3;  ++varIdxX)
    {
      double*  varXs = centeredVals[varIdxX];
      for  (kkint32 varIdxY = varIdxX;  varIdxY < 3;  ++varIdxY)
      {
        // Calculate the covariance between chanIdx0 and chanIdx1

        double*  varYs = centeredVals[varIdxY];
        double total = 0.0f;
        for  (row = 0;  row < totPixels;  ++row)
        {
          if  (maskArea[row] > maskTh)
            total += varXs[row] * varYs[row];
        }
        (*cov)[varIdxX][varIdxY] = total / (double)(totalMaskPixels - 1);
        (*cov)[varIdxY][varIdxX]  = (*cov)[varIdxX][varIdxY];
      }
    }

    for  (col = 0;  col < 3;  col++)
    {
      delete  centeredVals[col];
      centeredVals[col] = NULL;
    }
    delete[]  centeredVals;   centeredVals = NULL;
    delete[]  means;          means  = NULL;
    delete[]  totals;         totals = NULL;
  }
 
  MatrixPtr      eigenVectors = NULL;
  VectorDouble*  eigenValues  = NULL;

  cov->EigenVectors (eigenVectors, eigenValues);
  if  (!eigenVectors)
  {
    cerr << std::endl << std::endl 
      << "Raster::CreateGrayScaleKLT  ***ERROR***   Could not derive Eigen Vectors of covariance matrix." << std::endl
      << std::endl;
    delete  cov;
    cov = NULL;
    return NULL;
  }

  kkint32 eigenValueMaxIdx = 0;
  double  eigenValueMax    = (*eigenValues)[0];
  for  (kkuint32 y = 1;  y < eigenValues->size ();  ++y)
  {
    if  ((*eigenValues)[y] > eigenValueMax)
    {
      eigenValueMaxIdx = y;
      eigenValueMax = (*eigenValues)[y];
    }
  }

  VectorDouble  eigenVector = eigenVectors->GetCol (eigenValueMaxIdx);
  // We are now going to do Matrix multiplication between the original image and the eigenVector
  // but since the eigen vector may have negative values the resultant numbers can be negative for
  // some pixels and a magnitude greater than 255.  We will place temporarily into a floating 
  // array,  after which we will no the adjustment value and scaling factor.

  double  redFact   = eigenVector[0];
  double  greenFact = eigenVector[1];
  double  blueFact  = eigenVector[2];
  double  adjVal    = 0.0;

  double  valMin = DBL_MAX;
  double  valMax = -9999999999.99;
  double*  adjChannel = new double [totPixels];
  for  (kkint32 y = 0;  y < totPixels;  ++y)
  {
    if  (maskArea[y] > maskTh)
    {
      adjVal = (double)redArea  [y] * redFact    +
               (double)greenArea[y] * greenFact  +
               (double)blueArea [y] * blueFact;
      if  (adjVal < valMin)  valMin = adjVal;
      if  (adjVal > valMax)  valMax = adjVal;
      adjChannel[y] = adjVal;
    }
    else
    {
      adjChannel[y] = 0.0;
    }
  }

  if  (valMax <= valMin)  valMax = valMin + 1.0;   // This would only happen if the whole image had the same GrayScale value.

  //double  adjScaleFact = 256.0 / (1 + valMax - valMin);
  double  adjScaleFact = 255.0 / (valMax - valMin);

  RasterPtr  result = AllocateARasterInstance (Height (), Width (), false);
  {
    uchar*  resultArea = result->GreenArea ();
    for  (kkint32 y = 0;  y < totPixels;  ++y)
    {
      if  (maskArea[y] > maskTh)
        resultArea[y] = (uchar)Min ((kkint32)((adjChannel[y] - valMin) * adjScaleFact + 0.5), (kkint32)255);
      else
        resultArea[y] = 0;
    }
  }
  delete[] adjChannel;    adjChannel   = NULL;
  delete   cov;           cov          = NULL;
  delete   eigenValues;   eigenValues  = NULL;
  delete   eigenVectors;  eigenVectors = NULL;

  return  result;
}  /* CreateGrayScaleKLTOnMaskedArea */



void   Raster::WhiteOutBackground ()
{
  for  (kkint32 r = 0;  r < height;  ++r)
  {
    for  (kkint32 c = 0;  c < width;  ++c)
    {
      if  (BackgroundPixel (r, c))
      {
        green[r][c] = backgroundPixelValue;
        if  (color)
        {
          red[r][c] = backgroundPixelValue;
          blue[r][c] = backgroundPixelValue;
        }
      }
    }
  }
}  /* WhiteOutBackground */



RasterPtr  Raster::CreateColorImageFromLabels ()
{
  kkint32  x = 0;

  // Determine number of unique values in greenArea channel.
  kkint32  freqCount[256];
  for  (x = 0;  x < 256;  ++x)
    freqCount[x] = 0;
  for  (x = 0;  x < totPixels;  ++x)
    freqCount[greenArea[x]]++;

  multimap<kkint32,uchar>  sortedFreqCounts;
  for  (x = 0;  x < 256;  ++x)
  {
    if  (freqCount[x] > 0)
      sortedFreqCounts.insert (pair<kkint32,uchar> (freqCount[x], (uchar)x));
  }

  PixelValue  colorAssignments[256];
  colorAssignments[0] = PixelValue::Black;
  kkint32  numUniqueValues = (kkint32)sortedFreqCounts.size ();
  multimap<kkint32,uchar>::reverse_iterator  idx;
  x = 0;
  for  (idx = sortedFreqCounts.rbegin ();  idx != sortedFreqCounts.rend ();  ++idx)
  {
    kkint32  pixelValue = idx->second;
    colorAssignments[pixelValue] = PixelValue::FromHSI ((float)x / (float)numUniqueValues, 1.0f, 1.0f);
    x++;
  }

  RasterPtr  colorImage = AllocateARasterInstance (height, width, true);
  uchar*  destRed   = colorImage->RedArea   ();
  uchar*  destGreen = colorImage->GreenArea ();
  uchar*  destBlue  = colorImage->BlueArea  ();
  for  (x = 0;  x < totPixels;  ++x)
  {
    PixelValue& pv = colorAssignments[greenArea[x]];
    destRed  [x] = pv.r;
    destGreen[x] = pv.g;
    destBlue [x] = pv.b;
  }

  return  colorImage;
}  /* CreateColorImageFromLabels */



void  Raster::FillBlob (RasterPtr   origImage,
                        BlobPtr     blob,
                        PixelValue  pixelValue
                       )
{
  if  (blob == NULL)
    return;

  if  (origImage->blobIds == NULL)
    return;

  if  ((origImage->Height () != height)  ||  (origImage->Width () != width))
    return;

  kkint32  blobId = blob->Id ();
  kkint32  row = 0, col = 0;

  kkint32  rowStart = Min (blob->RowTop   (), height - 1);
  kkint32  rowEnd   = Min (blob->RowBot   (), height - 1);
  kkint32  colStart = Min (blob->ColLeft  (), width  - 1);
  kkint32  colEnd   = Min (blob->ColRight (), width  - 1);

  for  (row = rowStart;  row <= rowEnd;  ++row)
  {
    for  (col = colStart;  col <= colEnd;  ++col)
    {
      if  (origImage->blobIds[row][col] == blobId)
      {
        SetPixelValue (row, col, pixelValue);
      }
    }
  }
}  /* FillBlob */



PointListPtr  Raster::DeriveImageLength () const
{
  PointListPtr  results = NULL;

  float  eigenRatio;
  float  orientationAngle;

  RasterPtr  workRaster = CreateErodedImage(MorphOp::MaskTypes::SQUARE3);
  workRaster->FillHole ();
  workRaster->Erosion(MorphOp::MaskTypes::SQUARE7);
  workRaster->ConnectedComponent (1);
  workRaster->CalcOrientationAndEigerRatio (eigenRatio,
                                            orientationAngle
                                           );
  if  ((orientationAngle > TwoPie)  ||  (orientationAngle < -TwoPie))
  {
    orientationAngle = 0.0;
    eigenRatio = 1.0;
  }
 
  RasterPtr rotatedImage = workRaster->Rotate (orientationAngle);
  kkint32  tlRow;
  kkint32  tlCol;
  kkint32  brRow;
  kkint32  brCol;

  rotatedImage->FindBoundingBox (tlRow, tlCol, brRow, brCol);
  if  (tlRow >= 0)
  {
    uchar**  imageData = rotatedImage->Green ();

    kkint32  boxWidth  = brCol - tlCol;

    kkint32  mark1Col = (kkint32)((float)boxWidth * 0.05f + 0.5f) + tlCol;
    kkint32  mark2Col = (kkint32)((float)boxWidth * 0.95f + 0.5f) + tlCol;

    kkint32  a1RowTot   = 0;
    kkint32  a1ColTot   = 0;
    kkint32  a1PixCount = 0l;

    kkint32  a2RowTot   = 0;
    kkint32  a2ColTot   = 0;
    kkint32  a2PixCount = 0l;

    kkint32  a3RowTot   = 0;
    kkint32  a3ColTot   = 0;
    kkint32  a3PixCount = 0;

    for  (kkint32 row = tlRow;  row <= brRow;  ++row)
    {
      kkint32 col = 0;

      uchar*  rowData = imageData[row];

      for  (col = tlCol;  col <= mark1Col;  ++col)
      {
        if  (rowData[col] > 2)
        {
          ++a1PixCount;
          a1RowTot += row;
          a1ColTot += col;
        }
      }

      for  (col = mark1Col + 1;  col <= mark2Col;  ++col)
      {
        if  (rowData[col] > 2)
        {
          ++a2PixCount;
          a2RowTot += row;
          a2ColTot += col;
        }
      }

      for  (col = mark2Col + 1;  col <= brCol;  ++col)
      {
        if  (rowData[col] > 2)
        {
          ++a3PixCount;
          a3RowTot += row;
          a3ColTot += col;
        }
      }
    }

    a1PixCount = Max ((kkint32)1, a1PixCount);
    a2PixCount = Max ((kkint32)1, a2PixCount);
    a3PixCount = Max ((kkint32)1, a3PixCount);

    Point p1 ((kkint16)(0.5f + (float)a1RowTot / (float)a1PixCount), (kkint16)(0.5f + (float)a1ColTot / (float)a1PixCount));
    Point p2 ((kkint16)(0.5f + (float)a2RowTot / (float)a2PixCount), (kkint16)(0.5f + (float)a2ColTot / (float)a2PixCount));
    Point p3 ((kkint16)(0.5f + (float)a3RowTot / (float)a3PixCount), (kkint16)(0.5f + (float)a3ColTot / (float)a3PixCount));

    Point p1Orig = RotateDerivePreRotatedPoint (height, width, p1, orientationAngle);
    Point p2Orig = RotateDerivePreRotatedPoint (height, width, p2, orientationAngle);
    Point p3Orig = RotateDerivePreRotatedPoint (height, width, p3, orientationAngle);

    results = new PointList (true);
    results->PushOnBack (new Point (p1Orig));
    results->PushOnBack (new Point (p2Orig));
    results->PushOnBack (new Point (p3Orig));
  }

  delete rotatedImage;  rotatedImage = NULL;
  delete workRaster;    workRaster   = NULL;

  return  results;

}  /* DeriveImageLength */
