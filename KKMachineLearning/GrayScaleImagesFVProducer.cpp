/* CameraAcquisition.cpp -- Base class for the Acquisition threads.
 * Copyright (C) 2011-2013  Kurt Kramer
 * For conditions of distribution and use, see copyright notice in LarcosCounterUnManaged.txt
 */
#include "FirstIncludes.h"

#include <errno.h>
#include <istream>
#include <iostream>
#include <queue>
#include <map>
#include <vector>
#if  defined(WIN32)
#include <windows.h>
#endif
#include "MemoryDebug.h"
using namespace std;

#include "Blob.h"
#include "ConvexHull.h"
#include "KKBaseTypes.h"
#include "GlobalGoalKeeper.h"
#include "Raster.h"
#include "RunLog.h"
using namespace KKB;


#include "FactoryFVProducer.h"
#include "FeatureVectorProducer.h"
#include "FileDesc.h"
using namespace  KKMLL;

#include "GrayScaleImagesFVProducer.h"

#include "FeatureFileIOC45.h"



kkint16  GrayScaleImagesFVProducer::maxNumOfFeatures      = 56;

kkint16  GrayScaleImagesFVProducer::SizeIndex             = 0;
kkint16  GrayScaleImagesFVProducer::Moment1Index          = 1;
kkint16  GrayScaleImagesFVProducer::Moment2Index          = 2;
kkint16  GrayScaleImagesFVProducer::Moment3Index          = 3;
kkint16  GrayScaleImagesFVProducer::Moment4Index          = 4;
kkint16  GrayScaleImagesFVProducer::Moment5Index          = 5;
kkint16  GrayScaleImagesFVProducer::Moment6Index          = 6;
kkint16  GrayScaleImagesFVProducer::Moment7Index          = 7;
kkint16  GrayScaleImagesFVProducer::Moment8Index          = 8;

kkint16  GrayScaleImagesFVProducer::EdgeSizeIndex         = 9;
kkint16  GrayScaleImagesFVProducer::EdgeMoment1Index      = 10;
kkint16  GrayScaleImagesFVProducer::EdgeMoment2Index      = 11;	
kkint16  GrayScaleImagesFVProducer::EdgeMoment3Index      = 12;
kkint16  GrayScaleImagesFVProducer::EdgeMoment4Index      = 13;
kkint16  GrayScaleImagesFVProducer::EdgeMoment5Index      = 14;
kkint16  GrayScaleImagesFVProducer::EdgeMoment6Index      = 15;
kkint16  GrayScaleImagesFVProducer::EdgeMoment7Index      = 16;
kkint16  GrayScaleImagesFVProducer::EdgeMoment8Index      = 17;

kkint16  GrayScaleImagesFVProducer::TransparancyConvexHullIndex = 18;
kkint16  GrayScaleImagesFVProducer::TransparancyPixelCountIndex = 19;
kkint16  GrayScaleImagesFVProducer::TransparancyOpen3Index      = 20;
kkint16  GrayScaleImagesFVProducer::TransparancyOpen5Index      = 21;
kkint16  GrayScaleImagesFVProducer::TransparancyOpen7Index      = 22;
kkint16  GrayScaleImagesFVProducer::TransparancyOpen9Index      = 23;
kkint16  GrayScaleImagesFVProducer::TransparancyClose3Index     = 24;
kkint16  GrayScaleImagesFVProducer::TransparancyClose5Index     = 25;
kkint16  GrayScaleImagesFVProducer::TransparancyClose7Index     = 26;

kkint16  GrayScaleImagesFVProducer::ConvexAreaIndex       = 27;
kkint16  GrayScaleImagesFVProducer::TransparancySizeIndex = 28;
kkint16  GrayScaleImagesFVProducer::TransparancyWtdIndex  = 29;
kkint16  GrayScaleImagesFVProducer::WeighedMoment0Index   = 30;
kkint16  GrayScaleImagesFVProducer::WeighedMoment1Index   = 31;
kkint16  GrayScaleImagesFVProducer::WeighedMoment2Index   = 32;
kkint16  GrayScaleImagesFVProducer::WeighedMoment3Index   = 33;
kkint16  GrayScaleImagesFVProducer::WeighedMoment4Index   = 34;
kkint16  GrayScaleImagesFVProducer::WeighedMoment5Index   = 35;
kkint16  GrayScaleImagesFVProducer::WeighedMoment6Index   = 36;
kkint16  GrayScaleImagesFVProducer::WeighedMoment7Index   = 37;
kkint16  GrayScaleImagesFVProducer::WeighedMoment8Index   = 38;

