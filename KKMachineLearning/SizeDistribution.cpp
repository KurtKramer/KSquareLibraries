#include "FirstIncludes.h"

//***************************************************************************
//*                                                                         *
//*-------------------------------------------------------------------------*
//*  History                                                                *
//*                                                                         *
//*  Prog       Date      Description                                       *
//*  ------- -----------  ------------------------------------------------- *
//*  Kurt    Jul-12-2003                                                    *
//***************************************************************************
//* 
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>


#include  "MemoryDebug.h"

using namespace  std;


#include "KKBaseTypes.h"
#include "KKQueue.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace  KKB;


#include "SizeDistribution.h"
#include "MLClass.h"
#include "FeatureVector.h"
using namespace  KKMachineLearning;



//#define  BucketCount  50
//#define  BucketSize   100


class  SizeDistribution::ClassTotals  
{
public:
  typedef  ClassTotals*  ClassTotalsPtr;

  ClassTotals (KKStr  _name,
               int32  _bucketCount,
               int32  _bucketSize
              ):
      bucketCount        (_bucketCount),
      bucketSize         (_bucketSize),
      count              (0),
      name               (_name),
      nameUpper          (_name),
      sizeBuckets        (NULL)

  {
    nameUpper.Upper ();

    sizeBuckets = new int32[bucketCount];
    for  (int32 x = 0; x < bucketCount; x++)
       sizeBuckets[x] = 0;

    count = 0;
  }

      
  ~ClassTotals ()
  {
    delete[]  sizeBuckets;
  }




  void  Increment (int32  size)
  {
    int32  bucket = (int32)(size / bucketSize);
    if  (bucket >= bucketCount)
      bucket = bucketCount - 1;

    sizeBuckets[bucket]++;
    count++;
  }



  void  AddIn (ClassTotalsPtr  classTotals);


  int32   BucketCount (int32 idx)
  {
    if  ((idx < 0)  &&  (idx >= bucketCount))
      return 0;
    return  sizeBuckets[idx];
  }


  void  PrintFormatedLine (ostream&  _outFile);

  void  PrintCSVLine      (ostream&  _outFile);

  void  PrintTabDelLine   (ostream&  _outFile);


  int32    bucketCount;
  int32    bucketSize;
  int32    count;
  KKStr    name;
  KKStr    nameUpper;
  int32*   sizeBuckets;

};  /* ClassTotals */




class  SizeDistribution::ClassTotalsList:  public KKQueue<ClassTotals>
{
public:
  ClassTotalsList (bool  _owner):
     KKQueue<ClassTotals> (_owner)
  {
  }
    
  ~ClassTotalsList ()
  {
  }
    


  ClassTotals::ClassTotalsPtr  LookUp (KKStr  _name)
  {
    _name.Upper ();

    int32  idx;
    ClassTotals::ClassTotalsPtr  classTotals = NULL;
    ClassTotals::ClassTotalsPtr  temp;

    for  (idx = 0; (idx < QueueSize ())  &&  (classTotals == NULL); idx++)
    {
      temp = IdxToPtr (idx);
      if  (temp->nameUpper == _name)
      {
        classTotals = temp;
      }
    }

    return  classTotals;
  }
};  /* ClassTotalsList */




void  SizeDistribution::ClassTotals::PrintFormatedLine (ostream&  o)
{
  KKStr  s (name);
  s.RightPad (20);
  o << s;

  s = count;
  s.LeftPad (9);
  o << s;

  int32  bucket;

  for (bucket = 0;  bucket < bucketCount; bucket++)
  {
    s = sizeBuckets [bucket];
    s.LeftPad (8);
    o << s;
  }
  o << endl;
}  /* PrintFormatedLine */







void  SizeDistribution::ClassTotals::PrintCSVLine (ostream&  o)
{
  o << "\"" << name << "\"," << count;

  int32  bucket;

  for (bucket = 0;  bucket <  bucketCount; bucket++)
  {
    o << "," << sizeBuckets[bucket];
  }
  o << endl;
}




void  SizeDistribution::ClassTotals::PrintTabDelLine (ostream&  o)
{
  o << "\"" << name << "\"" << "\t" << count;

  int32  bucket;

  for (bucket = 0;  bucket <  bucketCount; bucket++)
  {
    o << "\t" << sizeBuckets[bucket];
  }
  o << endl;
}






void   SizeDistribution::ClassTotals::AddIn (ClassTotalsPtr  classTotals)
{
  int32  idx;

  count = count + classTotals->count;

  for  (idx = 0; idx < bucketCount; idx++)
  {
    sizeBuckets[idx] = sizeBuckets[idx] + classTotals->sizeBuckets[idx];
  }

}  /* AddIn */




