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



ImageFeaturesDataIndexed::ImageFeaturesDataIndexed (FeatureVectorList&  images):
   RBTree<ImageDataTreeEntry, ExtractFeatureData, ImageFeaturesNodeKey> (extractFeatureData, true)
{
  FeatureVectorPtr  image = NULL;

  FeatureVectorList::iterator idx;
  for  (idx = images.begin ();  idx != images.end ();  idx++)
  {
    image = *idx;
    RBInsert (image);
  }
}





void  ImageFeaturesDataIndexed::RBInsert (FeatureVectorPtr  image)
{
  RBTree<ImageDataTreeEntry, ExtractFeatureData, ImageFeaturesNodeKey>::RBInsert (new ImageDataTreeEntry (image));
}





FeatureVectorPtr  ImageFeaturesDataIndexed::GetEqual (const FeatureVectorPtr  image)
{
  ImageDataTreeEntryPtr entry = new ImageDataTreeEntry (image);

  ImageDataTreeEntryPtr  newEntry = RBTree<ImageDataTreeEntry, ExtractFeatureData, ImageFeaturesNodeKey>::GetEqual (entry->NodeKey ());

  FeatureVectorPtr  imageFound = NULL;
  if  (newEntry)
    imageFound = newEntry->Image ();

  delete  entry;  entry = NULL;

  return  imageFound;
}





ImageDataTreeEntry::ImageDataTreeEntry (FeatureVectorPtr  _image):
         image   (_image),
         nodeKey (_image)

{
}



ImageFeaturesNodeKey::ImageFeaturesNodeKey (FeatureVectorPtr  _image):
  image (_image)
{
}




bool  ImageFeaturesNodeKey::operator== (const ImageFeaturesNodeKey& rightNode)  const
{
  return  (CompareTwoImages (image, rightNode.image) == 0);
}

bool   ImageFeaturesNodeKey::operator< (const ImageFeaturesNodeKey& rightNode)  const
{
  return  (CompareTwoImages (image, rightNode.image) < 0);
}

bool   ImageFeaturesNodeKey::operator> (const ImageFeaturesNodeKey& rightNode)  const
{
  return  (CompareTwoImages (image, rightNode.image) > 0);
}



kkint32  ImageFeaturesNodeKey::CompareTwoImages (const FeatureVectorPtr i1,
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