kkint16  GrayScaleImagesFVProducer::IntensityHist1Index   = 39;
kkint16  GrayScaleImagesFVProducer::IntensityHist2Index   = 40;
kkint16  GrayScaleImagesFVProducer::IntensityHist3Index   = 41;
kkint16  GrayScaleImagesFVProducer::IntensityHist4Index   = 42;
kkint16  GrayScaleImagesFVProducer::IntensityHist5Index   = 43;
kkint16  GrayScaleImagesFVProducer::IntensityHist6Index   = 44;
kkint16  GrayScaleImagesFVProducer::IntensityHist7Index   = 45;

kkint16  GrayScaleImagesFVProducer::DarkSpotCount0        = 46;
kkint16  GrayScaleImagesFVProducer::DarkSpotCount1        = 47;
kkint16  GrayScaleImagesFVProducer::DarkSpotCount2        = 48;
kkint16  GrayScaleImagesFVProducer::DarkSpotCount3        = 49;
kkint16  GrayScaleImagesFVProducer::DarkSpotCount4        = 50;
kkint16  GrayScaleImagesFVProducer::DarkSpotCount5        = 51;
kkint16  GrayScaleImagesFVProducer::DarkSpotCount6        = 52;
kkint16  GrayScaleImagesFVProducer::DarkSpotCount7        = 53;
kkint16  GrayScaleImagesFVProducer::DarkSpotCount8        = 54;
kkint16  GrayScaleImagesFVProducer::DarkSpotCount9        = 55;


const  KKStr  GrayScaleImagesFVProducer::featureNames[] =
{
  "Size",                    //  0
  "Moment1",                 //  1 
  "Moment2",                 //  2
  "Moment3",                 //  3
  "Moment4",                 //  4
  "Moment5",                 //  5
  "Moment6",                 //  6
  "Moment7",                 //  7
  "Moment8",                 //  8
  
  "EdgeSize",                //  9
  "EdgeMoment1",             // 10
  "EdgeMoment2",             // 11
  "EdgeMoment3",             // 12
  "EdgeMoment4",             // 13
  "EdgeMoment5",             // 14
  "EdgeMoment6",             // 15
  "EdgeMoment7",             // 16
  "EdgeMoment8",             // 17

  "TransparancyConvexHull",  // 18
  "TransparancyPixelCount",  // 19
  
  "TransparancyOpen3",       // 20    Granulometric features
  "TransparancyOpen5",       // 21
  "TransparancyOpen7",       // 22
  "TransparancyOpen9",       // 23
  "TransparancyClose3",      // 24
  "TransparancyClose5",      // 25
  "TransparancyClose7",      // 26

  "ConvexArea",              // 27
  "TransparancySize",        // 28
  "TransparancyWtd",         // 29

  "WeighedMoment0",          // 30
  "WeighedMoment1",          // 31
  "WeighedMoment2",          // 32
  "WeighedMoment3",          // 33
  "WeighedMoment4",          // 34
  "WeighedMoment5",          // 35
  "WeighedMoment6",          // 36
  "WeighedMoment7",          // 37
  "WeighedMoment8",          // 38

  "IntensityHist1",          // 39
  "IntensityHist2",          // 40
  "IntensityHist3",          // 41 
  "IntensityHist4",          // 42
  "IntensityHist5",          // 43
  "IntensityHist6",          // 44
  "IntensityHist7",          // 45
  "DarkSpotCount0",          // 46
  "DarkSpotCount1",          // 47
  "DarkSpotCount2",          // 48
  "DarkSpotCount3",          // 49
  "DarkSpotCount4",          // 50
  "DarkSpotCount5",          // 51
  "DarkSpotCount6",          // 52
  "DarkSpotCount7",          // 53
  "DarkSpotCount8",          // 54
  "DarkSpotCount9"           // 55
};