SizeDistribution::SizeDistribution (int32    _bucketCount,
                                    int32    _bucketSize,
                                    RunLog&  _log
                                   ):
  bucketCount  (_bucketCount),
  bucketSize   (_bucketSize),
  log          (_log),
  totals (new SizeDistribution::ClassTotalsList (true))

{
  if  (bucketCount < 1)
  {
    log.Level (-1) << endl << endl << endl
                   << "SizeDistribution::SizeDistribution     *** Invalid Bucket Size[" << bucketSize << "] ***" << endl
                   << endl;
  }
}


  
SizeDistribution::~SizeDistribution ()
{
  delete  totals;
}



MLClassListPtr   SizeDistribution::BuildImageClassList ()  const
{
  MLClassListPtr  mlClasses = new MLClassList ();

  ClassTotalsList::const_iterator  cIDX;
  for  (cIDX = totals->begin ();  cIDX != totals->end ();  cIDX++)
  {
    ClassTotals::ClassTotalsPtr  ct = *cIDX;
    mlClasses->PushOnBack (MLClass::CreateNewMLClass (ct->name));
  }

  mlClasses->SortByName ();

  return  mlClasses;
}  /* BuildImageClassList */



void   SizeDistribution::Increment (MLClassPtr   mlClass,
                                    int32           size
                                   )
{
  if  (mlClass == NULL)
    mlClass = MLClass::GetUnKnownClassStatic ();

  ClassTotals::ClassTotalsPtr  classTotals = totals->LookUp (mlClass->Name ());
  if  (!classTotals)
  {
    classTotals = new ClassTotals (mlClass->Name (), bucketCount, bucketSize);
    totals->PushOnBack (classTotals);
  }    

  classTotals->Increment (size);
}  /* Increment */





void   SizeDistribution::PrintFormatedDistributionMatrix (ostream&  o)  const
{
  PrintFormatedHeader (o);

  ClassTotals::ClassTotalsPtr  classTotals = NULL;

  ClassTotals  grandTotals ("Grand Totals", bucketCount, bucketSize);

  int32  idx;

  for  (idx = 0;  idx < totals->QueueSize (); idx++)
  {
    classTotals = totals->IdxToPtr (idx);
    classTotals->PrintFormatedLine (o);
    grandTotals.AddIn (classTotals);
  }

  o << endl;
  grandTotals.PrintFormatedLine (o);
}  /* PrintFormatedDistributionMatrix */






void   SizeDistribution::PrintCSVDistributionMatrix (ostream&  o)  const
{
  PrintCSVHeader (o);

  ClassTotals::ClassTotalsPtr  classTotals = NULL;

  ClassTotals  grandTotals ("Grand Totals", bucketCount, bucketSize);

  int32  idx;

  for  (idx = 0;  idx < totals->QueueSize (); idx++)
  {
    classTotals = totals->IdxToPtr (idx);
    classTotals->PrintCSVLine (o);
    grandTotals.AddIn (classTotals);
  }

  o << endl;
  grandTotals.PrintCSVLine (o);
}  /* PrintCSVDistributionMatrix */




void   SizeDistribution::PrintTabDelDistributionMatrix (ostream&  o)  const
{
  PrintTabDelHeader (o);

  ClassTotals::ClassTotalsPtr  classTotals = NULL;

  ClassTotals  grandTotals ("Grand Totals", bucketCount, bucketSize);

  int32  idx;

  for  (idx = 0;  idx < totals->QueueSize (); idx++)
  {
    classTotals = totals->IdxToPtr (idx);
    classTotals->PrintTabDelLine (o);
    grandTotals.AddIn (classTotals);
  }

  o << endl;
  grandTotals.PrintTabDelLine (o);
}  /* PrintTabDelDistributionMatrix */







void  SizeDistribution::PrintFormatedHeader (ostream&  o)  const
{
   o << "Class Name              TOTAL";
   int32  imageSize = 0;
   int32  bucket;

   for (bucket = 0;  bucket <  (bucketCount - 1); bucket++)
   {
      imageSize = imageSize + bucketSize;
      KKStr  sizeStr;
      sizeStr << imageSize;
      sizeStr.LeftPad (8);
      o << sizeStr;
   }

   KKStr  s = ">";
   s << imageSize;
   s.LeftPad (8);
   o << s;

   o << endl;

   o << "==================      =====";

   for (bucket = 0;  bucket <  bucketCount; bucket++)
   {
      o << "    ====";
   }
   o << endl;
}  /* PrintFormatedHeader */




