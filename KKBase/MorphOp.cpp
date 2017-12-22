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


StructureType  MorphOp::maskShapes[] = {StructureType::stCross,   // CROSS3 
                                        StructureType::stCross,   // CROSS5
                                        StructureType::stSquare,  // SQUARE3
                                        StructureType::stSquare,  // SQUARE5
                                        StructureType::stSquare,  // SQUARE7
                                        StructureType::stSquare,  // SQUARE9
                                        StructureType::stSquare   // SQUARE11
                                       };


kkint32   MorphOp::Biases (MaskTypes  mt)
{
  if  (mt < MaskTypes::CROSS3)
    mt = MaskTypes::CROSS3;

  else if  (mt > MaskTypes::SQUARE9)
    mt = MaskTypes::SQUARE9;

  return  biases[(int)mt];
}



StructureType  MorphOp::MaskShapes (MaskTypes  mt)
{
  if  (mt < MaskTypes::CROSS3)
    mt = MaskTypes::CROSS3;

  else if  (mt > MaskTypes::SQUARE9)
    mt = MaskTypes::SQUARE9;

  return  maskShapes[(int)mt];
}



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



kkMemSize  MorphOp::MemoryConsumedEstimated ()
{
  return 0;
}



KKB::KKStr   MorphOp::OperationTypeToStr (OperationType  _operation)
{
  if (_operation == OperationType::Binarize)
    return "Binarize";

  else if  (_operation == OperationType::BmiFiltering)
    return "BmiFiltering";

  else if  (_operation == OperationType::ConvexHull)
    return "ConvexHull";

  else if  (_operation == OperationType::Dilation)
    return "Dilation";

  else if  (_operation == OperationType::Erosion)
    return "Erosion";

  else if  (_operation == OperationType::MaskExclude)
    return "MaskExclude";

  else if (_operation == OperationType::ReduceByEvenMultiple)
    return "ReduceByFactor";

  else if (_operation == OperationType::ReduceByFactor)
    return "ReduceByEvenMultiple";
  
  else if (_operation == OperationType::SobelEdgeDetection)
    return "SobelEdgeDetection";

  else if (_operation == OperationType::Stretcher)
    return "Stretcher";

  else if (_operation == OperationType::ThinContour)
    return "ThinContour";

  else
    return "NULL";
}



MorphOp::OperationType   MorphOp::OperationTypeFromStr (const KKB::KKStr&  _operationStr)
{
  if  (_operationStr.EqualIgnoreCase ("Binarize"))
    return  OperationType::Binarize;

  else if  (_operationStr.EqualIgnoreCase ("BmiFiltering"))
    return  OperationType::BmiFiltering;

  else if  (_operationStr.EqualIgnoreCase ("ConvexHull"))
    return  OperationType::ConvexHull;

  else if (_operationStr.EqualIgnoreCase ("Dilation"))
    return  OperationType::Dilation;

  else if  (_operationStr.EqualIgnoreCase ("Erosion"))
    return  OperationType::Erosion;

  else if  (_operationStr.EqualIgnoreCase ("MaskExclude"))
    return  OperationType::MaskExclude;

  else if (_operationStr.EqualIgnoreCase ("ReduceByEvenMultiple"))
    return  OperationType::ReduceByEvenMultiple;

  else if (_operationStr.EqualIgnoreCase ("ReduceByFactor"))
    return  OperationType::ReduceByFactor;

  else if (_operationStr.EqualIgnoreCase ("SobelEdgeDetection"))
    return  OperationType::SobelEdgeDetection;

  else if (_operationStr.EqualIgnoreCase ("Stretcher"))
    return  OperationType::Stretcher;

  else if (_operationStr.EqualIgnoreCase ("ThinContour"))
    return  OperationType::ThinContour;

  else
    return  OperationType::Null;
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