GrayScaleImagesFVProducer::GrayScaleImagesFVProducer (FactoryFVProducerPtr  factory):
    FeatureVectorProducer ("GrayScaleImages",
                           factory
                          ),
    totPixsForMorphOps (4000000),
    workRaster1Area (NULL),
    workRaster2Area (NULL),
    workRaster3Area (NULL),
    workRaster1Rows (NULL),
    workRaster2Rows (NULL),
    workRaster3Rows (NULL)
{
  workRaster1Area = new uchar[totPixsForMorphOps];
  workRaster2Area = new uchar[totPixsForMorphOps];
  workRaster3Area = new uchar[totPixsForMorphOps];
}




GrayScaleImagesFVProducer::~GrayScaleImagesFVProducer ()
{
  delete  workRaster1Area;  workRaster1Area = NULL;
  delete  workRaster2Area;  workRaster2Area = NULL;
  delete  workRaster3Area;  workRaster3Area = NULL;
  delete  workRaster1Rows;  workRaster1Rows = NULL;
  delete  workRaster2Rows;  workRaster2Rows = NULL;
  delete  workRaster3Rows;  workRaster3Rows = NULL;
}



void  GrayScaleImagesFVProducer::ReductionByMultiple (kkint32        multiple,
                                                      const Raster&  srcRaster,
                                                      Raster&        destRaster
                                                     )  
                                            
{
  kkint32  srcHeight = srcRaster.Height ();
  kkint32  srcWidth  = srcRaster.Width ();

  kkint32  destHeight = (srcHeight + multiple - 1) / multiple;
  kkint32  destWidth  = (srcWidth  + multiple - 1) / multiple;

  kkint32  srcRow  = 0;
  kkint32  srcCol  = 0;
  kkint32  destRow = 0;
  kkint32  destCol = 0;

  uchar**  srcMatrix  = srcRaster.Green  ();
  uchar**  destMatrix = destRaster.Green ();

  kkint32  heightOffset = (srcHeight % multiple) / 2;
  kkint32  widthOffset  = (srcWidth  % multiple) / 2;

  srcRow = -heightOffset;

  kkint32  r, c;

  for  (destRow = 0;  destRow < destHeight;  ++destRow)
  {
    srcCol = -widthOffset;
    for  (destCol = 0;  destCol < destWidth;  ++destCol)
    {
      kkint32 total = 0;

      kkint32  srcRowStart = Max (0, srcRow);
      kkint32  srcRowEnd   = Min (srcRow + multiple, srcHeight);
      kkint32  srcColStart = Max (0, srcCol);
      kkint32  srcColEnd   = Min (srcCol + multiple, srcWidth);

      kkint32  count = 0;

      for  (r = srcRowStart;  r <srcRowEnd;  ++r)
      {
        for  (c = srcColStart;  c <srcColEnd;  ++c)
        {
          total += srcMatrix[r][c];
          ++count;
        }
      }
      destMatrix[destRow][destCol] = (uchar)(total / count);
      srcCol += multiple;
    }
    srcRow += multiple;
  }
}  /* ReductionByMultiple */




void  GrayScaleImagesFVProducer::SaveIntermediateImage (const Raster&  raster, 
                                                        const KKStr&   desc,
                                                        RasterListPtr  intermediateImages
                                                       )
{
  if  (!intermediateImages)
    return;

  RasterPtr  newRaster = NULL;

  kkint32  largestDim = Max (raster.Height (), raster.Width ());
  if  (largestDim < 300)
  {
    newRaster = new Raster (raster);
  }
  else
  {
    kkint32  reductionMultiple = 2;
    while  ((largestDim / reductionMultiple) > 300)
     reductionMultiple++;
    newRaster = raster.ReduceByEvenMultiple (reductionMultiple);
  }

  newRaster->FileName (desc);
  intermediateImages->PushOnBack (newRaster);
}  /* SaveIntermediateImage */