void  SizeDistribution::PrintCSVHeader (ostream&  o)  const
{
   o << "\"Class Name\",Sum,";

   int32  imageSize = 0;
   int32  bucket;

   for (bucket = 0;  bucket <  (bucketCount - 1); bucket++)
   {
      imageSize = imageSize + bucketSize;
      o << "," << imageSize;
   }

   o << ",>" << imageSize;

   o << endl;
}  /* PrintCSVHeader */




void  SizeDistribution::PrintTabDelHeader (ostream&  o)  const
{
   o << "\"Class Name\"" << "\t" << "Sum";

   int32  imageSize = 0;
   int32  bucket;

   for (bucket = 0;  bucket <  (bucketCount - 1); bucket++)
   {
      imageSize = imageSize + bucketSize;
      o << "\t" << imageSize;
   }

   o << "\t" << ">" << imageSize;

   o << endl;
}  /* PrintTabDelHeader */




void  SizeDistribution::PrintByClassCollumns (ostream&      o,
                                              VectorUlong*  scanLinesPerMeterDepth
                                             )  const
{

  KKStr  hd1, hd2, hd3;


  MLClassListPtr  classes = BuildImageClassList ();
  MLClassList::const_iterator  cIDX;
 

  // Find the first and last buckets with activity
  
  int32  firstBucket = -1;
  int32  lastBucket = 0;
  
  for  (int32 bucketIDX = 0;  bucketIDX < bucketCount;  bucketIDX++)
  {
    int32  bucketTotal = 0;
    for  (cIDX = classes->begin ();  cIDX != classes->end ();  cIDX++)
    {
      MLClassPtr  mlClass = *cIDX;
      ClassTotals::ClassTotalsPtr  classTotals = totals->LookUp (mlClass->Name ());
      bucketTotal += classTotals->BucketCount (bucketIDX);
    }

    if  (bucketTotal > 0)
    {
      if  (firstBucket < 0)
        firstBucket = bucketIDX;
      lastBucket = bucketIDX;
    }
  }

  if  (firstBucket < 0)
  {
    o << endl << endl
      << "SizeDistribution::PrintByClassCollumns     *** There is no SizeDistribution Data ***" << endl
      << endl;
    return;
  }


  VectorInt  finaleTotals (classes->size (), 0);
  int32      grandTotal = 0;

  classes->ExtractThreeTitleLines (hd1, hd2, hd3);

  o << ""      << "\t" << ""      << "\t" << hd1 << "\t" << ""       << endl
    << ""      << "\t" << "Scan"  << "\t" << hd2 << "\t" << "Bucket" << endl
    << "Depth" << "\t" << "Lines" << "\t" << hd3 << "\t" << "Total"  << endl;

  uint32 totalScanLines = 0;
  int32  imageSize = firstBucket * bucketSize;

  for  (int32 bucketIDX = firstBucket;  bucketIDX <= lastBucket;  bucketIDX++)
  {
    int32  nextImageSize = imageSize + bucketSize;
    uint32 scanLinesDepthForThisBucket = 0;
    if  (scanLinesPerMeterDepth != NULL)
    {
      for  (int32 x = imageSize;  x < Min (nextImageSize, (int32)scanLinesPerMeterDepth->size ());  x++)
        scanLinesDepthForThisBucket += (*scanLinesPerMeterDepth)[x];
    }

    o <<  imageSize << "\t" << scanLinesDepthForThisBucket;
    totalScanLines += scanLinesDepthForThisBucket;

    int32  bucketTotal = 0;

    int32  intIDX = 0;
    for  (cIDX = classes->begin ();  cIDX != classes->end ();  cIDX++)
    {
      MLClassPtr  mlClass = *cIDX;
      ClassTotals::ClassTotalsPtr  classTotals = totals->LookUp (mlClass->Name ());

      int32  qtyThisBucket = classTotals->BucketCount (bucketIDX);

      o << "\t" << qtyThisBucket;

      bucketTotal           += qtyThisBucket;
      finaleTotals[intIDX]  += qtyThisBucket;
      grandTotal            += qtyThisBucket;

      intIDX++;
    }

    imageSize = nextImageSize;
    o << "\t" << bucketTotal << endl;
  }

  {
    o << endl
      << "FinaleTotals"  << "\t"  << totalScanLines;
    for  (uint32 x = 0;  x < classes->size ();  x++)
    {
      o << "\t" << finaleTotals[x];
    }
    o << "\t" << grandTotal << endl
      << endl;
  }

  delete  classes;  classes = NULL;
}  /* ReportByClassCollumns */
