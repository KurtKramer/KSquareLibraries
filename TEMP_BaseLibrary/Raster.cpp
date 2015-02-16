/* Raster.cpp -- Class for one raster image.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
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

#if  defined(FFTW_AVAILABLE)
#include <fftw3.h>
#endif

#include "Raster.h"

#include "BasicTypes.h"
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
#include "OSservices.h"
#include "SimpleCompressor.h"
#include "Sobel.h"
using namespace KKU;


volatile GoalKeeperPtr  Raster::goalKeeper = NULL;
volatile bool           Raster::rasterInitialized = false;

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
  delete  goalKeeper;
  goalKeeper = NULL;
}




//  The following code is being added to support the tracking down of memory leaks in Raster.
map<RasterPtr, RasterPtr>  Raster::allocatedRasterInstances;
void  Raster::AddRasterInstance (const RasterPtr  r)
{
  /*
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
  */
}


void  Raster::RemoveRasterInstance (const RasterPtr  r)
{
  /*
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
  */
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




typedef  enum  
{
  Cross,
  Square
}  MaskShapes;


int32  biases[] = {1,  // CROSS3 
                   2,  // CROSS5
                   1,  // SQUARE3
                   2,  // SQUARE5
                   3,  // SQUARE7
                   4,  // SQUARE9
                   5   // SQUARE11
                  };


int32  maskShapes[] = {Cross,   // CROSS3 
                       Cross,   // CROSS5
                       Square,  // SQUARE3
                       Square,  // SQUARE5
                       Square,  // SQUARE7
                       Square,  // SQUARE9
                       Square   // SQUARE11
                      };



//****************************************************************************************
//*   Used to help quickly calvculate a Intnsity Histogram of a image.  The image has 8  *
//*   levels of gray scale, from 0 -> 7.                                                 *
//****************************************************************************************
//                                       1  1  1  1  1  1  1  1  1  1  2  2  2  2  2  2  2  2  2  2  3  3 
//         0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1 
int32  freqHistBucketIdx[256] = 
          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //   0  - 31
           1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //  31  - 63
           2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, //  64  - 95
           3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, //  96 - 127
           4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, // 128 - 159
           5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, // 160 - 191
           6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, // 192 - 223
           7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7  // 224 - 255
          };

inline  
int32  Min (int32  x1,  int32  x2)
{
  if  (x1 < x2)
    return  x1;
  else
    return  x2;
}



inline  
int32  Max6 (int32 x1,  int32 x2,  int32 x3,  
             int32 x4,  int32 x5,  int32 x6
            )
{
  int32  r;
  
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
int32  Max9 (int32 x1,  int32 x2,  int32 x3,  
           int32 x4,  int32 x5,  int32 x6,  
           int32 x7,  int32 x8,  int32 x9
          )
{
  int32  r;
  
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
  backgroundPixelTH    (DefaultBackgroundPixelTH),
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
}



Raster::Raster (int32  _height,
                int32  _width
               ):

  backgroundPixelValue (0),
  backgroundPixelTH    (DefaultBackgroundPixelTH),
  blobIds              (NULL),
  centroidCol          (0.0f),
  centroidRow          (0.0f),
  color                (false),
  fileName             (),
  foregroundPixelCount (0),
  foregroundPixelValue (255),
  fourierMag           (NULL),
  fourierMagArea       (NULL),
  height               (_height),
  maxPixVal            (0),
  totPixels            (0),
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



Raster::Raster (int32 _height,
                int32 _width,
                bool  _color
               ):

  backgroundPixelValue (0),
  backgroundPixelTH    (DefaultBackgroundPixelTH),
  blobIds              (NULL),
  centroidCol          (0.0f),
  centroidRow          (0.0f),
  color                (_color),
  fileName             (),
  foregroundPixelCount (0),
  foregroundPixelValue (255),

  fourierMag           (NULL),
  fourierMagArea       (NULL),

  height               (_height),
  maxPixVal            (0),
  totPixels            (0),

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
    backgroundPixelTH    = 255 - DefaultBackgroundPixelTH;
  }

  AllocateImageArea ();
}


/**
 *@brief  Constructs a Raster from a BMP image loadded from disk.
 *@detais If BMP Image is a grayscale value pixel values will be reveresed.  See description of grayscale constructor.
 */
Raster::Raster (const BmpImage&  _bmpImage):

  backgroundPixelValue  (0),
  backgroundPixelTH     (DefaultBackgroundPixelTH),
  blobIds               (NULL),
  centroidCol           (0.0f),
  centroidRow           (0.0f),
  color                 (false),
  fileName              (_bmpImage.FileName ()),
  foregroundPixelCount  (0),
  foregroundPixelValue  (255),
  fourierMag            (NULL),
  fourierMagArea        (NULL),
  height                (_bmpImage.Height ()),
  maxPixVal             (0),
  totPixels             (0),
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

  int32  row;

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
  fileName              (_raster.fileName),
  foregroundPixelCount  (_raster.foregroundPixelCount),
  foregroundPixelValue  (_raster.foregroundPixelValue),
  fourierMag            (NULL),
  fourierMagArea        (NULL),
  height                (_raster.height),
  maxPixVal             (_raster.maxPixVal),
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
  memcpy (greenArea, _raster.greenArea, totPixels);

  if  (color)
  {
    memcpy (redArea,  _raster.redArea,  totPixels);
    memcpy (blueArea, _raster.blueArea, totPixels);
  }


  if  (_raster.fourierMagArea)
  {
    AllocateFourierMagnitudeTable ();
    memcpy (fourierMagArea, _raster.fourierMagArea, sizeof (float) * totPixels);
  }
}



Raster::Raster (const Raster&  _raster,
                int32          _row,
                int32          _col,
                int32          _height,
                int32          _width
               ):

  backgroundPixelValue  (_raster.backgroundPixelValue),
  backgroundPixelTH     (_raster.backgroundPixelTH),
  blobIds               (NULL),
  centroidCol           (-1.0f),
  centroidRow           (-1.0f),
  color                 (_raster.color),
  fileName              (),
  foregroundPixelCount  (0),
  foregroundPixelValue  (_raster.foregroundPixelValue),
  fourierMag            (NULL),
  fourierMagArea        (NULL),
  height                (_height),
  maxPixVal             (0),
  totPixels             (_height * _width),
  weOwnRasterData       (true),
  width                 (_width),

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
                int32         _row,
                int32         _col
               ):

  backgroundPixelValue  (_raster.backgroundPixelValue),
  backgroundPixelTH     (_raster.backgroundPixelTH),
  blobIds               (NULL),
  centroidCol           (-1),
  centroidRow           (-1),
  color                 (false),
  fileName              (),
  foregroundPixelCount  (0),
  foregroundPixelValue  (_raster.foregroundPixelValue),
  fourierMag            (NULL),
  fourierMagArea        (NULL),
  height                (biases[_mask] * 2 + 1),
  maxPixVal             (0),
  totPixels             (0),
  weOwnRasterData       (true),
  width                 (biases[_mask] * 2 + 1),

  redArea   (NULL),
  greenArea (NULL),
  blueArea  (NULL),

  red       (NULL),
  green     (NULL),
  blue      (NULL)

{
  AddRasterInstance (this);

  int32  r = _row - biases[_mask];
  int32  c = _col - biases[_mask];

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


/**
 *@brief Constructs a raster object from a Image File.
 *@details If a color image the pixel values will be reveresed, see constructor
 *         for grayscale images.
 */
Raster::Raster (const KKStr&  _fileName,
                bool&          validFile
               ):

  backgroundPixelValue  (0),
  backgroundPixelTH     (DefaultBackgroundPixelTH),
  blobIds               (NULL),
  centroidCol           (-1),
  centroidRow           (-1),
  color                 (false),
  fileName              (_fileName),
  foregroundPixelCount  (0),
  foregroundPixelValue  (255),
  fourierMag            (NULL),
  fourierMagArea        (NULL),
  height                (0),
  maxPixVal             (0),
  totPixels             (0),
  weOwnRasterData       (true),
  width                 (0),

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




Raster::Raster (int32    _height,
                int32    _width,
                uchar*   _grayScaleData,
                uchar**  _grayScaleRows
               ):
  backgroundPixelValue (0),
  backgroundPixelTH    (DefaultBackgroundPixelTH),
  blobIds              (NULL),
  centroidCol          (0.0f),
  centroidRow          (0.0f),
  color                (false),
  fileName             (),
  foregroundPixelCount (0),
  foregroundPixelValue (255),
  fourierMag           (NULL),
  fourierMagArea       (NULL),
  height               (_height),
  maxPixVal            (0),
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



Raster::Raster (int32         _height,
                int32         _width,
                const uchar*  _grayScaleData
               ):
  backgroundPixelValue (0),
  backgroundPixelTH    (DefaultBackgroundPixelTH),
  blobIds              (NULL),
  centroidCol          (0.0f),
  centroidRow          (0.0f),
  color                (false),
  fileName             (),
  foregroundPixelCount (0),
  foregroundPixelValue (255),
  fourierMag           (NULL),
  fourierMagArea       (NULL),
  height               (_height),
  maxPixVal            (0),
  totPixels            (0),
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
  memcpy (greenArea, _grayScaleData, totPixels);
}


Raster::Raster (int32         _height,
                int32         _width,
                const uchar*  _redChannel,
                const uchar*  _greenChannel,
                const uchar*  _blueChannel
               ):
  backgroundPixelValue (0),
  backgroundPixelTH    (DefaultBackgroundPixelTH),
  blobIds              (NULL),
  centroidCol          (0.0f),
  centroidRow          (0.0f),
  color                (true),
  fileName             (),
  foregroundPixelCount (0),
  foregroundPixelValue (255),
  fourierMag           (NULL),
  fourierMagArea       (NULL),
  height               (_height),
  maxPixVal            (0),
  totPixels            (0),
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
    KKU::KKStr errMsg;
    errMsg << "Raster::Raster    ***ERROR***   One of the [rpvided channels is 'NULL'.";
    cerr << std::endl << std::endl << errMsg << std::endl << std::endl;
    throw KKException (errMsg);
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



int32  Raster::MemoryConsumedEstimated ()  const
{
  int32  blobIdsMem = 0;
  if  (blobIds)
    blobIdsMem = sizeof(int32) * totPixels + sizeof (int32*) * height;

  int32  fourierMem = 0;
  if  (fourierMagArea)
    fourierMem = sizeof (float) * totPixels + sizeof (float*) * height;

  int32  pixelMem = totPixels + sizeof (uchar*) * height;
  if  (color)
    pixelMem = pixelMem * 3;

  int32  memoryConsumedEstimated =
    sizeof (uchar)    * 4   +
    sizeof (float)    * 2   +
    sizeof (bool)     * 2   +
    sizeof (int32)    * 4   +
    sizeof (int32**)  * 1   +
    sizeof (uchar*)   * 3   + 
    sizeof (uchar**)  * 3   +
    sizeof (float*)   * 1   +
    sizeof (float**)  * 1   +
    fileName.MemoryConsumedEstimated () +
    blobIdsMem                          +
    fourierMem                          +
    pixelMem;

  return  memoryConsumedEstimated;
}


