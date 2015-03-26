/* MorphOp.cpp -- Base class to be used by all Morphological operators..
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */

#include  "FirstIncludes.h"
#include  <stdlib.h>
#include  <iostream>
#include  <map>
#include  <vector>

#include  "MemoryDebug.h"
using namespace std;

#include "Raster.h"

#include "MorphOp.h"
using namespace KKB;





kkint32  MorphOp::biases[] = {1,  // CROSS3 
                            2,  // CROSS5
                            1,  // SQUARE3
                            2,  // SQUARE5
                            3,  // SQUARE7
                            4,  // SQUARE9
                            5   // SQUARE11
                           };


kkint32  MorphOp::maskShapes[] = {stCross,   // CROSS3 
                                stCross,   // CROSS5
                                stSquare,  // SQUARE3
                                stSquare,  // SQUARE5
                                stSquare,  // SQUARE7
                                stSquare,  // SQUARE9
                                stSquare   // SQUARE11
                               };


MorphOp::MorphOp ():
    backgroundPixelTH    (31),
    backgroundPixelValue (0),
    srcRaster            (NULL),
    srcRedArea           (NULL),
    srcGreenArea         (NULL),
    srcBlueArea          (NULL),
    srcRed               (NULL),
    srcGreen             (NULL),
    srcBlue              (NULL),
    srcColor             (false),
    srcHeight            (0),
    srcWidth             (0)
{
}



MorphOp::~MorphOp ()
{
}



KKB::KKStr   MorphOp::OperationTypeToStr (OperationType  _operation)
{
  if  (_operation == moStretcher)
    return "Stretcher";

  else if  (_operation == moBinarize)
    return "Binarize";

  else if  (_operation == moConvexHull)
    return "ConvexHull";

  else if  (_operation == moErosion)
    return "Erosion";

  else if  (_operation == moMaskExclude)
    return "MaskExclude";

  else
    return "NULL";
}



MorphOp::OperationType    MorphOp::OperationTypeFromStr (const KKB::KKStr&  _operationStr)
{
  if  (_operationStr.EqualIgnoreCase ("Stretcher"))
    return  moStretcher;

  else if  (_operationStr.EqualIgnoreCase ("Binarize"))
    return  moBinarize;

  else if  (_operationStr.EqualIgnoreCase ("ConvexHull"))
    return  moConvexHull;

  else if  (_operationStr.EqualIgnoreCase ("Erosion"))
    return  moErosion;

  else if  (_operationStr.EqualIgnoreCase ("MaskExclude"))
    return  moMaskExclude;

  else
    return  moNULL;
}



void  MorphOp::SetSrcRaster (RasterConstPtr  _srcRaster)
{
  srcRaster = _srcRaster;

  srcColor   = srcRaster->Color  ();
  srcHeight  = srcRaster->Height ();
  srcWidth   = srcRaster->Width  ();

  backgroundPixelTH    = srcRaster->BackgroundPixelTH    ();
  backgroundPixelValue = srcRaster->BackgroundPixelValue ();

  srcRedArea   = srcRaster->RedArea   ();
  srcGreenArea = srcRaster->GreenArea ();
  srcBlueArea  = srcRaster->BlueArea  ();

  srcRed   = srcRaster->Red   ();
  srcGreen = srcRaster->Green ();
  srcBlue  = srcRaster->Blue  ();
}  /* SetSrcRaster */



bool  MorphOp::BackgroundPixel (uchar  pixel)  const
{
  return (pixel <= backgroundPixelTH);
}  /* BackgroundPixel */



bool  MorphOp::BackgroundPixel (kkint32  row,
                                kkint32  col
                               )  const
{
  if  ((row < 0)           ||  
       (row >= srcHeight)  ||
       (col < 0)           ||
       (col >= srcWidth)   ||
       (srcGreen == NULL)
      )
    return false;

  return (srcGreen[row][col] <= backgroundPixelTH);
}  /* BackgroundPixel */



bool  MorphOp::ForegroundPixel (uchar  pixel)  const
{
  return (pixel > backgroundPixelTH);
}  /* ForegroundPixel */



bool  MorphOp::ForegroundPixel (kkint32  row,
                                kkint32  col
                               )  const
{
  if  ((row < 0)           ||  
       (row >= srcHeight)  ||
       (col < 0)           ||
       (col >= srcWidth)   ||
       (srcGreen == NULL)
      )
    return false;

  return (srcGreen[row][col] > backgroundPixelTH);
}  /* ForegroundPixel */






/**
 *@brief Returns true if all the pixels covered by the specified mask are Foreground pixels.
 *@see  Erosion, Dilation, Closing, Opening, MaskType
 */
bool  MorphOp::Fit (MaskTypes  mask,
                    kkint32    row, 
                    kkint32    col
                   )  const
{
  kkint32  bias = biases[mask];
  kkint32  r, c;
  kkint32  rStart = row - bias;
  kkint32  rEnd   = row + bias;
  kkint32  cStart = col - bias;
  kkint32  cEnd   = col + bias;

  if  (rStart  < 0)           rStart = 0;
  if  (rEnd    >= srcHeight)  rEnd = srcHeight - 1;
  if  (cStart  < 0)           cStart = 0;
  if  (cEnd    >= srcWidth)   cEnd = srcWidth - 1;

  if  (maskShapes[mask] == stSquare)
  {
    for  (r = rStart;  r <= rEnd;  r++)
    {
      uchar const* rowPtr = srcGreen[r];

      for  (c = cStart;  c <= cEnd;  c++)
      {
        if  (BackgroundPixel (rowPtr[c]))
          return  false;
      }
    }
  }

  else
  {  
    for  (r = rStart;  r <= rEnd;  r++)
    {
      if  (BackgroundPixel (srcGreen[r][col]))
        return  false;
    }

    for  (c = cStart;  c <= cEnd;  c++)
    {
      if  (BackgroundPixel (srcGreen[row][c]))
        return  false;
    }
  }

  return  true;
}  /* Fit */