void  GrayScaleImagesFVProducer::BinarizeImageByThreshold (uchar          lower,
                                                           uchar          upper,
                                                           const Raster&  src,
                                                           Raster&        dest
                                                          )
{
  uchar const *  srcData  = src.GreenArea ();
  uchar *        destData = dest.GreenArea ();
  
  kkint32  totPixels = src.TotPixels ();

  memset (destData, 0, totPixels);

  kkint32 x = 0;
  for  (x = 0;  x < totPixels;  ++x)
  {
    if  ((srcData[x] >= lower)  &&  (srcData[x] < upper))
      destData[x] = 255;
  }
}  /* BinarizeImageByThreshold */



FeatureVectorPtr  GrayScaleImagesFVProducer::ComputeFeatureVector (const Raster&     srcImage,
                                                                   const MLClassPtr  knownClass,
                                                                   RasterListPtr     intermediateImages,
                                                                   float             priorReductionFactor,
                                                                   RunLog&           runLog
                                                                  )
                                                                  
{
  FeatureVectorPtr  fv = new FeatureVector (maxNumOfFeatures);
  fv->MLClass (knownClass);
  float*  featureData = fv->FeatureDataAlter ();

  fv->Version (Version ());

  kkint32 areaBeforeReduction = 0;
  float  weighedSizeBeforeReduction = 0.0f;

  kkint32 row = 0;
  kkint32 col = 0;

  kkuint32  intensityHistBuckets[8];
  srcImage.CalcAreaAndIntensityFeatures (areaBeforeReduction, 
                                         weighedSizeBeforeReduction,
                                         intensityHistBuckets
                                        );

  kkint32 srcHeight = srcImage.Height ();
  kkint32 srcWidth  = srcImage.Width  ();

  kkint32 reducedHeight = srcHeight;
  kkint32 reducedWidth  = srcWidth;

  kkint32 reducedSquareArea = reducedHeight * reducedWidth;

  kkint32 reductionMultiple = 1;

  while  (reducedSquareArea > totPixsForMorphOps)
  {
    ++reductionMultiple;
    reducedHeight = (srcHeight + reductionMultiple - 1) / reductionMultiple;
    reducedWidth  = (srcWidth  + reductionMultiple - 1) / reductionMultiple;
    reducedSquareArea = reducedHeight * reducedWidth;
  }

  kkint32  reductionMultipleSquared = reductionMultiple * reductionMultiple;

  float    totalReductionMultiple = priorReductionFactor * reductionMultiple;
  float    totalReductionMultipleSquared = totalReductionMultiple * totalReductionMultiple;

  delete  workRaster1Rows;  workRaster1Rows = new uchar*[reducedHeight];
  delete  workRaster2Rows;  workRaster2Rows = new uchar*[reducedHeight];
  delete  workRaster3Rows;  workRaster3Rows = new uchar*[reducedHeight];

  uchar*  wp1 = workRaster1Area;
  uchar*  wp2 = workRaster2Area;
  uchar*  wp3 = workRaster3Area;

  for  (row = 0;  row < reducedHeight; ++row)
  {
    workRaster1Rows[row] = wp1;
    workRaster2Rows[row] = wp2;
    workRaster3Rows[row] = wp3;
    wp1 += reducedWidth;
    wp2 += reducedWidth;
    wp3 += reducedWidth;
  }

  Raster  workRaster1 (reducedHeight, reducedWidth, workRaster1Area, workRaster1Rows);
  Raster  workRaster2 (reducedHeight, reducedWidth, workRaster2Area, workRaster2Rows);
  Raster  workRaster3 (reducedHeight, reducedWidth, workRaster3Area, workRaster3Rows);

  Raster const * initRaster = NULL;
  RasterPtr      wr1        = NULL;
  RasterPtr      wr2        = NULL;

  if  (reductionMultiple > 1)
  {
    try
    {
      ReductionByMultiple (reductionMultiple, srcImage, workRaster1);
    }
    catch  (...)
    {
      runLog.Level (-1) << endl << "GrayScaleImagesFVProducer::ComputeFeatureVector   ***ERROR***  Exception calling 'ReductionByMultiple'."  << endl << endl;
      return NULL;
    }
    initRaster = &workRaster1;
    wr1        = &workRaster2;
    wr2        = &workRaster3;
  }
  else
  {
    initRaster = &srcImage;
    wr1        = &workRaster1;
    wr2        = &workRaster2;
  }

  if  (areaBeforeReduction < 20)
  {
    for  (kkint32 tp = 0;  tp < maxNumOfFeatures;  tp++)
      featureData[tp] = 9999999;
    return fv;
  }
  
  float  convexf = 0.0;
  float  centralMoments[9];
  float  centralMomentsWeighted[9];

  kkint32  pixelCountReduced = 0;
  float    pixelCountWeightedReduced = 0.0f;

  initRaster->ComputeCentralMoments (pixelCountReduced, pixelCountWeightedReduced, centralMoments, centralMomentsWeighted);


  float  edgeMomentf[9];

  initRaster->Dilation (wr1);
  wr1->Dilation (wr2);
  wr2->FillHole (wr1);

  wr1->Erosion (wr2);
  wr2->Edge (wr1);
  wr1->CentralMoments (edgeMomentf);
  if  (intermediateImages)
  {
    kkint32  numEdgePixelsFound = (kkint32)(edgeMomentf[0]);
    SaveIntermediateImage (*wr2, "Edge_Image_" + StrFormatInt (numEdgePixelsFound, "ZZZZ0"), intermediateImages);
  }

  kkint32 area = (kkint32)(centralMoments[0] + 0.5f);  // Moment-0 is the same as the number of foreground pixels in example.
  {
    ConvexHullPtr  ch = new ConvexHull ();
    ch->Filter (*initRaster, wr1);
    convexf = (float)ch->ConvexArea ();

    if  (intermediateImages)
    {
      KKStr convexImageFileName = "ConvexHull_" +
                                   StrFormatInt ((kkint32)convexf, "ZZZZZ0");
      SaveIntermediateImage (*wr1, convexImageFileName, intermediateImages);
    }

    //delete  convexImage;
    //convexImage = NULL;
    delete  ch;
    ch = NULL;
  }

  initRaster->Erosion (wr1);
  wr1->Dilation (wr2);
  float  areaOpen3 = (float)(wr2->ForegroundPixelCount());
  if  (intermediateImages)
    SaveIntermediateImage (*wr2, "Opening3_" + StrFormatInt ((kkint32)areaOpen3, "ZZZZZZ0"), intermediateImages);


  initRaster->Erosion (wr1, MorphOp::MaskTypes::SQUARE5);
  wr1->Dilation (wr2, MorphOp::MaskTypes::SQUARE5);
  float  areaOpen5 = (float)(wr2->ForegroundPixelCount ());
  if  (intermediateImages)
    SaveIntermediateImage (*wr2, "Opening5_" + StrFormatInt ((kkint32)areaOpen5, "ZZZZZZ0"), intermediateImages);

  initRaster->Erosion (wr1, MorphOp::MaskTypes::SQUARE7);
  wr1->Dilation (wr2, MorphOp::MaskTypes::SQUARE7);
  float  areaOpen7 = (float)(wr2->ForegroundPixelCount ());
  if  (intermediateImages)
    SaveIntermediateImage (*wr2, "Opening7_" + StrFormatInt ((kkint32)areaOpen7, "ZZZZZZ0"), intermediateImages);
  
  wr2->Erosion (wr1, MorphOp::MaskTypes::SQUARE9);
  wr1->Dilation (wr2, MorphOp::MaskTypes::SQUARE9);
  float  areaOpen9 = (float)(wr2->ForegroundPixelCount ());
  if  (intermediateImages)
    SaveIntermediateImage (*wr2, "Opening9_" + StrFormatInt ((kkint32)areaOpen9, "ZZZZZZ0"), intermediateImages);

  initRaster->Dilation (wr1);
  wr1->Erosion (wr2);
  float  areaClose3 = (float)(wr2->ForegroundPixelCount ());
  if  (intermediateImages)
    SaveIntermediateImage (*wr2, "Close3_" + StrFormatInt ((kkint32)areaClose3, "ZZZZZZ0"), intermediateImages);

  wr2->FillHole (wr1);
  float  tranf = (float)(wr2->ForegroundPixelCount ());
  if  (intermediateImages)
    SaveIntermediateImage (*wr2, "FillHole_" + StrFormatInt ((kkint32)tranf, "ZZZZZZ0"), intermediateImages);

  initRaster->Dilation (wr1, MorphOp::MaskTypes::SQUARE5);
  wr1->Erosion (wr2, MorphOp::MaskTypes::SQUARE5);
  float  areaClose5 = (float)(wr2->ForegroundPixelCount ());
  if  (intermediateImages)
    SaveIntermediateImage (*wr2, "Close5_" + StrFormatInt ((kkint32)areaClose5, "ZZZZZZ0"), intermediateImages);
  
  initRaster->Dilation (wr1, MorphOp::MaskTypes::SQUARE7);
  wr1->Erosion   (wr2, MorphOp::MaskTypes::SQUARE7);
  float  areaClose7 = float (wr2->ForegroundPixelCount ());
  if  (intermediateImages)
    SaveIntermediateImage (*wr2, "Close7_" + StrFormatInt ((kkint32)areaClose7, "ZZZZZZ0"), intermediateImages);

  {
    featureData[SizeIndex]    = float (areaBeforeReduction * priorReductionFactor);
    featureData[Moment1Index] = float (centralMoments[1]);
    featureData[Moment2Index] = float (centralMoments[2]);
    featureData[Moment3Index] = float (centralMoments[3]);
    featureData[Moment4Index] = float (centralMoments[4]);
    featureData[Moment5Index] = float (centralMoments[5]);
    featureData[Moment6Index] = float (centralMoments[6]);
    featureData[Moment7Index] = float (centralMoments[7]);
    featureData[Moment8Index] = float (centralMoments[8]);

    featureData[WeighedMoment0Index] = centralMomentsWeighted[0] * totalReductionMultiple;
    featureData[WeighedMoment1Index] = centralMomentsWeighted[1];
    featureData[WeighedMoment2Index] = centralMomentsWeighted[2];
    featureData[WeighedMoment3Index] = centralMomentsWeighted[3];
    featureData[WeighedMoment4Index] = centralMomentsWeighted[4];
    featureData[WeighedMoment5Index] = centralMomentsWeighted[5];
    featureData[WeighedMoment6Index] = centralMomentsWeighted[6];
    featureData[WeighedMoment7Index] = centralMomentsWeighted[7];
    featureData[WeighedMoment8Index] = centralMomentsWeighted[8];

    featureData[EdgeSizeIndex]    = (float)edgeMomentf[0] * totalReductionMultiple;
    featureData[EdgeMoment1Index] = (float)edgeMomentf[1];
    featureData[EdgeMoment2Index] = (float)edgeMomentf[2];
    featureData[EdgeMoment3Index] = (float)edgeMomentf[3];
    featureData[EdgeMoment4Index] = (float)edgeMomentf[4];
    featureData[EdgeMoment5Index] = (float)edgeMomentf[5];
    featureData[EdgeMoment6Index] = (float)edgeMomentf[6];
    featureData[EdgeMoment7Index] = (float)edgeMomentf[7];
    featureData[EdgeMoment8Index] = (float)edgeMomentf[8];

    // Need to adjust for any reduction in Image Size
    //featureData[SizeIndex]           = float (areaBeforeReduction);
    //featureData[EdgeSizeIndex]       = float (edgeMomentf[0] * (float)(reductionMultipleSquared));
    //featureData[WeighedMoment0Index] = weighedSizeBeforeReduction;
  }


  if ((area > convexf)  &&  (convexf > 0))
     featureData[TransparancyConvexHullIndex] = 1.0;
  else 
     featureData[TransparancyConvexHullIndex] = (float)area / (float)convexf;

  featureData[TransparancyPixelCountIndex] = (float)area / (float)tranf;
  featureData[TransparancyOpen3Index]      = (float)(area - areaOpen3)  / (float)area;
  featureData[TransparancyOpen5Index]      = (float)(area - areaOpen5)  / (float)area;
  featureData[TransparancyOpen7Index]      = (float)(area - areaOpen7)  / (float)area;                
  featureData[TransparancyOpen9Index]      = (float)(area - areaOpen9)  / (float)area;
  featureData[TransparancyClose3Index]     = (float)(area - areaClose3) / (float)area;
  featureData[TransparancyClose5Index]     = (float)(area - areaClose5) / (float)area;
  featureData[TransparancyClose7Index]     = (float)(area - areaClose7) / (float)area;
 
  {
    // This part has to be done after 'CalcOrientationAndEigerRatio' is called. That is where the example centroid is calculated.
    //fv->CentroidCol (initRaster->CentroidCol () * reductionMultiple);
    //fv->CentroidRow (initRaster->CentroidRow () * reductioqnMultiple);
  }

  featureData[ConvexAreaIndex]       = convexf * totalReductionMultipleSquared;
  featureData[TransparancySizeIndex] = (float)(centralMoments[0] / convexf);
  featureData[TransparancyWtdIndex]  = (float)(centralMomentsWeighted[0] / convexf);

  float  areaD = float (areaBeforeReduction);

  featureData[IntensityHist1Index] = ((float)intensityHistBuckets[1] / areaD);
  featureData[IntensityHist2Index] = ((float)intensityHistBuckets[2] / areaD);
  featureData[IntensityHist3Index] = ((float)intensityHistBuckets[3] / areaD);
  featureData[IntensityHist4Index] = ((float)intensityHistBuckets[4] / areaD);
  featureData[IntensityHist5Index] = ((float)intensityHistBuckets[5] / areaD);
  featureData[IntensityHist6Index] = ((float)intensityHistBuckets[6] / areaD);
  featureData[IntensityHist7Index] = ((float)intensityHistBuckets[7] / areaD);


  {
    RasterPtr  darkSpots = NULL;

    BinarizeImageByThreshold (200, 255, *initRaster, *wr1); 
   
    wr1->Erosion (wr2, MorphOp::MaskTypes::SQUARE3);
    wr2->Erosion (wr1, MorphOp::MaskTypes::SQUARE3);

    KKB::BlobListPtr  blobs = wr1->ExtractBlobs (3);

    int  darkSpotFreq[10];
    int x;
    for  (x = 0;  x < 10;  ++x)
      darkSpotFreq[x] = 0;

    KKB::BlobList::iterator  idx;
    for  (idx = blobs->begin ();  idx != blobs->end ();  ++idx)
    {
      KKB::BlobPtr  b = idx->second;

      double l =  ::log ((double)(b->PixelCount ()));

      double logOfThree = ::log (3.0);

      int  index = (int)floor (l / logOfThree);
      index = Min (Max (0, index), 9);
      ++(darkSpotFreq[index]);
    }

    featureData[DarkSpotCount0] = (float)darkSpotFreq[0];
    featureData[DarkSpotCount1] = (float)darkSpotFreq[1];
    featureData[DarkSpotCount2] = (float)darkSpotFreq[2];
    featureData[DarkSpotCount3] = (float)darkSpotFreq[3];
    featureData[DarkSpotCount4] = (float)darkSpotFreq[4];
    featureData[DarkSpotCount5] = (float)darkSpotFreq[5];
    featureData[DarkSpotCount6] = (float)darkSpotFreq[6];
    featureData[DarkSpotCount7] = (float)darkSpotFreq[7];
    featureData[DarkSpotCount8] = (float)darkSpotFreq[8];
    featureData[DarkSpotCount9] = (float)darkSpotFreq[9];

    delete  blobs;
    blobs = NULL;
  }

  if  (intermediateImages)
  {
    RasterPtr  thinnedImage = initRaster->ThinContour ();
    SaveIntermediateImage  (*thinnedImage, "_Thinned", intermediateImages);
    delete  thinnedImage;  
    thinnedImage = NULL;
  }

  featureData[0] = (float)areaBeforeReduction;  // In case the example was reduced.
  fv->OrigSize ((float)areaBeforeReduction);
 
  return fv;
}  /* ComputeFeatureVector */




