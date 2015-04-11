#include  "FirstIncludes.h"

#include  <stdio.h>
#include  <ctype.h>
#include  <time.h>

#include  <string>
#include  <iostream>
#include  <fstream>
#include  <vector>

#include  "MemoryDebug.h"

using namespace std;


#include "KKBaseTypes.h"
#include "OSservices.h"
using namespace  KKB;


#include "ImageFeaturesNameIndexed.h"
#include "FeatureVector.h"
using namespace  KKMLL;




ExtractImageFileName  extractImageFileName;



ImageFeaturesNameIndexed::ImageFeaturesNameIndexed ():
   RBTree<FeatureVector, ExtractImageFileName, KKStr> (extractImageFileName, false)
{
}



ImageFeaturesNameIndexed::ImageFeaturesNameIndexed (FeatureVectorList&  images):
   RBTree<FeatureVector, ExtractImageFileName, KKStr> (extractImageFileName, false)
{
  FeatureVectorPtr  image = NULL;

  FeatureVectorList::iterator  idx;
  for  (idx = images.begin ();  idx != images.end ();  idx++)
  {
    image = *idx;
    RBInsert (image);
  }
}





KKStr  ExtractImageFileName::ExtractKey (FeatureVectorPtr  image)
{
  return  osGetRootName (image->ImageFileName ());
}
