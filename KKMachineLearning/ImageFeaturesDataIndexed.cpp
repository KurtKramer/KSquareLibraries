#include "FirstIncludes.h"
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <string>
#include <limits>
#include <iostream>
#include <fstream>
#include <vector>
#include "MemoryDebug.h"
using namespace  std;


#include "KKBaseTypes.h"
using namespace  KKB;


#include "ImageFeaturesDataIndexed.h"
#include "FeatureVector.h"
using namespace  KKMLL;



typedef  ImageDataTreeEntry*  ImageDataTreeEntryPtr;


ExtractFeatureData  extractFeatureData;



ImageFeaturesDataIndexed::ImageFeaturesDataIndexed ():
   RBTree<ImageDataTreeEntry, ExtractFeatureData, ImageFeaturesNodeKey> (extractFeatureData, true)
{
}



ImageFeaturesDataIndexed::ImageFeaturesDataIndexed (const FeatureVectorList&  examples):
   RBTree<ImageDataTreeEntry, ExtractFeatureData, ImageFeaturesNodeKey> (extractFeatureData, true)
{
  FeatureVectorPtr  example = NULL;
  for  (auto  idx:  examples)
  {
    example = idx;
    RBInsert (example);
  }
}





void  ImageFeaturesDataIndexed::RBInsert (FeatureVectorPtr  example)
{
  RBTree<ImageDataTreeEntry, ExtractFeatureData, ImageFeaturesNodeKey>::RBInsert (new ImageDataTreeEntry (example));
}





FeatureVectorPtr  ImageFeaturesDataIndexed::GetEqual (const FeatureVectorPtr  example)
{
  ImageDataTreeEntryPtr entry = new ImageDataTreeEntry (example);

  ImageDataTreeEntryPtr  newEntry = RBTree<ImageDataTreeEntry, ExtractFeatureData, ImageFeaturesNodeKey>::GetEqual (entry->NodeKey ());

  FeatureVectorPtr  exampleFound = NULL;
  if  (newEntry)
    exampleFound = newEntry->Example ();

  delete  entry;  entry = NULL;
                                                                                                          
  return  exampleFound;
}





ImageDataTreeEntry::ImageDataTreeEntry (FeatureVectorPtr  _example):
         example  (_example),
         nodeKey  (_example)

{
}



ImageFeaturesNodeKey::ImageFeaturesNodeKey (FeatureVectorPtr  _example):
  example (_example)
{
}




bool  ImageFeaturesNodeKey::operator== (const ImageFeaturesNodeKey& rightNode)  const
{
  return  (CompareTwoExamples (example, rightNode.example) == 0);
}

bool   ImageFeaturesNodeKey::operator< (const ImageFeaturesNodeKey& rightNode)  const
{
  return  (CompareTwoExamples (example, rightNode.example) < 0);
}

bool   ImageFeaturesNodeKey::operator> (const ImageFeaturesNodeKey& rightNode)  const
{
  return  (CompareTwoExamples (example, rightNode.example) > 0);
}



kkint32  ImageFeaturesNodeKey::CompareTwoExamples (const FeatureVectorPtr i1,
                                                   const FeatureVectorPtr i2
                                                 )  const
{
  const float*  f1 = i1->FeatureDataConst ();
  const float*  f2 = i2->FeatureDataConst ();

  for  (kkint32 x = 0;  x < i1->NumOfFeatures ();  x++)
  {
    if  (f1[x] < f2[x])
      return -1;
    else if  (f1[x] > f2[x])
      return 1;
  }

  return 0;
} /* CompareTwoImageFeaturesObjects */