const type_info*   GrayScaleImagesFVProducer::FeatureVectorTypeId () const
{
  return  &(typeid (FeatureVector));
}


const type_info*  GrayScaleImagesFVProducer::FeatureVectorListTypeId () const
{
  return  &(typeid (FeatureVectorList));
}



FileDescPtr  GrayScaleImagesFVProducer::DefineFileDesc ()  const
{
  return  DefineFileDescStatic ();
}  /* DefineFileDesc */



FileDescPtr  GrayScaleImagesFVProducer::existingFileDesc = NULL;

FileDescPtr  GrayScaleImagesFVProducer::DefineFileDescStatic ()
{
  if  (existingFileDesc)
    return  existingFileDesc;

  GlobalGoalKeeper::StartBlock ();
  if  (!existingFileDesc)
  {
    bool  alreadyExists = false;
    FileDescPtr  tempFileDesc = new KKMLL::FileDesc ();
    for  (kkint32 fieldNum = 0;  fieldNum < maxNumOfFeatures;  ++fieldNum)
    {
      tempFileDesc->AddAAttribute (featureNames[fieldNum], AttributeType::Numeric, alreadyExists);
    }
    tempFileDesc->Version (_GrayScaleImagesFVProducer_VersionNum_);

    existingFileDesc = FileDesc::GetExistingFileDesc (tempFileDesc);
    tempFileDesc = NULL;
  }
  GlobalGoalKeeper::EndBlock ();

  // Lets make sure that one was already created by opening up a data file.
  return existingFileDesc;
}