void  Raster::Initialize (int32    _height,
                          int32    _width,
                          uchar*   _grayScaleData,
                          uchar**  _grayScaleRows,
                          bool     _takeOwnership
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



void  Raster::Initialize (int32    _height,
                          int32    _width,
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


RasterPtr  Raster::AllocateARasterInstance (int32  height,
                                            int32  width,
                                            bool   color
                                           )  const
{
  return new Raster (height, width, color);
}



RasterPtr  Raster::AllocateARasterInstance (const Raster& r)  const
{
  return new Raster (r);
}



RasterPtr  Raster::AllocateARasterInstance (const Raster& _raster,  /**<  Source Raster                                       */
                                            int32         _row,     /**<  Starting Row in '_raster' to copy from.             */
                                            int32         _col,     /**<  Starting Col in '_raster' to copy from.             */
                                            int32         _height,  /**<  Height of resultant raster. Will start from '_row'  */
                                            int32         _width    /**<  Width of resultant raster.                          */
                                           ) const
{
  return new Raster (_raster, _row, _col, _height, _width);
}




bool  Raster::ForegroundPixel (uchar  pixel)  const
{
  return (pixel > backgroundPixelTH);
}  /* ForegroundPixel */



bool  Raster::ForegroundPixel (int32  row,
                               int32  col
                              )  const
{
  return (green[row][col] > backgroundPixelTH);
}  /* ForegroundPixel */



bool  Raster::BackgroundPixel (uchar  pixel)  const
{
  return  (pixel <= backgroundPixelTH);
}  /* ForegroundPixel */




bool  Raster::BackgroundPixel (int32  row,
                               int32  col
                              )  const
{
  return (green[row][col] <= backgroundPixelTH);
}  /* ForegroundPixel */



int32 Raster::TotalBackgroundPixels () const
{
  int32  totalBackgroundPixels = 0;
  for  (int32 x = 0;  x < totPixels;  ++x)
  {
    if  (greenArea[x] > backgroundPixelTH)
      ++totalBackgroundPixels;
  }
  return  totalBackgroundPixels;
}  /* TotalBackgroundPixels */




float    Raster::CentroidCol ()
{
  if  (centroidCol >= 0)
    return  centroidCol;

  float  centroidColWeighted;
  float  centroidRowWeighted;

  int32  weight = 0;
  CalcCentroid (totPixels, weight, centroidRow, centroidCol, centroidRowWeighted, centroidColWeighted);

  return  centroidCol;
} 



float    Raster::CentroidRow ()
{
  if  (centroidRow >= 0)
    return  centroidRow;

  float  centroidColWeighted;
  float  centroidRowWeighted;
  int32  weight = 0;
  CalcCentroid (totPixels, weight, centroidRow, centroidCol, centroidRowWeighted, centroidColWeighted);
  return  centroidRow;
}




RasterPtr  Raster::CreatePaddedRaster (BmpImage&  image,
                                       int32      padding
                                      )
{
  int32  oldWidth  = image.Width  ();
  int32  oldHeight = image.Height ();

  int32  newWidth  = oldWidth  + 2 * padding;
  int32  newHeight = oldHeight + 2 * padding;

  RasterPtr  paddedRaster = new Raster (newHeight, newWidth, false);

  //const uchar**  oldRows = image.Image ();

  uchar**  newRows = paddedRaster->green;

  int32  newRow = padding;
  int32  row;
  int32  col;

  int32  paddedForgroudPixelCount = 0;

  for  (row = 0;  row < oldHeight;  row++)
  {
    const uchar* oldRow = image.ImageRow (row);
    
    int32  newCol = padding;
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



void   Raster::ReverseImage ()
{
  int32 x = 0;

  for  (x = 0;  x < totPixels;  x++)
  {
    greenArea[x] = 255 - greenArea[x];
    if  (color)
    {
      redArea[x]  = 255 - redArea[x];
      blueArea[x] = 255 - blueArea[x];
    }
  }

  uchar  temp = backgroundPixelValue;
  backgroundPixelValue = foregroundPixelValue;
  foregroundPixelValue = temp;
  backgroundPixelTH = 255 - backgroundPixelTH;
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

  int32  row;
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

  for  (int32 row = 0;  row < height;  row++)
  {
    fourierMag[row] = rowPtr;
    rowPtr          = rowPtr + width;
  }
}  /* AllocateFourierMagnitudeTable */



uchar  Raster::GetPixelValue (int32 row,  int32 col)  const

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



void  Raster::GetPixelValue (int32   row,
                             int32   col,
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




void  Raster::GetPixelValue (int32       row,
                             int32       col,
                             PixelValue& p
                            )  const
{
  GetPixelValue (row, col, p.r, p.g, p.b);
}  /* GetPixelValue */






uchar  Raster::GetPixelValue (ColorChannels  channel,
                              int32          row,
                              int32          col
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

  if  (channel == GreenChannel)
    return  green [row][col];

  if  (!color)
  {
    cerr << "***ERROR*** Raster::GetPixelValue   *** ERROR ***,  Not a Color Raster."  << std::endl;
    exit (-1);
  }

  if  (channel == RedChannel)
    return red [row][col];
  else
    return blue[row][col];
}  /* GetPixelValue */




void   Raster::SetPixelValue (int32  row,
                              int32  col,
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





void   Raster::SetPixelValue (int32              row,
                              int32              col,
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




void   Raster::SetPixelValue (int32  row,
                              int32  col,
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
                             int32          row,
                             int32          col,
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

  if  (channel == GreenChannel)
  {
    green[row][col] = pixVal;
    return;
  }

  if  (!color)
  {
    cerr << "***ERROR*** Raster::SetPixelValue   *** ERROR ***,  Not a Color Raster."  << std::endl;
  }

  if  (channel == RedChannel)
    red  [row][col]  = pixVal;
  else
    blue [row][col] = pixVal;

}  /* SetPixelValue  */



bool  Raster::AreThereEdgePixels (int32 edgeWidth)
{
  for  (int32 edgeIdx = 0;  edgeIdx < edgeWidth;  ++edgeIdx)
  {
    int32  topRow = edgeIdx;
    int32  botRow = height - (edgeIdx + 1);

    for  (int c = 0;  c < width;  ++c)
    {
      if  (ForegroundPixel (topRow, c)  ||  ForegroundPixel (botRow, c))
        return true;
    }

    int32  leftCol = edgeIdx;
    int32  rightCol = width - (1 + edgeIdx);

    for  (int r = 0;  r < height;  ++r)
    {
      if  (ForegroundPixel (r, leftCol)  ||  ForegroundPixel (r, rightCol))
        return true;
    }
  }
  return  false;
}  /* AreThereEdgePixels */



void  Raster::DrawGrid (float              pixelsPerMinor,
                        uint32             minorsPerMajor,
                        const PixelValue&  hashColor,
                        const PixelValue&  gridColor
                       )
{
  int  x = 0;

  int  hashLen = 4;

  // Horizontal Hash Marks

  int32  horzOffset = (width - (int32)(((float)width / pixelsPerMinor) * pixelsPerMinor) / 2);
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

  // Verticle Hash Marks
  int32  vertOffset = (height - (int32)((height / pixelsPerMinor) * pixelsPerMinor) / 2);
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
void Raster::CalcDialatedValue (int32   row,
                                int32   col,
                                int32&    totVal,
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




RasterPtr  Raster::CreateDialatedRaster ()  const
{
  int32  row;
  int32  col;

  int32  resultForegroundPixelCount = 0;

  RasterPtr  result = AllocateARasterInstance (*this);

  uchar** resultRows = result->green;

  int32  totValue     = 0;
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
}  /* CreateDialatedRaster */




void  Raster::Dialation ()
{
  RasterPtr  tempRaster = CreateDialatedRaster ();

  delete  greenArea;
  delete  green;

  greenArea = tempRaster->greenArea;
  green     = tempRaster->green;
 
  tempRaster->greenArea = NULL;
  tempRaster->green     = NULL;

  foregroundPixelCount = tempRaster->foregroundPixelCount;

  delete  tempRaster;
}  /* Dialation */

  


RasterPtr  Raster::CreateDialatedRaster (MaskTypes  mask)  const
{
  int32 row;
  int32 col;

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

  int32  resultForegroundPixelCount = 0;
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
}  /* CreateDialatedRaster */





void  Raster::Dialation (MaskTypes  mask)
{    
  RasterPtr tempRaster = CreateDialatedRaster (mask);

  delete  greenArea;
  delete  green;

  foregroundPixelCount = tempRaster->foregroundPixelCount;

  greenArea = tempRaster->greenArea;
  green     = tempRaster->green;

  tempRaster->greenArea = NULL;
  tempRaster->green     = NULL;

  delete  tempRaster;
} /* Dilation */



void   Raster::FillRectangle (int32              tlRow,
                              int32              tlCol,
                              int32              brRow,
                              int32              brCol,
                              const PixelValue&  fillColor
                             )
{
  tlRow = Min (tlRow, height - 1);
  brRow = Min (brRow, height - 1);

  tlCol = Min (tlCol, width - 1);
  brCol = Min (brCol, width - 1);

  for  (int32 row = tlRow;  row <= brRow;  ++row)
  {
    if  (color)
    {
      uchar*  rowRed   = red   [row];
      uchar*  rowGreen = green [row];
      uchar*  rowBlue  = blue  [row];
      for  (int32 col = tlCol;  col <= brCol;  ++col)
      {
        rowRed  [col] = fillColor.r;
        rowGreen[col] = fillColor.g;
        rowBlue [col] = fillColor.b;
      }
    }
    else
    {
      uchar*  rowGreen = green[row];
      for  (int32 col = tlCol;  col <= brCol;  ++col)
        rowGreen[col] = fillColor.g;
    }
  }
}  /* FillRectangle */




void  Raster::FillHoleGrow (int32  _row, 
                            int32  _col
                           )
{
  int32  totPixels = height * width;
  PointList  expandList (true);

  expandList.PushOnBack (new Point (_row, _col));
  
  while  (expandList.QueueSize () > 0)
  {
    PointPtr  nextPixel = expandList.PopFromFront ();
  
    int32  r = nextPixel->Row ();
    int32  c = nextPixel->Col ();
 
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





void  Raster::FillHole ()
{
  int32  r;
  int32  c;
  Raster  mask (*this);

  uchar**  maskRows = mask.green;

  int32  lastRow = height - 1;
  int32  lastCol = width  - 1;

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




void  Raster::Erosion ()
{
  Raster  origRaster (*this);

  uchar**  origRows = origRaster.green;

  foregroundPixelCount = 0;

  int32  r;
  int32  c;

  int32  lastRow = height - 1;
  int32  lastCol = width - 1;

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
  int32  r;
  int32  c;

  int32  bias = biases[mask];

  int32  maskRowStart = 0 - bias;
  int32  maskRowEnd   = 0 + bias;
  int32  maskColStart = 0 - bias;
  int32  maskColEnd   = 0 + bias;
  int32  maskRow;
  int32  maskCol;
  bool   fit = false;


  foregroundPixelCount = 0;
  Raster  tempRaster (*this);
  uchar**     tempGreen = tempRaster.Green ();
  uchar*      tempRowData = NULL;
  MaskShapes  m = (MaskShapes)maskShapes[mask];


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

        else if  (m == Square)
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


void  Raster::ErosionChanged (MaskTypes  mask, 
                              int32      row, 
                              int32      col
                             )
{
  int32  r;
  int32  c;

  int32  bias = biases[mask];

  int32  maskRowStart ;
  int32  maskRowEnd  ; 
  int32  maskColStart ;
  int32  maskColEnd  ;
  maskRowStart = 0 - bias;
  maskRowEnd   = 0+ bias;
  maskColStart = 0 - bias;
  maskColEnd   = 0 + bias;
  
  int32  maskRow;
  int32  maskCol;
  bool  fit;

  foregroundPixelCount = 0;
  Raster      tempRaster (*this);
  uchar**     tempGreen = tempRaster.Green ();
  uchar*      tempRowData = NULL;
  MaskShapes  m = (MaskShapes)maskShapes[mask];

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

        else if  (m == Square)
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
                               int32      row, 
                               int32      col
                              )
{
  int32  r;
  int32  c;

  int32  bias = biases[mask];

  int32  maskRowStart = 0;
  int32  maskRowEnd   = 0; 
  int32  maskColStart = 0;
  int32  maskColEnd   = 0;

  maskRowStart = 0 - bias;
  maskRowEnd   = 0 + bias;
  maskColStart = 0 - bias;
  maskColEnd   = 0 + bias;
  
  int32  maskRow;
  int32  maskCol;
  bool  fit;

  foregroundPixelCount = 0;

  Raster  tempRaster (*this);

  uchar**     tempGreen   = tempRaster.Green ();
  uchar*      tempRowData = NULL;
  MaskShapes  m           = (MaskShapes)maskShapes[mask];

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

        else if  (m == Square)
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
                               int32      blobrowstart, 
                               int32      blobrowend, 
                               int32      blobcolstart, 
                               int32      blobcolend
                              )
{
  int32  r;
  int32  c;

  int32  bias = biases[mask];

  int32  maskRowStart = 0 - bias;
  int32  maskRowEnd   = 0 + bias;
  int32  maskColStart = 0 - bias;
  int32  maskColEnd   = 0 + bias;

  int32  maskRow = 0;
  int32  maskCol = 0;
  bool  fit    = false;

  foregroundPixelCount = 0;
  Raster  tempRaster (*this);

  uchar**     tempGreen   = tempRaster.Green ();
  uchar*      tempRowData = NULL;

  MaskShapes  m = (MaskShapes)maskShapes[mask];

  for  (r = 0;  r < height;  r++)
  {
    //if ((r>= blobrowstart+30) && (r<= blobrowend-30))
    // {
    // }
  // else
  // {
    maskColStart = 0 - bias;
    maskColEnd   = 0 + bias;

    tempRowData = tempGreen[r];

    for  (c = 0; c < width; c++)
    {
      if  ((c >= blobcolstart + 100)  &&  (c <= blobcolend - 100))
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

          else if  (m == Square)
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
  int32  r;
  int32  c;

  RasterPtr   erodedRaster = AllocateARasterInstance (*this);;

  uchar*      srcRow    = NULL;
  uchar**     destGreen  = erodedRaster->Green ();
  uchar*      destRow    = NULL;

  int32  erodedForegroundPixelCount = foregroundPixelCount;


  for  (r = 0;  r < height;  r++)
  {
    destRow = destGreen[r];
    srcRow  = green[r];

    for  (c = 0; c < width; c++)
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
  Dialation ();
}  /* Opening */



void  Raster::Opening (MaskTypes mask)
{
  Erosion (mask);
  Dialation (mask);
}  /* Open */




void  Raster::Closing ()
{
  Dialation ();
  Erosion ();
}  /* Open */





void  Raster::Closing (MaskTypes mask)
{
  Dialation (mask);
  Erosion (mask);
}  /* Open */





inline
bool  Raster::CompletlyFilled3By3 (int32  row, 
                                   int32  col
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
  int32  r;
  int32  c;

  int32  lastRow = height - 1;
  int32  lastCol = width - 1;

  Raster  orig (*this);
  uchar**  origRows = orig.green;

  uchar*  origRowLast = NULL;
  uchar*  origRowCur  = NULL;
  uchar*  origRowNext = NULL;
   
  foregroundPixelCount = 0;

  for  (r = 1; r < lastRow; r++)
  {
    origRowLast = origRows[r - 1];
    origRowCur  = origRows[r];
    origRowNext = origRows[r + 1];

    for  (c = 1; c < lastCol; c++)
    {
      if  (ForegroundPixel (origRows[r][c]))
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
          green[r][c] = backgroundPixelValue;
        }
        else
        {
          green[r][c] = foregroundPixelValue;
          foregroundPixelCount++;
        }
      }
    }
  }
}  /*  Edge */





inline
int32 Raster::BlobId (int32  row,
                      int32  col
                     )  const
{
  if  ((row < 0)  ||  (row >= height)  ||
       (col < 0)  ||  (col >= width))
    return  -1;

  if  (!blobIds)
    return -1;

  return  blobIds[row][col];
}




int32  Raster::NearestNeighborUpperLeft (int32 row,
                                         int32 col,
                                         int32 dist
                                        )
{
  int32  nearestBlob = -1;
  int32 c, r, startCol, blobID;

  startCol = col - 1;

  for (r = row - dist;  r < row;  r++)
  {
    if  (r >= 0)
    {
      for  (c = startCol;  c <= col;  c++)
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
int32  Raster::NearestNeighborUpperRight (int32 row,
                                          int32 col,
                                          int32 dist
                                         )
{
  int32  nearestBlob = -1;
  int32 r, c, endCol, blobID;

  endCol = col + 1;
  for  (r = row - dist;  r < row;  r++)
  {
    if  (r >= 0)
    {
      if  (endCol >= width)
        endCol = width - 1;

      for  (c = col + 1;  c <= endCol;  c++)
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




BlobListPtr   Raster::ExtractBlobs (int32  dist)
{
  uchar*   curRow         = NULL;
  int32*   curRowBlobIds  = NULL;

  int32    col = 2;
  int32    row = 2;

  BlobPtr  curBlob    = NULL;
  int32    curBlobId  = 0;
  int32    nearBlobId = 0;

  int32    blankColsInARow = 0;

  AllocateBlobIds ();  

  BlobListPtr blobs = new BlobList (true);

  for  (row = 0; row < height; row++)
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
        {
          nearBlobId = NearestNeighborUpperRight (row, col, dist);
        }


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
            curBlobId = curBlob->id;
          }
          else
          {
            curBlob = blobs->NewBlob (row, col);
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
          // Check Upper Left 1st
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
    BlobPtr  blob = *idx;
    int32  blobId = blob->Id ();
    int32  row = 0, col = 0;

    PixelValue  color;
    switch  (blobId % 8)
    {
    case  0:  color = PixelValue::Red;      break;
    case  1:  color = PixelValue::Green;    break;
    case  2:  color = PixelValue::Blue;     break;
    case  3:  color = PixelValue::Yellow;   break;
    case  4:  color = PixelValue::Orange;   break;
    case  5:  color = PixelValue::Magenta;  break;
    case  6:  color = PixelValue::Purple;   break;
    case  7:  color = PixelValue::Teal;     break;
    }

    int32  rowStart = Min (blob->RowTop   (), height - 1);
    int32  rowEnd   = Min (blob->RowBot   (), height - 1);
    int32  colStart = Min (blob->ColLeft  (), width  - 1);
    int32  colEnd   = Min (blob->ColRight (), width  - 1);

    for  (row = rowStart;  row <= rowEnd;  ++row)
    {
      for  (col = colStart;  col <= colEnd;  ++col)
      {
        if  (blobIds[row][col] == blobId)
        {
          colorImage->SetPixelValue (row, col, color);
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
    BlobPtr  blob = *idx;
    int32  blobId = blob->Id ();
    int32  row = 0, col = 0;

    int32  rowStart = Min (blob->RowTop   (), height - 1);
    int32  rowEnd   = Min (blob->RowBot   (), height - 1);
    int32  colStart = Min (blob->ColLeft  (), width  - 1);
    int32  colEnd   = Min (blob->ColRight (), width  - 1);

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
    for  (int32 row = 0; row < height; row++)
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
  int32  row, col;

  DeleteExistingBlobIds ();

  blobIds = new int32*[height];

  for  (row = 0; row < height; row++)
  {
    blobIds[row] = new int32[width];
    for  (col = 0; col < width; col++)
    {
      blobIds[row][col] = -1;
    }
  }
}  /* AllocateBlobIds */





void  Raster::ConnectedComponentOrig ()
{
  uchar connectedComponentDist = 3;

  uchar*   curRow            = NULL;
  int32*   curRowBlobIds     = NULL;

  int32    col = 2;
  int32    row = 2;

  BlobPtr  curBlob    = NULL;
  int32    curBlobId  = 0;
  int32    nearBlobId = 0;

  int32    blankColsInARow = 0;

  AllocateBlobIds ();
  BlobListPtr blobs = new BlobList (true);

  for  (row = 0; row < height; row++)
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

        nearBlobId = NearestNeighborUpperLeft (row, col, connectedComponentDist);
                                       
        if  (nearBlobId < 0)
        {
          nearBlobId = NearestNeighborUpperRight (row, col, connectedComponentDist);
        }
  

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
          curBlob->colLeft   = Min (curBlob->colLeft,  col);
          curBlob->colRight  = Max (curBlob->colRight, col);
          curBlob->rowBot    = Max(curBlob->rowBot,    row);
          curBlob->rowTop    = Min (curBlob->rowTop,   row);
          curBlob->pixelCount++;
        }
      }
      else
      {
        blankColsInARow++;

        if  (blankColsInARow > connectedComponentDist)
        {
          curBlob = NULL;
          curBlobId = -1;
        }

        else if  (curBlob)
        {
          nearBlobId = NearestNeighborUpperLeft (row, col, connectedComponentDist);

          if  (nearBlobId >= 0)
          {
            if  (nearBlobId != curBlobId)
            {
              blobs->MergeBlobIds (curBlob, nearBlobId, blobIds);
            }
          }
        }
      }

      col++;
    }
  }

  BlobPtr largestBlob = blobs->LocateLargestBlob ();
  if  (largestBlob)
  {
    int32  blobId = largestBlob->Id ();
     
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
}  /* ConnectedComponentOrig */




void  Raster::ConnectedComponent (uchar connectedComponentDist)
{
  if  (connectedComponentDist < 1)
    connectedComponentDist = 3;

  int32  row = 0, col = 0;

  BlobListPtr  blobs = ExtractBlobs (connectedComponentDist);

  BlobPtr largestBlob = blobs->LocateLargestBlob ();
  if  (largestBlob)
  {
    int32  blobId = largestBlob->Id ();
     
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

  int32  row = 0, col = 0;

  BlobListPtr  blobs = ExtractBlobs (connectedComponentDist);

  BlobPtr largestBlob = blobs->LocateMostComplete ();
  if  (largestBlob)
  {
    int32  blobId = largestBlob->Id ();
     
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
  int32*   curRowBlobIds  = NULL;
  int32*   prevRowBlobIds = NULL;

  int32    col = 2;
  int32    row = 2;

  BlobPtr  curBlob    = NULL;
  int32    curBlobId  = 0;
  int32    nearBlobId = 0;

  int32    blankColsInARow = 0;

  // Initialize Blob ID's

  blobIds = new int32*[height];

  for  (row = 0; row < height; row++)
  {
    blobIds[row] = new int32[width];
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
    int32  blobId = largestBlob->Id ();
    
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
                                        int32          padding
                                       )  const
{
  if  (blob == NULL)
    return NULL;

  RasterPtr  blobRaster = AllocateARasterInstance (blob->Height () + 2 * padding, blob->Width () + 2 * padding, color);

  int32  blobRow = padding;
  int32  blobCol = padding;

  int32  row = 0;
  int32  col = 0;

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

  int32  row;
  int32  col;

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



int32  Raster::CalcArea ()
{
  int32  r, c;

  int32  area = 0;

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






void  Raster::CalcAreaAndIntensityHistogramWhite (int32&  area,
                                                  uint32  intensityHistBuckets[8]
                                                 )
{
  int32  c;
  int32  r;
  int32  x;

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




void  Raster::CalcAreaAndIntensityHistogram (int32&  area,
                                             uint32  intensityHistBuckets[8]
                                            )
{
  int32  r, c;

  area = 0;

  for  (int32 x = 0; x < 8; x++)
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




void   Raster::CalcAreaAndIntensityFeatures (int32&  area,
                                             float&  weighedSize,
                                             uint32  intensityHistBuckets[8],
                                             int32&  areaWithWhiteSpace,
                                             uint32  intensityHistBucketsWhiteSpace[8]
                                            )
{
  int32  x;

  // We first need to determine what background pixels are not part of the image.  We assume
  // that any background pixels that are connected to the border of the image are not part
  // of the image.
  Raster  mask (*this);
  mask.FillHole ();
  
  long  totalPixelValues = 0;

  area               = 0;
  weighedSize        = 0.0f;
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

  weighedSize = (float)totalPixelValues / (float)maxPixVal;
}  /* CalcAreaAndIntensityFeatures */





void   Raster::CalcAreaAndIntensityFeatures (int32&    area,
                                             float&  weighedSize,
                                             uint32  intensityHistBuckets[8]
                                            )
{
  int32  x;

  long  totalPixelValues = 0;

  area               = 0;
  weighedSize        = 0.0f;

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

  weighedSize = (float)totalPixelValues / (float)255.0f;
}  /* CalcAreaAndIntensityFeatures */






float  Raster::CalcWeightedArea ()  const
{
  int32  r, c;

  float  area = 0;

  uchar  pixVal = 0;

  //foregroundPixelCount = 0;

  for  (r = 0; r < height; r++)
  {
    for  (c = 0; c < width; c++)
    {
      pixVal = green[r][c];
      if  (ForegroundPixel (pixVal))
      {
        area = area + (float)pixVal / (float)maxPixVal;
        //foregroundPixelCount++;
      }
    }
  }

  return  area;
}  /* CalcWeightedArea */


       

  
void  Raster::CentralMoments (float  features[9])  const
{
  float m00 = Moment (0, 0);
  float ew  = Moment (1, 0) / m00;
  float eh  = Moment (0, 1) / m00;
        
  float cm00   = m00;
  float gamma2 = m00 * m00;
  float gamma3 = gamma2 * (float)sqrt (m00);
        
  float cm20 = CenMoment (2, 0, ew, eh) / gamma2;
  float cm02 = CenMoment (0, 2, ew, eh) / gamma2;
  float cm11 = CenMoment (1, 1, ew, eh) / gamma2;

  float cm30 = CenMoment (3, 0, ew, eh) / gamma3;
  float cm03 = CenMoment (0, 3, ew, eh) / gamma3;
  float cm12 = CenMoment (1, 2, ew, eh) / gamma3;
  float cm21 = CenMoment (2, 1, ew, eh) / gamma3;

  features[0] = cm00;

  features[1] = cm20 + cm02;
 
  features[2] = (cm20 - cm02) * (cm20 - cm02) + 
                4.0f * cm11 * cm11;

  features[3] = (cm30 - 3.0f * cm12) * (cm30 - 3.0f * cm12) + 
                (3.0f * cm21 - cm03) * (3.0f * cm21 - cm03);

  features[4] = (cm30 + cm12) * (cm30 + cm12) + (cm21 + cm03) * (cm21 + cm03);

  features[5] = (cm30 - 3.0f * cm12) * (cm30 + cm12) * ((cm30 + cm12) * (cm30 + cm12) - 
                 3.0f * (cm21 + cm03) * (cm21 + cm03))  +  (3.0f * cm21 - cm03) * (cm21 + cm03) * 
                (3.0f * (cm30 + cm12) * (cm30 + cm12)   -  (cm21 + cm03) * (cm21 + cm03));

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
    





float   Raster::Moment (int32 p, int32 q)  const

{
  float m = 0.0f;
  float colMult = 1.0f;
  float rowMult = 1.0f;
 
  int32 col;
  int32 row;
  
  for (col = 0; col < width; col++)
  {
    colMult = pow ((float)col, p);

    for  (row = 0; row < height; row++)
    {
      if  (ForegroundPixel (green[row][col]))
      {
        rowMult = pow ((float)row, q);
        m += colMult * rowMult;
      }
    }
  }      

  return m;
}  /* Moment */



 



float  Raster::CenMoment (int32   p, 
                          int32   q, 
                          float   ew, 
                          float   eh
                         )  const
{
  float  cm = 0.0;
  int32   col;
  int32   row;

  float   deltaCol = 0.0f;
  float   colPow   = 0.0f;

  float   deltaRow = 0.0f;
  float   rowPow   = 1.0f;
 
  for (col = 0; col < width; col++)
  {
    deltaCol = col - ew;
    colPow = pow (deltaCol, p);

    for  (row = 0; row < height; row++)
    {
      if  (ForegroundPixel (green[row][col]))
      {
        deltaRow = row - eh;
        rowPow = pow (deltaRow, q);
        cm = cm + colPow * rowPow;
      }
    }
  }

 return cm;
}  /* CenMoment */




void  Raster::CentralMomentsWeighted (float  features[9])  const
{
  float m00 = MomentWeighted (0, 0);
  float ew  = MomentWeighted (1, 0) / m00;
  float eh  = MomentWeighted (0, 1) / m00;
        
  float cm00   = m00;
  float gamma2 = m00 * m00;
  float gamma3 = gamma2 * (float)sqrt (m00);
        
  float cm20 = CenMomentWeighted (2, 0, ew, eh) / gamma2;
  float cm02 = CenMomentWeighted (0, 2, ew, eh) / gamma2;
  float cm11 = CenMomentWeighted (1, 1, ew, eh) / gamma2;

  float cm30 = CenMomentWeighted (3, 0, ew, eh) / gamma3;
  float cm03 = CenMomentWeighted (0, 3, ew, eh) / gamma3;
  float cm12 = CenMomentWeighted (1, 2, ew, eh) / gamma3;
  float cm21 = CenMomentWeighted (2, 1, ew, eh) / gamma3;


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

//added by baishali to calculate eccentricity
  features[8] = (float)((((cm20 - cm02) * (cm20 - cm02)) - 
                (4.0 * cm11 * cm11))/((cm20 + cm02) * (cm20 + cm02)));

  return;
}  /* CentralMomentsWeighted */





float  Raster::MomentWeighted (int32 p, 
                               int32 q
                              )  const
{
  float m = (float)0.0;
 
  int32 col;
  int32 row;
  int32 k;

  uchar  pixVal;

  float  mult;


  for (col = 0; col < width; col++)
  {
    for  (row = 0; row < height; row++)
    {
      pixVal = green[row][col];

      if  (ForegroundPixel (pixVal))
      {
        mult = (float)1.0;

        for  (k = 0; k < p;  k++)
          mult *= col;

        for  (k = 0; k < q;  k++)
          mult *= row;

        m = m + mult * ((float)pixVal / (float)maxPixVal);
      }
    }
  }      

  return  m;
}  /* MomentWeighted */




float  Raster::CenMomentWeighted (int32  p, 
                                  int32  q, 
                                  float  ew, 
                                  float  eh
                                 )  const
{
  float  cm = 0.0;
  int32  col;
  int32  row;
  int32  k;
 
  uchar  pixVal;

  for (col = 0; col < width; col++)
  {
    float  deltaCol = col - ew;
    float  colPow = 1.0f;

    for  (k = 0; k < p; k++)
      colPow = colPow * deltaCol;


    for  (row = 0; row < height; row++)
    {
      pixVal = green[row][col];

      if  (ForegroundPixel (pixVal))
      {
        float  deltaRow = row - eh;

        float  rowPow = 1.0f;
        for  (k = 0; k < q; k++)
           rowPow = rowPow * deltaRow;

        cm = cm + colPow * rowPow * (float)pixVal / (float)maxPixVal;
      }
    }
  }

 return cm;
}  /* CenMomentWeighted2 */



/**
 *@brief Returns back a two dimension array that is a copy of the specified region.
 *@details The caller will take owneship of the two dimensional array creatd.
 */
uchar**  Raster::GetSubSet (uchar**  _src,
                            int32    _row,
                            int32    _col,
                            int32    _height,
                            int32    _width
                           )  const
{
  int32  endR = _row + _height - 1;
  int32  endC = _col + _width - 1;

  if  ((_row < 0)  ||  (endR >= height) ||
       (_col < 0)  ||  (endC >= width))
  {
    cerr << "***ERROR***, Raster::Raster  *** ERROR ***,  Index's Exceed Raster Bounds" << std::endl;
    cerr << "       Raster Dimnsions["       << width    << ", "  << height << "]."     << std::endl;
    cerr << "       Requested Coordinites [" << _row     << ", "  << _col      << "], "
         << "       Height["  << _height << "],  Width[" << _width << "]."
         << std::endl;
    //WaitForEnter ();
    exit (-1);
  }

  int32  row = 0;
  int32  col = 0;

  int32  totalPixelArea = _height * _width;
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
 *@see  Erosion, Dialation, Closing, Opening, MaskType
 */
bool  Raster::Fit (MaskTypes  mask,
                   int32      row, 
                   int32      col
                  )  const
{
  int32  bias = biases[mask];
  int32  r, c;
  int32  rStart = row - bias;
  int32  rEnd   = row + bias;
  int32  cStart = col - bias;
  int32  cEnd   = col + bias;

  if  (rStart  < 0)        rStart = 0;
  if  (rEnd    >= height)  rEnd = height - 1;
  if  (cStart  < 0)        cStart = 0;
  if  (cEnd    >= width)   cEnd = width - 1;

  if  (maskShapes[mask] == Square)
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
 *@brief  Used by morphological operators to determine the average pixel value of the foreground pixels that the specifies mask covers.
 *@see  Erosion, Dialation, Closing, Opening, MaskType
 */
uchar  Raster::Hit  (MaskTypes  mask,
                     int32      row,
                     int32      col
                    )  const
{
  if ((row                < biases[mask])   || 
      (row + biases[mask] >= height)        ||
      (col                < biases[mask])   || 
      (col + biases[mask] >= width))
  {
    //return 0;
  }

  uchar  pixVal = 0;
  int32  totPixVal = 0;
  int16  numOfHits = 0;

  int32  startRow = Max (row - biases[mask], (int32)0);
  int32  endRow   = Min (row + biases[mask], height - 1);
  int32  startCol = Max (col - biases[mask], (int32)0);
  int32  endCol   = Min (col + biases[mask], width - 1);

  int32  r, c;

  if  (maskShapes[mask] == Square)
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

  int32 idx = 0;
  for  (int32 row = 0; row < height; row++ )
  {
    for (int32 col = 0; col < width; col++ )
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
    mag = fourierMagArray[idx];                                                          // kk 2004-May-18

    // destData[idx] = (uchar)(dest[idx].re * maxPixVal / maxAmplitude);

    // kk  2004-May-18
    // Changed the above line to use the constant 255 instead of maxPixVal,  
    // If we have an image who's maxPixVal is less than 255 then the values
    // being calc'ed for the fourier features will not be consistent.
    // destData[idx] = (uchar)(dest[idx].re * 255 / maxAmplitude);
    destData[idx] = (uchar)(log (fourierMagArray[idx]) * 255.0f / maxAmplitudeLog);
  }

  delete  dest;      dest     = NULL;
  delete  destArea;  destArea = NULL;

  return  fourierImage;
}  /* FastFourierKK */





RasterPtr  Raster::FastFourier ()  const
{
  if  (!rasterInitialized)
    Initialize ();

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

  int32  col;
  int32  row;

  int32  idx = 0;

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

  RasterPtr fourierImage = AllocateARasterInstance (height, width, false);

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
    // being calc'ed for the fourier features will not be consistent.
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

  int32  leftStart   = (int32)(width  / 2);
  int32  bottomStart = (int32)(height / 2);

  int32  r1, c1;
  int32  r2, c2;

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
          << "*** ERROR ***    This Raster image is not the result of a fast fourier" << std::endl
          << std::endl;
     osWaitForEnter ();
     exit (-1);
  }

  float  cr = height / (float)2.0;
  float  cw = width  / (float)2.0;

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

  int32  count[5];

  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  count[3] = 0;
  count[4] = 0;

  int32  row;
  int32  col;

  float  deltaRow;
  float  deltaRowSqr = (float)0.0;

  float  deltaCol;
  float  deltaColSqr = (float)0.0;

  float  distFromCent;

  for  (row = 0 ; row < height; row++)
  {
    deltaRow = cr - (float)row;
    deltaRowSqr = deltaRow * deltaRow;

    for  (col = 0; col < (int32)cw; col++)
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

  int32  x;

  for  (x = 0; x < 5; x++)
  {
    fourierFeatures[x] = fourierFeatures[x] / count[x];
  }
}  /* ExtractFourierFeatures */



/**
 *@brief Returns the differenmyt between two angles in degrees.
 *@details  Givn two angles, will determine the angular distance
 * between them, going from the ang1 to ang2.  A positive
 * delta indicates that ang2 is infront of ang1, negative
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



void  Raster::CalcOrientationAndEigerRatio (float&  eigenRatio,
                                            float&  orientationAngle
                                           )  
{
#ifdef  WIN32

  float  centroidColWeighted;
  float  centroidRowWeighted;

  int32  size;
  int32  weight;

  CalcCentroid (size, weight, centroidRow, centroidCol, centroidRowWeighted, centroidColWeighted);

  double  cov[2][2];

  cov[0][0] = 0.0;
  cov[0][1] = 0.0;
  cov[1][0] = 0.0;
  cov[1][1] = 0.0;

  int32  col;
  uchar  pixVal;
  int32  row;

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

  // KAK 2013-07-07
  // The  ang0 MUST be either <0 or >0  or ==0   if all three conditions fail;  then ang0 is an invalid value;  in that case we will assume the image has no orientation.
  if  (!((ang0 < 0)  ||  (ang0 == 0)  ||  (ang0 > 0)))
  {
    ang0 = -OneHalfPie;
    eigenRatio = 1.0f;
  }

  orientationAngle = float (-ang0 - OneHalfPie);
  if  (fabs (orientationAngle) > OneHalfPie)
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
      orientationAngle = (float)OneHalfPie;
    }
  }

  return;

#endif
} /* CalcOrientation */




Point  DerivePreRotatedPoint (int32  height,
                              int32  width,
                              int32  rotatedRow,
                              int32  rotatedCol,
                              float  turnAngle
                             )
{
  int32  diag = (int32)sqrt ((float)(height * height + width * width)) + 10;

  float  a11 = (float)(cos (-turnAngle));
  float  a12 = (float)(sin (-turnAngle));
  float  b1  = width  * 0.5f;

  float  a21 = -a12;
  float  a22 = a11;
  float  b2  = height * 0.5f;

  int32  halfDiag = (diag + 1) / 2;

  int32  centDestRow = rotatedRow - halfDiag;
  int32  centDestCol = rotatedCol - halfDiag;

  int32  srcY = (int32)((float)(a21 * centDestCol) + (float)(a22 * centDestRow) + b2 + 0.5);
  int32  srcX = (int32)((float)(a11 * centDestCol) + (float)(a12 * centDestRow) + b1 + 0.5);

  return  Point (srcY, srcX);
}




RasterPtr  Raster::Rotate (float  turnAngle)
{

  int32  diag = (int32)sqrt ((float)(Height () * Height () + Width () * Width ())) + 10;
  int32  halfDiag = (diag + 1) / 2;

  //  J( x+width(J)/2, y+height(J)/2 )= I( A11x+A12y+b1, A21x+A22y+b2 ),
  //  C:\Program Files\OpenCV\docs\ref\OpenCVRef_ImageProcessing.htm
  //      Example. Using cvGetQuadrangeSubPix for image rotation


  RasterPtr  rotatedImage = AllocateARasterInstance (diag, diag, false);

  float  a11 = (float)(cos (-turnAngle));
  float  a12 = (float)(sin (-turnAngle));
  float  b1  = width  * 0.5f;

  float  a21 = -a12;
  float  a22 = a11;
  float  b2  = height * 0.5f;

  int32  centDestCol = 0;
  int32  centDestRow = 0;

  int32  srcCol;
  int32  srcRow;

  int32  destX;
  int32  destY;


  for  (centDestRow = -halfDiag; centDestRow <  halfDiag; centDestRow++)
  {
    for  (centDestCol = -halfDiag; centDestCol <  halfDiag; centDestCol++)
    {
      srcRow = (int32)((float)(a21 * centDestCol) + (float)(a22 * centDestRow) + b2 + 0.5);
      if  ((srcRow >= 0)  &&  (srcRow < height))
      {
        srcCol = (int32)((float)(a11 * centDestCol) + (float)(a12 * centDestRow) + b1 + 0.5);
   
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


void  Raster::FindBoundingBox (int32&  tlRow,
                               int32&  tlCol,
                               int32&  brRow,
                               int32&  brCol
                              )  const
{
  tlRow = INT_MAX;
  tlCol = INT_MAX;

  brRow = INT_MIN;
  brCol = INT_MIN;

  int32  col = 0;
  int32  row = 0;

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
    uchar*  rowData = green[row];

    while  (col < width)
    {
      if  (ForegroundPixel (rowData[col]))
      {
        if  (col < tlCol)
          tlCol = col;

        if  (col > brCol)
          brCol = col;

        if  (row > brRow)
          brRow = row;
      }
      ++col;
    }

    col = 0;
    ++row;
  }



/*

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

      int32  lastColUsed = 0;

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
*/

  return;

}  /* FindBoundingBox */  



void  Raster::CalcCentroid (int32&  size,
                            int32&  weight,
                            float&  rowCenter,  
                            float&  colCenter,
                            float&  rowCenterWeighted,
                            float&  colCenterWeighted
                           )
{
  size              = 0;
  weight            = 0;
  rowCenter         = 0;
  colCenter         = 0;
  rowCenterWeighted = 0;
  colCenterWeighted = 0;

  int32  row;  
  int32  col;

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



RasterPtr  Raster::CreateGrayScale ()  const
{
  if  (!color)
  {
    // Already a Grayscale image,  Just return copy of self.
    return  (AllocateARasterInstance (*this));
  }

  int32  h = height;
  int32  w = width;

  RasterPtr  grayRaster = AllocateARasterInstance (h, w, false);
  uchar* grayPtr  = grayRaster->GreenArea ();
  uchar* redPtr   = redArea;
  uchar* greenPtr = greenArea;
  uchar* bluePtr  = blueArea;

  for  (int32 x = 0;  x < totPixels;  ++x, ++grayPtr, ++redPtr, ++greenPtr, ++bluePtr)
  {
    *grayPtr = (uchar)((uint32)((0.30f) * (float)(*redPtr)    +  
                              (0.59f) * (float)(*greenPtr)  +  
                              (0.11f) * (float)(*bluePtr)
                             )
                      );
  }
  return  grayRaster;
}  /* CreateGrayScale */




HistogramPtr  Raster::HistogramGrayscale ()  const
{
  HistogramPtr histogram = new KKU::Histogram (0, 256, 1.0, false);

  int32  row, col;
 
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
    case  RedChannel:   data = redArea;    break;
    case  GreenChannel: data = greenArea;  break;
    case  BlueChannel:  data = blueArea;   break;
    }
  }

  if  (!data)
    data = greenArea;

  HistogramPtr histogram = new KKU::Histogram (0, 256, 1.0, false);
  for  (int32 x = 0;  x < totPixels;  x++)
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


  int32*  equalizedMapTable = equalizedHistogram->EqualizedMapTable ();

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

  int32  row, col;

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




void  Raster::DrawLine (int32 bpRow,    int32 bpCol,
                        int32 epRow,    int32 epCol
                       )
{
  DrawLine (bpRow, bpCol,  
            epRow, epCol,
            255
           );
}





void  Raster::DrawLine (int32 bpRow,    int32 bpCol,
                        int32 epRow,    int32 epCol,
                        uchar pixelVal
                       )
{
  DrawLine (bpRow, bpCol, epRow, epCol, pixelVal, pixelVal, pixelVal);
}  /* DrawLine */;




void  Raster::DrawLine (int32 bpRow,    int32 bpCol,
                        int32 epRow,    int32 epCol,
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
         << "Raster::DrawLine,  Out of Raster Boundries   Height[" << height << "]  width[" << width << "]."  << std::endl
         << "                   BeginPoint" << Point (bpRow, bpCol) << "   EndPoint" << Point (epRow, epCol) << "." << std::endl
         << std::endl;
    return;
  }

  int32  deltaY = epRow - bpRow;
  int32  deltaX = epCol - bpCol;

  int32  row, col;

  if  (deltaY == 0)
  {
    // We have a Straight Horizontal Line

    row = bpRow;
    
    int32  startCol;
    int32  endCol;

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
    
    int32  startRow;
    int32  endRow;

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
    int32  startCol;
    int32  endCol;

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
      row = (int32)(m * col + c + 0.5);
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
    int32  startRow;
    int32  endRow;

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
      col = (int32)(((row - c) / m) + 0.5);
      green[row][col] = g;
      if  (color)
      {
        red [row][col] = r;
        blue[row][col] = b;
      }
    }
  }
}  /* DrawLine */;


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



void  Raster::DrawLine (int32 bpRow,    int32 bpCol,
                        int32 epRow,    int32 epCol,
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
         << "Raster::DrawLine,  Out of Raster Boundries   Height[" << height << "]  width[" << width << "]."  << std::endl
         << "                   BeginPoint" << Point (bpRow, bpCol) << "   EndPoint" << Point (epRow, epCol) << "." << std::endl
         << std::endl;
    return;
  }

  int32  deltaY = epRow - bpRow;
  int32  deltaX = epCol - bpCol;

  int32  row, col;

  if  (deltaY == 0)
  {
    // We have a Straight Horizontal Line

    row = bpRow;
    
    int32  startCol;
    int32  endCol;

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
    
    int32  startRow;
    int32  endRow;

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

  float  c = bpRow - m * bpCol;

  if  (fabs (m) < 0.5)
  {
    int32  startCol;
    int32  endCol;

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
      row = (int32)(m * col + c + 0.5);
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
    int32  startRow;
    int32  endRow;

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
      col = (int32)(((row - c) / m) + 0.5);
      green[row][col] = MergeAlpfaBeta (alpha, g, beta, green[row][col]);
      if  (color)
      {
        red [row][col] = MergeAlpfaBeta (alpha, r, beta, red [row][col]);
        blue[row][col] = MergeAlpfaBeta (alpha, b, beta, blue[row][col]);
      }
    }
  }
}  /* DrawLine */;



void   Raster::DrawLine (int32  bpRow,    int32 bpCol,
                         int32  epRow,    int32 epCol,
                         PixelValue  pixelVal
                        )
{
  DrawLine (bpRow, bpCol, epRow, epCol, pixelVal.r, pixelVal.g, pixelVal.b);
}  /* DrawLine */;




void   Raster::DrawLine (int32  bpRow,    int32 bpCol,
                         int32  epRow,    int32 epCol,
                         PixelValue  pixelVal,
                         float       alpha
                        )
{
  DrawLine (bpRow, bpCol, epRow, epCol, pixelVal.r, pixelVal.g, pixelVal.b, alpha);
}  /* DrawLine */;




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
  DrawPointList (Point ((int32)0, (int32)0), borderPixs, pixelValue.r, pixelValue.g, pixelValue.b);
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
  DrawPointList (Point ((int32)0, (int32)0), borderPixs, redVal, greenVal, blueVal);
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

  int32  row, col;

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

  int32  row, col;

  for  (pIDX = borderPixs.begin ();  pIDX != borderPixs.end ();  pIDX++)
  {
    pixel = *pIDX;
    if  (lastPoint)
    {
      int32  deltaRow = lastPoint->Row () - pixel->Row ();
      int32  deltaCol = lastPoint->Col () - pixel->Col ();

      int32  distSquared = deltaRow * deltaRow  +  deltaCol * deltaCol;

      if  (distSquared > 5)
      {
        // We need to draw a line between the two points.
        DrawLine (*lastPoint + offset, *pixel + offset, linePixelValue);
      }
    }

    row = pixel->Row () + offset.Row ();
    col = pixel->Col () + offset.Col ();

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
                       const PixelValue&  color,
                       int32              size    // Diameter in Pixels of Dot
                      )
{
  SetPixelValue (point, color);
  
  double  radius = (double)size / 2.0;
  double  radiusSquared = radius * radius;

  int32  left  = Max ((int32)0,           (int32)floor ((double)point.Col () - radius));
  int32  right = Min ((int32)(width - 1), (int32)ceil  ((double)point.Col () + radius));

  int32  bot   = Max ((int32)0,            (int32)floor ((double)point.Row () - radius));
  int32  top   = Min ((int32)(height - 1), (int32)ceil  ((double)point.Row () + radius));

  int32  row, col;

  for  (row = bot;  row <= top;  row++)
  {
    int32  deltaRow = row - point.Row ();
    int32  deltaRowSquared = deltaRow * deltaRow;

    if  (deltaRowSquared <= radiusSquared)
    {
      for  (col = left;  col <= right;  col++)
      {
        int32  deltaCol = col - point.Col ();
        double  distFromCenterSquares = deltaRowSquared + deltaCol * deltaCol;

        if  (distFromCenterSquares <= radiusSquared)
        {
          // we are within the dot.
          SetPixelValue (row, col, color);
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
  int32  row, col;
  float  x, y;

  float  start = -(float)ceil  (radius);
  float  end   = +(float)floor (radius);

  float  radiusSquare = radius * radius;

  for  (x = start;  x <= end;  x++)
  {
    y = (float)sqrt (radiusSquare - (x * x));
    col = (int32)(x + centerCol + 0.5);
    row = (int32)(centerRow + 0.5 + y);
    SetPixelValue (row, col, pixelValue);

    row = (int32)(centerRow + 0.5 - y);
    SetPixelValue (row, col, pixelValue);


    row = (int32)(x + centerRow + 0.5);
    col = (int32)(centerCol + 0.5 + y);
    SetPixelValue (row, col, pixelValue);

    col = (int32)(centerCol + 0.5 - y);
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

    int32  adjRow = (int32)(centerRow + row + 0.5f);
    int32  adjCol = (int32)(centerCol + col + 0.5f);

    if  ((adjRow >= 0)  &&  (adjRow < height)  &&
         (adjCol >= 0)  &&  (adjCol < width))
      SetPixelValue ((int32)(centerRow + row + 0.5f), (int32)(centerCol + col + 0.5f), pixelValue);

    angle += angleIncrement;
  }
}  /* DrawCircle */





void  Raster::DrawCircle (const Point&       point,
                          int32              radius,
                          const PixelValue&  color
                         )
{
  DrawCircle ((float)point.Row (),
              (float)point.Col (),
              (float)radius,
              color
             );
}  /* DrawCircle */








void  Raster::SmoothImageChannel (uchar**  src,
                                  uchar**  dest,
                                  int32    maskSize
                                 )  const
{
  int32  row, col;

  int32  firstMaskRow, firstMaskCol;
  int32  lastMaskRow,  lastMaskCol;

  int32  maskRow,  maskCol;

  int32  maskOffset = maskSize / 2;
  
  for  (row = 0;  row < height;  row++)
  {
    firstMaskRow = row - maskOffset;
    lastMaskRow  = firstMaskRow + maskSize - 1;

    firstMaskRow = Max ((int32)0,    firstMaskRow);
    lastMaskRow  = Min (lastMaskRow, (int32)(height - 1));

    for  (col = 0;  col < width;  col++)
    {
      firstMaskCol = col - maskOffset;
      lastMaskCol = firstMaskCol + maskSize - 1;
      firstMaskCol = Max (firstMaskCol, (int32)0);
      lastMaskCol  = Min (lastMaskCol,  (int32)(width - 1));

      int32  total      = 0;
      int32  numOfCells = 0;

      for  (maskRow = firstMaskRow;  maskRow <= lastMaskRow;  maskRow++)
      {
        for  (maskCol = firstMaskCol;  maskCol <= lastMaskCol;  maskCol++)
        {
          total += src[maskRow][maskCol];
          numOfCells++;
        }
      }

      dest[row][col] = (uchar)((int32)((float)((float)total / (float)numOfCells) + 0.5f));
    }
  }

  return;
} /* SmoothImageChannel */





RasterPtr  Raster::CreateSmoothImage (int32 maskSize)  const
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
                 int32 arraySize, 
                 int32 Kth
                )
{
  T    pv;
  int32  left  = 0;
  int32  right = arraySize - 1;


  int32  pivotIndex = right;

  int32  partitionIndex = -1;

  T temp;

  while  (partitionIndex != Kth)
  {
    pv = values[pivotIndex];
    
    partitionIndex = left;
    for  (int32 i = left;  i < right;  i++)
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





RasterPtr  Raster::CreateSmoothedMediumImage (int32 maskSize)  const
{
  RasterPtr  result = AllocateARasterInstance (*this);
  if  (maskSize < 2)
    return result;

  uchar**  destR = result->Red   ();
  uchar**  destG = result->Green ();
  uchar**  destB = result->Blue  ();

  int32  maxCandidates = maskSize * maskSize;
  uchar*  candidatesRed   = NULL;
  uchar*  candidatesGreen = new uchar[maxCandidates];
  uchar*  candidatesBlue  = NULL;
  if  (color)
  {
    candidatesRed  = new uchar[maxCandidates];
    candidatesBlue = new uchar[maxCandidates];
  }

  int32  numCandidates = 0;
  int32  middleCandidate = 0;

  int32  row, col;

  int32  firstMaskRow, firstMaskCol;
  int32  lastMaskRow,  lastMaskCol;

  int32  maskRow,  maskCol;

  int32  maskOffset = maskSize / 2;

  //uchar*  srcRow = NULL;
  
  for  (row = 0;  row < height;  row++)
  {
    firstMaskRow = row - maskOffset;
    lastMaskRow  = firstMaskRow + maskSize - 1;

    firstMaskRow = Max ((int32)0, firstMaskRow);
    lastMaskRow  = Min (lastMaskRow, (int32)(height - 1));
    
    for  (col = 0;  col < width;  col++)
    {
      firstMaskCol = col - maskOffset;
      lastMaskCol = firstMaskCol + maskSize - 1;
      firstMaskCol = Max (firstMaskCol, (int32)0);
      lastMaskCol  = Min (lastMaskCol,  (int32)(width - 1));

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

  delete  candidatesRed;    candidatesRed   = NULL;
  delete  candidatesGreen;  candidatesGreen = NULL;
  delete  candidatesBlue;   candidatesBlue  = NULL;

  return  result;
} /* CreateSmoothedMediumImage */






RasterPtr  Raster::HalfSize ()
{
  int32  hHeight = int32 (height / 2);
  int32  hWidth  = int32 (width  / 2);

  RasterPtr  halfSize = AllocateARasterInstance (hHeight, hWidth, color);

  int32  row = 0;
  int32  col = 0;
  int32  hRow, hCol;
  for  (hRow = 0;  hRow < hHeight;  hRow++)
  {
    col = 0;
    for  (hCol = 0;  hCol < hWidth;  hCol++)
    {
      halfSize->SetPixelValue (hRow, hCol, green[row][col]);
      if  (color)
      {
        halfSize->SetPixelValue (RedChannel,  hRow, hCol, red [row][col]);
        halfSize->SetPixelValue (BlueChannel, hRow, hCol, blue[row][col]);
      }

      col += 2;
    }

    row += 2;
  }

  return  halfSize;
}  /* HalfSize */





RasterPtr  Raster::ReduceByEvenMultiple (int32  multiple)  const
{
  // We will pad one extra pixel top, bot, left, and right.
  // This is nessasary because some feature calcs assume that there edge rows are empty.

  int32  nHeight = int32 (height / multiple) + 2;
  int32  nWidth  = int32 (width  / multiple) + 2; 

  int32  row = 0;
  int32  col = 0;
  int32  nRow, nCol;

  uint32**  workRaster  = new uint32*[nHeight];
  uchar**   workDivisor = new uchar*[nHeight];
  uint32*   workRow     = NULL;

  uchar*  workDivisorRow = NULL;

  for  (nRow = 0;  nRow < nHeight;  nRow++)
  {
    workRow = new uint32[nWidth];
    workRaster[nRow] = workRow;

    workDivisorRow = new uchar[nWidth];
    workDivisor[nRow] = workDivisorRow;

    for  (nCol = 0;  nCol < nWidth;  nCol++)
    {
      workRow[nCol] = 0;
      workDivisorRow[nCol] = 0;
    }
  }

  nRow = 1;
  int32  intermediateRow = 0;
  int32  intermediateCol = 0;
  uchar*  srcRow = NULL;

  for  (row = 0;  row < height;  row++)
  {
    srcRow = green[row];
    intermediateCol = 0;
    nCol = 1;
    workRow = workRaster[nRow];
    workDivisorRow = workDivisor[nRow];

    for  (col = 0;  col < width;  col++)
    {
      workRow[nCol] += srcRow[col];
      workDivisorRow[nCol]++;

      intermediateCol++;
      if  (intermediateCol >= multiple)
      {
        intermediateCol = 0;
        nCol++;
      }
    }

    intermediateRow++;
    if  (intermediateRow >= multiple)
    {
      intermediateRow = 0;
      nRow++;
    }
  }

  RasterPtr  reducedRaster = AllocateARasterInstance (nHeight, nWidth, false);

  uchar*  destRow = NULL;

  int32  newPixelVal           = 0;
  int32  nMaxPixVal            = 0;
  int32  nForegroundPixelCount = 0;

  for  (nRow = 0;  nRow < nHeight;  nRow++)
  {
    destRow = (reducedRaster->Green ())[nRow];
    workRow = workRaster[nRow];
    workDivisorRow = workDivisor[nRow];

    for  (nCol = 0;  nCol < nWidth;  nCol++)
    {
      newPixelVal = workRow[nCol];
      if  (newPixelVal > 0)  
      {
        nForegroundPixelCount++;
        newPixelVal = (int32)(0.5f + (float)(newPixelVal) / (float)(workDivisorRow[nCol]));
        destRow[nCol] = (uchar)(newPixelVal);
        if  (newPixelVal > nMaxPixVal)
          nMaxPixVal = newPixelVal;
      }
    }

    delete  workRaster[nRow];  
    delete  workDivisor[nRow];
    workRaster[nRow]  = NULL;
    workDivisor[nRow] = NULL;
  }
  delete  workRaster;
  workRaster = NULL;
  delete  workDivisor;
  workDivisor = NULL;

  reducedRaster->ForegroundPixelCount (nForegroundPixelCount);
  reducedRaster->MaxPixVal ((uchar)nMaxPixVal);

  return  reducedRaster;
}  /* ReduceByEvenMultiple */



RasterPtr  Raster::ReduceByFactor (float factor)  const  //  0 < factor <= 1.0
{
  if  (factor <= 0.0f)  
    factor = 0.1f;
  
  else if  (factor > 1.0f)
    factor = 1.0f;

  int32  c, r;

  int32  newR;

  int32  newHeight = (int32)(height * factor + 0.5f);
  int32  newWidth  = (int32)(width  * factor + 0.5f);
  if  (newHeight < 2)
    newHeight = 2;

  if  (newWidth < 2)
    newWidth = 2;

  int32  newTotal  = newHeight * newWidth;

  float*  accumulatorAreaGreen = new float[newTotal];
  float*  accumulatorAreaRed   = NULL;
  float*  accumulatorAreaBlue  = NULL;
  if  (color)
  {
    accumulatorAreaRed   = new float[newTotal];
    accumulatorAreaBlue  = new float[newTotal];
    memset (accumulatorAreaRed,  0, newTotal * sizeof (float));
    memset (accumulatorAreaBlue, 0, newTotal * sizeof (float));
  }

  float*  divisorArea          = new float[newTotal];

  memset (accumulatorAreaGreen, 0, newTotal * sizeof (float));
  memset (divisorArea,          0, newTotal * sizeof (float));

  float** accumulatorRed   = NULL;
  float** accumulatorGreen = new float*[newHeight];
  float** accumulatorBlue  = NULL;

  if  (color)
  {
    accumulatorRed  = new float*[newHeight];
    accumulatorBlue = new float*[newHeight];
  }


  float** divisor   = new float*[newHeight];

  float*  rowFactor = new float[height + 1];
  float*  colFactor = new float[width  + 1];

  for  (r = 0;  r < height;  r++)
    rowFactor[r] = r * factor;
  rowFactor[height] = (float)newHeight;

  for  (c = 0;  c < width;  c++)
    colFactor[c] = c * factor;
  colFactor[width] = (float)newWidth;

  float*  arPtr = accumulatorAreaRed;
  float*  agPtr = accumulatorAreaGreen;
  float*  abPtr = accumulatorAreaBlue;
  float*  daPtr = divisorArea;
  for  (newR = 0;  newR < newHeight;  newR++)
  {
    accumulatorGreen [newR] = agPtr;
    divisor          [newR] = daPtr;
    agPtr += newWidth;
    daPtr += newWidth;

    if  (color)
    {
      accumulatorRed [newR] = arPtr;
      accumulatorBlue[newR] = abPtr;
      arPtr += newWidth;
      abPtr += newWidth;
    }
  }

  uchar  rValue = 0, gValue = 0, bValue = 0;

  for  (r = 0;  r < height;  r++)
  {
    int32  thisRow = (int32)rowFactor[r];
    if  (thisRow >= newHeight)
      thisRow = newHeight - 1;

    int32  nextRow = (int32)rowFactor[r + 1];
    if  (nextRow >= newHeight)
      nextRow = newHeight - 1;

    float  amtThisRow = 1.0f;
    float  amtNextRow = 0.0f;

    if  (nextRow > thisRow)
    {
      amtThisRow = (float)nextRow - rowFactor[r];
      amtNextRow = 1.0f - amtThisRow;
    }

    for  (c = 0;  c < width;  c++)
    {
      gValue = green[r][c];
      if  (color)
      {
        rValue = red [r][c];
        bValue = blue[r][c];
      }

      int32  thisCol = (int32)colFactor[c];
      if  (thisCol >= newWidth)
        thisCol = newWidth - 1;

      int32  nextCol = (int32)colFactor[c + 1];
      if  (nextCol >= newWidth)
        nextCol = newWidth - 1;

      float  amtThisCol = 1.0f;
      float  amtNextCol = 0.0f;

      if  (nextCol > thisCol)
      {
        amtThisCol = (float)nextCol - colFactor[c];
        amtNextCol = 1.0f - amtThisCol;
      }

      accumulatorGreen[thisRow][thisCol] += gValue * amtThisRow * amtThisCol;
      if  (color)
      {
        accumulatorRed [thisRow][thisCol] += rValue * amtThisRow * amtThisCol;
        accumulatorBlue[thisRow][thisCol] += bValue * amtThisRow * amtThisCol;
      }

      divisor   [thisRow][thisCol] += amtThisRow * amtThisCol;

      if  (nextRow > thisRow)
      {
        accumulatorGreen[nextRow][thisCol] += gValue * amtNextRow * amtThisCol;
        if  (color)
        {
          accumulatorRed [nextRow][thisCol] += rValue * amtNextRow * amtThisCol;
          accumulatorBlue[nextRow][thisCol] += bValue * amtNextRow * amtThisCol;
        }
        divisor   [nextRow][thisCol] += amtNextRow * amtThisCol;

        if  (nextCol > thisCol)
        {
          accumulatorGreen[nextRow][nextCol] += gValue * amtNextRow * amtNextCol;
          if  (color)
          {
            accumulatorRed [nextRow][nextCol] += rValue * amtNextRow * amtNextCol;
            accumulatorBlue[nextRow][nextCol] += bValue * amtNextRow * amtNextCol;
          }
          divisor   [nextRow][nextCol] += amtNextRow * amtNextCol;
        }
      }
      else
      {
        if  (nextCol > thisCol)
        {
          accumulatorGreen[thisRow][nextCol] += gValue * amtThisRow * amtNextCol;
          if  (color)
          {
            accumulatorRed [thisRow][nextCol] += rValue * amtThisRow * amtNextCol;
            accumulatorBlue[thisRow][nextCol] += bValue * amtThisRow * amtNextCol;
          }
          divisor   [thisRow][nextCol] += amtThisRow * amtNextCol;
        }
      }
    }  /*  for (c)  */
  }  /*  for (r)  */

  int32  x;
  RasterPtr  reducedRaster = AllocateARasterInstance (newHeight, newWidth, color);
  uchar*  newRedArea   = reducedRaster->RedArea   ();
  uchar*  newGreenArea = reducedRaster->GreenArea ();
  uchar*  newBlueArea  = reducedRaster->BlueArea  ();
  for  (x = 0;  x < newTotal;  x++)
  {
    if  (divisorArea[x] == 0.0f)
    {
      newGreenArea[x] = 0;
      if  (color)
      {
        newRedArea [x] = 0;
        newBlueArea[x] = 0;
      }
    }
    else
    {
      newGreenArea[x] = (uchar)(accumulatorAreaGreen[x] / divisorArea[x] + 0.5f);
      if  (color)
      {
        newRedArea [x] = (uchar)(accumulatorAreaRed [x] / divisorArea[x] + 0.5f);
        newBlueArea[x] = (uchar)(accumulatorAreaBlue[x] / divisorArea[x] + 0.5f);
      }
    }
     
  }

  delete  accumulatorAreaRed;    accumulatorAreaRed   = NULL;
  delete  accumulatorAreaGreen;  accumulatorAreaGreen = NULL;
  delete  accumulatorAreaBlue;   accumulatorAreaBlue  = NULL;

  delete  divisorArea;       divisorArea    = NULL;

  delete  accumulatorRed;    accumulatorRed    = NULL;
  delete  accumulatorGreen;  accumulatorGreen  = NULL;
  delete  accumulatorBlue;   accumulatorBlue   = NULL;
  delete  divisor;           divisor           = NULL;

  delete  rowFactor;       rowFactor      = NULL;
  delete  colFactor;       colFactor      = NULL;

  return  reducedRaster;
}  /* ReduceByFactor */




RasterPtr  Raster::SobelEdgeDetector ()
{
  Sobel  sobel (this);
  return sobel.MagnitudeImage ();
}  /* SobelEdgeDetector */




RasterPtr  Raster::BinarizeByThreshold (uchar  min,
                                        uchar  max
                                       )
{
  RasterPtr result = AllocateARasterInstance (height, width, false);

  uchar*  resultGreenArea = result->GreenArea ();

  for  (int32 x = 0;  x < totPixels;  x++)
  {
    if  ((greenArea[x] >= min)  &&  (greenArea[x] <= max))
      resultGreenArea[x] = foregroundPixelValue;
    else
      resultGreenArea[x] = backgroundPixelValue;
  }

  return  result;
}  /* BinarizeByThreshold */




RasterPtr  Raster::ExtractChannel (ColorChannels  channel)
{
  if  (!color)
    return  AllocateARasterInstance (*this);

  RasterPtr  r = AllocateARasterInstance (height, width, false);

  uchar*  src = NULL;

  KKStr  rootName = osGetRootName (FileName ());

  if  (channel == RedChannel)
  {
    r->FileName (rootName + "_Red.bmp") ;
    src = RedArea ();
  }

  else if  (channel == GreenChannel)
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




//******************************************************************************
//*  Wrote this method to deel with polution sample particles, meeded to help  *
//* segment out particles from a verynoisy background.                         *
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

  int32  r, c;

  float  threshold;

  float  backgroundValue;
  float  forgroundValue;

  RasterPtr  gsImage = NULL;

  if  (Color ())
  {
    gsImage = CreateGrayScale ();

    RasterPtr  redPart    = ExtractChannel (RedChannel);
    RasterPtr  greenPart  = ExtractChannel (GreenChannel);
    RasterPtr  bluePart   = ExtractChannel (BlueChannel);

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

    int32  totalOfPixelVals = 0;
    int32  count = 0;
 
    int32  lastRow = Height () - 1;
    int32  lastCol = Width () - 1;

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
    SaveImage (*mask, baseName + "_Mask_" + StrFormatInt ((int32)threshold, "zz0") + ".bmp");

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
          destRaster->SetPixelValue (r, c, 255 - gsImage->GetPixelValue (r, c));
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




RasterListPtr  Raster::SplitImageIntoEqualParts (int32 numColSplits,
                                                 int32 numRowSplits
                                                )  const
{
  if  ((numColSplits < 1)  ||  (numColSplits >= width))
    return NULL;

  if  ((numRowSplits < 1)  ||  (numRowSplits >= height))  
    return NULL;

  int32  numOfParts = numColSplits * numRowSplits;

  RasterListPtr  parts = new RasterList (true, numOfParts);

  int32  partStartingCol = 0;
  int32  partStartingRow = 0;
 
  // becuase the uimage might not divide evenly we may have to do some
  // adjusting,  posibly loosing some rows and columns off the edges.
  int32  partWidth  = (int32)((float)(width  / numColSplits) + 0.5);
  int32  colsNeeded = numColSplits * partWidth;
  if  (colsNeeded > width)
  {
    partWidth--;
    partStartingCol = (width - (partWidth * numColSplits)) / 2;
  }

  int32  partHeight = (int32)((float)(height  / numRowSplits) + 0.5);
  int32  rowsNeeded = numRowSplits * partHeight;
  if  (rowsNeeded > height)
  {
    partWidth--;
    rowsNeeded = numRowSplits * partHeight;
    partStartingRow = (height -rowsNeeded) / 2;
  }

  int32  splitRow;
  int32  splitCol;

  int32  partStartRow = partStartingRow;

  for  (splitRow = 0;  splitRow < numRowSplits;  splitRow++)
  {
    int32  partEndRow = partStartRow + partHeight - 1;

    int32  partStartCol = partStartingCol;

    for  (splitCol = 0;  splitCol < numColSplits;  splitCol++)
    {
      int32  partEndCol = partStartCol + partWidth - 1;

      RasterPtr  part = new Raster (*this, partStartRow, partStartCol, partHeight, partWidth);

      parts->PushOnBack (part);

      partStartCol = partEndCol + 1;
    }

    partStartRow = partEndRow + 1;
  }

  return  parts;
}  /* SplitImageIntoEqualParts */



void  Raster::ErodeSpurs ()
{
  Raster  origRaster (*this);

  uchar**  origGreen = origRaster.green;

  int32  r;
  int32  c;

  int32  firstRow = 1;
  int32  firstCol = 1;
  int32  lastRow  = height - 1;
  int32  lastCol  = width - 1;

  for  (r = firstRow; r < lastRow; r++)
  {
    for  (c = firstCol; c < lastCol; c++)
    {
      if  (ForegroundPixel (green[r][c]))
      {
        // We have a forground Pixel.

        if  ((BackgroundPixel (origGreen[r - 1][c - 1]))  &&
             (BackgroundPixel (origGreen[r - 1][c]    ))  &&
             (BackgroundPixel (origGreen[r - 1][c + 1]))  &&
             (BackgroundPixel (origGreen[r    ][c - 1]))  &&
             (BackgroundPixel (origGreen[r    ][c + 1])))
        {
          // Top Spur
          green[r][c] = backgroundPixelValue;
        }

        else
        if  ((BackgroundPixel (origGreen[r - 1][c - 1]))  &&
             (BackgroundPixel (origGreen[r    ][c - 1]))  &&
             (BackgroundPixel (origGreen[r + 1][c + 1]))  &&
             (BackgroundPixel (origGreen[r - 1][c    ]))  &&
             (BackgroundPixel (origGreen[r + 1][c    ])))
        {
          // Left Spur
          green[r][c] = backgroundPixelValue;
        }

        else
        if  ((BackgroundPixel (origGreen[r + 1][c - 1]))  &&
             (BackgroundPixel (origGreen[r + 1][c    ]))  &&
             (BackgroundPixel (origGreen[r + 1][c + 1]))  &&
             (BackgroundPixel (origGreen[r    ][c - 1]))  &&
             (BackgroundPixel (origGreen[r    ][c + 1])))
        {
          // Bottom Spur
          green[r][c] = backgroundPixelValue;
        }

        else
        if  ((BackgroundPixel (origGreen[r - 1][c + 1]))  &&
             (BackgroundPixel (origGreen[r    ][c + 1]))  &&
             (BackgroundPixel (origGreen[r + 1][c + 1]))  &&
             (BackgroundPixel (origGreen[r - 1][c    ]))  &&
             (BackgroundPixel (origGreen[r + 1][c    ])))
        {
          // Right Spur
          green[r][c] = backgroundPixelValue;
        }
      }
    }
  }
}  /* ErodeSpurs */




//****************************************************************
//* The following ThinningCode was lifted out of a IPL98 
//* Library and modified to conform with this object.
//***************************************************************
//
//   The Image Processing Library 98, IPL98    
//   by René Dencker Eriksen - edr@mip.sdu.dk
//
//  from module   "~\ipl98\source\ipl98\kernel_c\algorithms\kernel_morphology.c"
//

//#define  DEBUG_ThinContour

#if  defined(DEBUG_ThinContour)
  int32  rasterGlobalHeight = 0;
  int32  rasterGlobalWidth  = 0;
#endif


bool k_ThinningStep2cdTests     (uchar m_Matrix22[][3]);

bool k_ThinningStep1cdTests     (uchar m_Matrix22[][3]);

bool k_ThinningCheckTransitions (uchar m_Matrix22[][3]);

bool k_ThinningSearchNeighbors  (int32    x, 
                                 int32    y,
                                 uchar**  g, 
                                 uchar    m_Matrix22[][3]
                                );

RasterPtr  Raster::ThinContour ()
{
  #if  defined(DEBUG_ThinContour)
  cout << std::endl << std::endl 
       << "Raster::ThinContour" << std::endl
       << std::endl;

    rasterGlobalHeight = height;
    rasterGlobalWidth  = width;
  #endif


  bool   PointsRemoved = false;
  uchar  m_Matrix22[3][3];

  int32  Iter = 0;
  int32  prem1;
  int32  prem2;
  int32  iCountX, iCountY;
  int32  pntinpic=0;

  
  PointList  pointList  (true);   // Owns the Point Objects,  init array to 1000
  PointList  removeList (true);   // Owns the Point Objects,  init array to 1000

  //ContourFollower contourFollwer (*this);
  //PointListPtr  borderPixs = contourFollower.GenerateContourList (blob);

  RasterPtr workRaster = AllocateARasterInstance (*this);
  workRaster->ConnectedComponent (3);

  uchar**   workGreen  = workRaster->Green ();
  workRaster->ErodeSpurs ();
  // workRaster->Dialation ();

  //   k_SetBorder(1,1,pImg);
  PointsRemoved = false;
  Iter++;


  /* step 1 Collecting the Black point in a list */
  prem1 = prem2 = 0;


  int32  minCol, maxCol, minRow, maxRow;

  workRaster->FindBoundingBox (minRow,
                               minCol,
                               maxRow,
                               maxCol
                              );

  if  ((minRow > maxRow)  ||  (minRow < 0)  ||  (minCol < 0))
  {
    #if  defined(DEBUG_ThinContour)
      cout << std::endl << std::endl 
           << "Raster::ThinContour    'FindBoundingBox'" << std::endl
           << "                        minRow[" << minRow << "]  maxRow[" << maxRow << "]" << std::endl
           << std::endl;
      cout.flush ();
    #endif
    // We must have a empty raster.  In this case there is nothing else we can do.
    return  workRaster;
  }

  for  (iCountY = minRow;  iCountY <= maxRow;  iCountY++)
  {
    minCol = 999999;
    maxCol = -1;

    for  (int32 x = 0; x < width; x++)
    {
      if  (ForegroundPixel (workGreen[iCountY][x]))
      {
        maxCol = Max (maxCol, x);
        minCol = Min (minCol, x);
      }
    }

    for  (iCountX = minCol;  iCountX <= maxCol;  iCountX++)
    {
      if  (ForegroundPixel (workGreen[iCountY][iCountX]))
      {
        PointPtr tempPoint = new Point (iCountY, iCountX);
        pntinpic++;

        if (k_ThinningSearchNeighbors  (iCountX, iCountY, workGreen, &m_Matrix22[0])  &&
            k_ThinningCheckTransitions (&m_Matrix22[0])                               &&
            k_ThinningStep1cdTests     (&m_Matrix22[0])
           )
        {
          prem1++;
          PointsRemoved = true;
          removeList.PushOnBack (tempPoint);
        }
        else
        {
          pointList.PushOnBack (tempPoint);
        }
      }
    }
  }

  #if  defined(DEBUG_ThinContour)
    cout << "Total black points:" << pntinpic << "\n";
    cout.flush ();
  #endif


  /* Set all pixels positions in RemoveList in image to white */
  {
    PointPtr  pixel = removeList.PopFromFront ();
    while  (pixel)
    {
      workGreen[pixel->Row ()][pixel->Col ()] = backgroundPixelValue;
      delete pixel;
      pixel = removeList.PopFromFront ();
    }
  }
  
  removeList.DeleteContents ();


  /* step 2 after step 1 which inserted points in list */
  if  (PointsRemoved)
  {
    #if  defined(DEBUG_ThinContour)
      cout << "PointsRemoved = true" << pntinpic << "\n";
      cout.flush ();
    #endif

    PointPtr  tempPoint = NULL;

    for  (iCountX = 0; iCountX < pointList.QueueSize ();  iCountX++)
    {
      tempPoint = pointList.IdxToPtr (iCountX);
      if  (tempPoint == NULL)
        continue;

      if  (k_ThinningSearchNeighbors  (tempPoint->Col (), tempPoint->Row (),  workGreen, &m_Matrix22[0])  &&
           k_ThinningCheckTransitions (&m_Matrix22[0])                                                    &&
           k_ThinningStep2cdTests     (&m_Matrix22[0])
          )
      {
        prem2++;
        PointsRemoved = true;
       
        //pointList.DeleteEntry (iCountX);
        pointList.SetIdxToPtr (iCountX, NULL); 
        removeList.PushOnBack (tempPoint);
        //iCountX--; /* Must decrease iCountX when a point has been removed */
      }
    }
  }

  /* Set all pixels positions in RemoveList in image to white */
  {
    PointPtr  pixel = removeList.PopFromFront ();
    while  (pixel)
    {
      workGreen[pixel->Row ()][pixel->Col ()] = backgroundPixelValue;
      delete pixel;
      pixel = removeList.PopFromFront ();
    }
  }

  removeList.DeleteContents ();
  #if  defined(DEBUG_ThinContour)
    cout  << "Iteration " << Iter << ": Points removed: " << prem1 << " + " << prem2 << " = " << prem1+prem2 << "\n";
    cout.flush ();
  #endif

  #if  defined(DEBUG_ThinContour)
    cout << std::endl << "ThinContour  Starting Step 1    PointsRemoved[" << (PointsRemoved?"True":"False") << "]" << "\n";
    cout.flush ();
  #endif
  /* step 1 */
  while  (PointsRemoved)
  {
    PointPtr  tempPoint = NULL;

    prem1 = prem2 = 0;

    Iter++;
    PointsRemoved = false;

    for  (iCountX = 0; iCountX < pointList.QueueSize (); iCountX++)
    {
      tempPoint = pointList.IdxToPtr (iCountX);
      if  (tempPoint == NULL)
        continue;
      
      if  ((k_ThinningSearchNeighbors  (tempPoint->Col (), tempPoint->Row (),  workGreen, &m_Matrix22[0]))  &&
           (k_ThinningCheckTransitions (&m_Matrix22[0]))                                                    &&
           (k_ThinningStep1cdTests     (&m_Matrix22[0]))
          )
      {
        prem1++;
        PointsRemoved = true;
        
        /*k_RemovePosFromGroupSlow(iCountX,&PointList);*/

        //pointList.DeleteEntry (iCountX);
        pointList.SetIdxToPtr (iCountX, NULL);
        removeList.PushOnBack (tempPoint);
        //iCountX--;  /* Must decrease iCountX when a point has been removed */
      }
    }
    
    /* Set all pixels positions in Remove List in image to white */
    #if  defined(DEBUG_ThinContour)
      cout << "Set all pixels positions in Remove List in image to white.   removeList.size()=[" << removeList.size () << "]" << "\n";
      cout.flush ();
    #endif

    {
      PointPtr  pixel = removeList.PopFromFront ();
      while  (pixel)
      {
        workGreen[pixel->Row ()][pixel->Col ()] = backgroundPixelValue;
        delete pixel;
        pixel = removeList.PopFromFront ();
      }

      removeList.DeleteContents ();
    }


    #if  defined(DEBUG_ThinContour)
      cout << "ThinContour  Starting Step 2" << "\n";
      cout.flush ();
    #endif
    /* step 2 */
    for  (iCountX = 0; iCountX < pointList.QueueSize (); iCountX++)
    {
       tempPoint = pointList.IdxToPtr (iCountX);
       if  (tempPoint == NULL)
         continue;

       if  (k_ThinningSearchNeighbors  (tempPoint->Col (), tempPoint->Row (), workGreen, &m_Matrix22[0])   &&
            k_ThinningCheckTransitions (&m_Matrix22[0]) &&
            k_ThinningStep2cdTests     (&m_Matrix22[0])
           )
       {
         prem2++;
         PointsRemoved = true;

         /*k_RemovePosFromGroupSlow(iCountX,&PointList);*/
         //pointList.DeleteEntry (iCountX);
         pointList.SetIdxToPtr (iCountX, NULL);
         removeList.PushOnBack (tempPoint);
         //iCountX--; /* Must decrease iCountX when a point has been removed */
       }
    }


    #if  defined(DEBUG_ThinContour)
      cout << "ThinContour  LastStep in loop" << "\n";
      cout.flush ();
    #endif

    /* Set all pixels positions in RemoveList in image to white */
    {
      PointPtr  pixel = removeList.PopFromFront ();
      while  (pixel)
      {
        workGreen[pixel->Row ()][pixel->Col ()] = backgroundPixelValue;
        delete pixel;
        pixel = removeList.PopFromFront ();
      }

      removeList.DeleteContents ();
    }

    #if  defined(DEBUG_ThinContour)
      cout << "Iteration " << Iter << ": Points removed: " << prem1 << " + " << prem2 << " = " << prem1 + prem2 << "\n";
      cout.flush ();
    #endif

  }

  #if  defined(DEBUG_ThinContour)
    cout << "ThinContour   Ready to Exit;  going to DeleteContents of 'pointList' and 'removeList'." << "\n";
    cout.flush ();
  #endif


  pointList.DeleteContents ();
  removeList.DeleteContents ();

  #if  defined(DEBUG_ThinContour)
    cout << "ThinContour   Exiting'." << "\n";
    cout.flush ();
  #endif

    return  workRaster;
}  /* ThinContour */







/* performes the tests (c') and (d') in step 2 as explained in Gonzales and Woods page 493 */

bool  k_ThinningStep2cdTests (uchar m_Matrix22[][3])
{
  if ((m_Matrix22[1][0] + m_Matrix22[2][1] + m_Matrix22[0][1]) &&
      (m_Matrix22[1][0] + m_Matrix22[1][2] + m_Matrix22[0][1])
     )
    return true;
  else
    return false;
}



/* performes the tests (c) and (d) in step 1 as explained in Gonzales and Woods page 492 */

bool  k_ThinningStep1cdTests (uchar  m_Matrix22[][3])
{
  if ((m_Matrix22[1][0] + m_Matrix22[2][1] + m_Matrix22[1][2]) &&
      (m_Matrix22[2][1] + m_Matrix22[1][2] + m_Matrix22[0][1])
     )
    return true;
  else
    return false;
}



/* returns true if there is exactly one transition in the region around the actual pixel */
bool k_ThinningCheckTransitions(uchar  m_Matrix22[][3])
{
  int32 iTransitions=0;

  if ((m_Matrix22[0][0]==1) && (m_Matrix22[1][0]==0)){
    ++iTransitions;}

  if ((m_Matrix22[1][0]==1) && (m_Matrix22[2][0]==0)){
    ++iTransitions;}

  if ((m_Matrix22[2][0]==1) && (m_Matrix22[2][1]==0)){
    ++iTransitions;}

  if ((m_Matrix22[2][1]==1) && (m_Matrix22[2][2]==0)){
    ++iTransitions;}
  
  if ((m_Matrix22[2][2]==1) && (m_Matrix22[1][2]==0)){
    ++iTransitions;}
  
  if ((m_Matrix22[1][2]==1) && (m_Matrix22[0][2]==0)){
    ++iTransitions;}

  if ((m_Matrix22[0][2]==1) && (m_Matrix22[0][1]==0)){
    ++iTransitions;}
  
  if ((m_Matrix22[0][1]==1) && (m_Matrix22[0][0]==0)){
    ++iTransitions;}

  if (iTransitions==1)
    return true;
  else
    return false;
}  /* k_ThinningCheckTransitions */





bool  k_ThinningSearchNeighbors  (int32   x,   // collumn
                                  int32   y,   // row
                                  uchar** g, 
                                  uchar   m_Matrix22[][3]
                                 )
/* As (a) in Gonzales and Woods, between 2 and 6 black neighbors */
{
  #if  defined(DEBUG_ThinContour)
    if  ((x < 1)  ||  (x >= (rasterGlobalWidth - 1)))
    {
      cout << "\n"
           << "k_ThinningSearchNeighbors    x[" << x << "] is to close to the edge." << "\n"
           << "\n";
    }

    if  ((y < 1)  ||  (y >= (rasterGlobalHeight - 1)))
    {
      cout << "\n"
           << "k_ThinningSearchNeighbors    y[" << y << "] is to close to the edge." << "\n"
           << "\n";
    }
  #endif


  int32  BlackNeighbor = 0;
  //added by baishali 
  if (y ==0 || x==0)
  { }
  else
  {
  m_Matrix22[0][0] = (g[y - 1][x - 1] > 0) ? 0:1;
  m_Matrix22[1][0] = (g[y - 1][x    ] > 0) ? 0:1;
  m_Matrix22[2][0] = (g[y - 1][x + 1] > 0) ? 0:1;
  m_Matrix22[0][1] = (g[y    ][x - 1] > 0) ? 0:1;
  m_Matrix22[2][1] = (g[y    ][x + 1] > 0) ? 0:1;
  m_Matrix22[0][2] = (g[y + 1][x - 1] > 0) ? 0:1;
  m_Matrix22[1][2] = (g[y + 1][x    ] > 0) ? 0:1;
  m_Matrix22[2][2] = (g[y + 1][x + 1] > 0) ? 0:1;
  m_Matrix22[1][1] = (g[y    ][x    ] > 0) ? 0:1;
  }

  if (m_Matrix22[0][0] == 0) {++BlackNeighbor;}
  if (m_Matrix22[1][0] == 0) {++BlackNeighbor;}
  if (m_Matrix22[2][0] == 0) {++BlackNeighbor;}
  if (m_Matrix22[0][1] == 0) {++BlackNeighbor;}
  if (m_Matrix22[2][1] == 0) {++BlackNeighbor;}
  if (m_Matrix22[0][2] == 0) {++BlackNeighbor;}
  if (m_Matrix22[1][2] == 0) {++BlackNeighbor;}
  if (m_Matrix22[2][2] == 0) {++BlackNeighbor;}


  if ((BlackNeighbor >= 2) && (BlackNeighbor <= 6))
    return true;
  else
    return false;
}  /* k_ThinningSearchNeighbors */




RasterPtr  Raster::TightlyBounded (uint32 borderPixels)  const
{
  int32  tlRow = 0;
  int32  tlCol = 0;
  int32  brRow = 0;
  int32  brCol = 0;

  FindBoundingBox (tlRow, tlCol, brRow, brCol);
  if  ((tlRow > brRow)  ||  (tlCol > brCol))
  {
    if  (borderPixels < 1)
      borderPixels = 1;
    return AllocateARasterInstance (borderPixels * 2, borderPixels * 2, color);
  }

  int32  newHeight = (brRow - tlRow) + borderPixels * 2 +  + 1;
  int32  newWidth  = (brCol - tlCol) + borderPixels * 2 +  + 1;

  RasterPtr  result = AllocateARasterInstance (newHeight, newWidth, color);

  uchar**  newRed   = result->Red   ();
  uchar**  newGreen = result->Green ();
  uchar**  newBlue  = result->Blue  ();

  int32  oldR, oldC;

  int32  newR = borderPixels;
  for  (oldR = tlRow;  oldR <= brRow;  oldR++)
  {
    int32  newC = borderPixels;
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



RasterPtr  Raster::ToColor ()  const
{
  if  (color)
    return AllocateARasterInstance (*this);

  RasterPtr  r = AllocateARasterInstance (height, width, true);
  memcpy (r->RedArea   (), greenArea, totPixels);
  memcpy (r->GreenArea (), greenArea, totPixels);
  memcpy (r->BlueArea  (), greenArea, totPixels);
  return r;
}  /* ToColor */



RasterPtr  Raster::BandPass (float  lowerFreqBound,    // Number's between 0.0 and 1.0
                             float  upperFreqBound     // Represent percentage.
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

  int32  col;
  int32  row;

  int32  idx = 0;


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


  // Because the lower frequencise are further away from the center than higher frequencies
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

  // We now need to transform the fourier results back to GreayScale.
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

      if  (BackgroundPixel (greenArea[idx]))
      {
        // Since in the original source image this pixel was a back ground pixel;  we will
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
        destData[idx] = smallestPixelVal + Min (largestPixelVal, (uchar)(0.5 + pixelValRange * (zed - smallestNum) / range));
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

  int32 x;

  RasterPtr  firstRaster = IdxToPtr (0);
  int32 height = firstRaster->Height ();
  int32 width  = firstRaster->Height ();
  int32 totPixels = height * width;

  uint32*  totGreenArea = new uint32[totPixels];
  memset (totGreenArea, 0, totPixels * sizeof (uint32));

  int32 idx = 0;
  int32 rastersAdded = 0;

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
        totGreenArea[x] += (uint32)((float)redArea[x]   * 0.39f +
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

  delete  totGreenArea;

  return  smoothedRaster;
}  /* CreateSmoothedFrame */






uchar*  Raster::ToCompressor (uint32&  compressedBuffLen)  const
{
  // Will first write Rater data to a buffer that will be compressed by zlib by the Compressor class.
  // 0 - 3:    Height:  high order to low order
  // 4 - 7:    Width:   high order to low order
  // 8 - 8:    Color    0 = Grayscale,  1 = Color
  // 9 - 8 + (Height * Width) Green Channel
  // xxxxx                    Red  Channel
  // xxxxx                    Blue Channel
  uint32  totalDataNeeded = totPixels + (color ? (2 * totPixels) : 0) + 9;
  uchar*  buff = new uchar[totalDataNeeded];
  if  (!buff)
    return  NULL;

  memset (buff, 0, totalDataNeeded);

  uint32  h = (uint32)height;
  buff[0] = h % 256;  h = h / 256;
  buff[1] = h % 256;  h = h / 256;
  buff[2] = h % 256;  h = h / 256;
  buff[3] = h % 256;  h = h / 256;


  uint32  w = (uint32)width;
  
  buff[4] = w % 256;  w = w / 256;
  buff[5] = w % 256;  w = w / 256;
  buff[6] = w % 256;  w = w / 256;
  buff[7] = w % 256;  w = w / 256;

  buff[8] = (color ? 1 : 0);

  int32  buffIdx = 9;
  int32  x = 0;
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
  delete  buff;  buff = NULL;
  return  compressedBuff;
}  /* ToCompressor */





RasterPtr   Raster::Padded (int32 padding)
{
  int32  newHeight = height + padding * 2;
  int32  newWidth  = width  + padding * 2;

  int32  r, c;

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
  int32 row, col, x, y;

  double  prefix =  1.0 / (2.0 * PIE * sigma * sigma);
  double  twoSigmaSquared = 2.0 * sigma * sigma;


  // Determine size of kernel
  double z = 100;
  int32  delta = 0;
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

  int32 len = delta * 2 + 1;

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
  int32  row, col;
  int32  kernelSideLen = kernel.NumOfCols ();
  int32  delta = kernelSideLen / 2;

  double**  kernalData = kernel.Data ();

  for  (row = 0;  row < height;  row++)
  {
    int32  maskTop = row - delta;
    for  (col = 0;  col < width;  col++)
    {
      int32  maskLeft = col - delta;
      int32  maskRow  = maskTop;
      double  total  = 0.0;
      double kernelTotal = 0.0;
      for  (int32 kernelRow = 0;  (kernelRow < kernelSideLen)  &&  (maskRow < height);  kernelRow++, maskRow++)
      {
        if  (maskRow >= 0)
        {
          double*  kernalRowData = kernalData[kernelRow];

          int32  maskCol = maskLeft;
          for  (int32 kernelCol = 0;  (kernelCol < kernelSideLen)  &&  (maskCol < width);  kernelCol++, maskCol++)
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

      dest[row][col] = (uchar)((int32)(total + 0.5));
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
  PixelValue color (0, 0, 0);
  //Vec<float,3> white(255,255,255);
  float tempH, tempS, tempI;
  float y, x, r, xOriginalPoint, yOriginalPoint;

  // converts from polar coordinates to cartesian coordinates. Hue is an angle and Saturation is a distance from an origin.
  // xOriginalPoint and yOriginalPoint are the threshold point coordinates (origin)
  xOriginalPoint = thresholdS * cos (thresholdH);
  yOriginalPoint = thresholdS * sin (thresholdH);

  RasterPtr  resultingImage = AllocateARasterInstance (*this);

  for  (int32 m = 0;  m < height;  m++)
  {
    for  (int32 n = 0;  n < width;  n++)
    {
      GetPixelValue (m, n, color);
      color.ToHSI (tempH, tempS, tempI);

      // Convert from cartesian polar coordinates to cartesian coordinates. These are the cartesian coordinates of the
      // current pixel
      x = tempS * cos (tempH);
      y = tempS * sin (tempH);

      // Calculate the distance from the threshold initial point to the current pixel color using 
      // the euclidean distance
      r = (float)(sqrt (pow (xOriginalPoint - x, 2) + pow (yOriginalPoint - y, 2) + pow (thresholdI - tempI, 2)));

      if  (r <= distance)
         resultingImage->SetPixelValue (m, n, color);
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
    // Already a Grayscale image,  nothing much to do.
    return AllocateARasterInstance (*this);
  }

  uint32  x = 0;

  MatrixPtr  cov = new Matrix (3, 3);
  {
    // Build a covariance matrix.
    int32  col = 0, row = 0;

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
 
    for  (int32 varIdxX = 0;  varIdxX < 3;  ++varIdxX)
    {
      double*  varXs = centeredVals[varIdxX];
      for  (int32 varIdxY = varIdxX;  varIdxY < 3;  ++varIdxY)
      {
        // Calc the covariance between chanIdx0 and chanIdx1

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
      delete  centeredVals[col];
      centeredVals[col] = NULL;
    }
    delete  centeredVals;   centeredVals = NULL;
    delete  means;          means  = NULL;
    delete  totals;         totals = NULL;
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

  int32   eigenValueMaxIdx = 0;
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
  for  (int32 y = 0;  y < totPixels;  ++y)
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
    for  (int32 y = 0;  y < totPixels;  ++y)
    {
      resultArea[y] = (uchar)Min ((int32)((adjChannel[y] - valMin) * adjScaleFact + 0.5), (int32)255);
    }
  }
  delete  adjChannel;    adjChannel   = NULL;
  delete  cov;           cov          = NULL;
  delete  eigenValues;   eigenValues  = NULL;
  delete  eigenVectors;  eigenVectors = NULL;

  return  result;
}  /* CreateGrayScaleKLT */




RasterPtr  Raster::CreateGrayScaleKLTOnMaskedArea (const Raster&  mask)  const
{
  if  (!color)
  {
    // Already a Grayscale image,  nothing much to do.
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

  int32  totalMaskPixels = mask.TotalBackgroundPixels ();

  MatrixPtr  cov = new Matrix (3, 3);
  {
    // Build a covariance matrix.
    int32  col = 0, row = 0;

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
 
    for  (int32 varIdxX = 0;  varIdxX < 3;  ++varIdxX)
    {
      double*  varXs = centeredVals[varIdxX];
      for  (int32 varIdxY = varIdxX;  varIdxY < 3;  ++varIdxY)
      {
        // Calc the covariance between chanIdx0 and chanIdx1

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
    delete  centeredVals;   centeredVals = NULL;
    delete  means;          means  = NULL;
    delete  totals;         totals = NULL;
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

  int32   eigenValueMaxIdx = 0;
  double  eigenValueMax    = (*eigenValues)[0];
  for  (uint32 y = 1;  y < eigenValues->size ();  ++y)
  {
    if  ((*eigenValues)[y] > eigenValueMax)
    {
      eigenValueMaxIdx = y;
      eigenValueMax = (*eigenValues)[y];
    }
  }

  VectorDouble  eigenVector = eigenVectors->GetCol (eigenValueMaxIdx);
  // We are now going to do Matrix multiplication between the original image and the eigenVector
  // but since the eigen vector may have negative values the resultant nuebts can be negative for
  // some pixels and a magnitude greater than 255.  We will place temporarily into a floating 
  // array,  after which we will no the adjustment value and scaling factor.

  double  redFact   = eigenVector[0];
  double  greenFact = eigenVector[1];
  double  blueFact  = eigenVector[2];
  double  adjVal    = 0.0;

  double  valMin = DBL_MAX;
  double  valMax = -9999999999.99;
  double*  adjChannel = new double [totPixels];
  for  (int32 y = 0;  y < totPixels;  ++y)
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
    for  (int32 y = 0;  y < totPixels;  ++y)
    {
      if  (maskArea[y] > maskTh)
        resultArea[y] = (uchar)Min ((int32)((adjChannel[y] - valMin) * adjScaleFact + 0.5), (int32)255);
      else
        resultArea[y] = 0;
    }
  }
  delete  adjChannel;    adjChannel   = NULL;
  delete  cov;           cov          = NULL;
  delete  eigenValues;   eigenValues  = NULL;
  delete  eigenVectors;  eigenVectors = NULL;

  return  result;
}  /* CreateGrayScaleKLTOnMaskedArea */



void   Raster::WhiteOutBackground ()
{
  for  (int32 r = 0;  r < height;  ++r)
  {
    for  (int32 c = 0;  c < width;  ++c)
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
  int32  x = 0;

  // Determine number of unique values in greenArea channel.
  int32  freqCount[256];
  for  (x = 0;  x < 256;  ++x)
    freqCount[x] = 0;
  for  (x = 0;  x < totPixels;  ++x)
    freqCount[greenArea[x]]++;

  multimap<int32,uchar>  sortedFreqCounts;
  for  (x = 0;  x < 256;  ++x)
  {
    if  (freqCount[x] > 0)
      sortedFreqCounts.insert (pair<int32,uchar> (freqCount[x], (uchar)x));
  }

  PixelValue  colorAssignments[256];
  colorAssignments[0] = PixelValue::Black;
  int32  numUniqueValues = sortedFreqCounts.size ();
  multimap<int32,uchar>::reverse_iterator  idx;
  x = 0;
  for  (idx = sortedFreqCounts.rbegin ();  idx != sortedFreqCounts.rend ();  ++idx)
  {
    int32  pixelValue = idx->second;
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
                        PixelValue  color
                       )
{
  if  (blob == NULL)
    return;

  if  (origImage->blobIds == NULL)
    return;

  if  ((origImage->Height () != height)  ||  (origImage->Width () != width))
    return;

  int32  blobId = blob->Id ();
  int32  row = 0, col = 0;

  int32  rowStart = Min (blob->RowTop   (), height - 1);
  int32  rowEnd   = Min (blob->RowBot   (), height - 1);
  int32  colStart = Min (blob->ColLeft  (), width  - 1);
  int32  colEnd   = Min (blob->ColRight (), width  - 1);

  for  (row = rowStart;  row <= rowEnd;  ++row)
  {
    for  (col = colStart;  col <= colEnd;  ++col)
    {
      if  (origImage->blobIds[row][col] == blobId)
      {
        SetPixelValue (row, col, color);
      }
    }
  }
}  /* FillBlob */
