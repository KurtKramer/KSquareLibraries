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




ExtractExampleFileName  extractImageFileName;



ImageFeaturesNameIndexed::ImageFeaturesNameIndexed ():
   RBTree<FeatureVector, ExtractExampleFileName, KKStr> (extractImageFileName, false)
{
}



ImageFeaturesNameIndexed::ImageFeaturesNameIndexed (const FeatureVectorList&  examples):
   RBTree<FeatureVector, ExtractExampleFileName, KKStr> (extractImageFileName, false)
{
  FeatureVectorPtr  example = NULL;
  for  (auto idx:  examples)
  {
    example = idx;
    RBInsert (example);
  }
}





KKStr  ExtractExampleFileName::ExtractKey (FeatureVectorPtr  example)
{
  return  osGetRootName (example->ExampleFileName ());
}