FeatureVectorListPtr  GrayScaleImagesFVProducer::ManufacturFeatureVectorList (bool     owner,
                                                                              RunLog&  runLog
                                                                             )
{
  return  new FeatureVectorList (FileDesc (), owner);
}






GrayScaleImagesFVProducerFactory::GrayScaleImagesFVProducerFactory ():
   FactoryFVProducer("GrayScaleImagesFVProducerFactory", "FeatureVector", "General Purpose Grayscale Image Feature Computer")
{
}



GrayScaleImagesFVProducerFactory::~GrayScaleImagesFVProducerFactory ()
{
}



FeatureFileIOPtr  GrayScaleImagesFVProducerFactory::DefaultFeatureFileIO ()  const
{
  return  FeatureFileIOC45::Driver ();
}


const type_info*  GrayScaleImagesFVProducerFactory::FeatureVectorTypeId ()  const  
{
  return  &typeid (FeatureVector);
}



const type_info*  GrayScaleImagesFVProducerFactory::FeatureVectorListTypeId  () const  
{
  return  &typeid (FeatureVectorList);
}



FileDescPtr  GrayScaleImagesFVProducerFactory::FileDesc ()  const
{
  return  GrayScaleImagesFVProducer::DefineFileDescStatic ();
}



GrayScaleImagesFVProducerPtr  GrayScaleImagesFVProducerFactory::ManufactureInstance (RunLog&  runLog)
{
  return new GrayScaleImagesFVProducer (this);
} /* ManufactureInstance */




FeatureVectorListPtr  GrayScaleImagesFVProducerFactory::ManufacturFeatureVectorList (bool     owner,
                                                                                     RunLog&  runLog
                                                                                    )
{
  return new FeatureVectorList (GrayScaleImagesFVProducer::DefineFileDescStatic (), owner);
}



GrayScaleImagesFVProducerFactory*  GrayScaleImagesFVProducerFactory::factory = Factory (NULL);




GrayScaleImagesFVProducerFactory*  GrayScaleImagesFVProducerFactory::Factory (RunLog*  runLog)
{
  GlobalGoalKeeper::StartBlock ();

  if  (factory == NULL)
  {
    factory = new GrayScaleImagesFVProducerFactory ();
    FactoryFVProducer::RegisterFactory (factory, runLog);
  }
  GlobalGoalKeeper::EndBlock ();

  return  factory;
}

