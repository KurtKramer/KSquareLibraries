#include "FirstIncludes.h"
#include <ctype.h>
#include <math.h>
#include <time.h>

#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string.h>
#include <typeinfo>
#include "MemoryDebug.h"
using namespace  std;


#include "KKBaseTypes.h"
using namespace  KKB;



#include "FeatureVectorProducer.h"
#include "FeatureVector.h"
using namespace  KKMachineLearning;



FeatureVectorProducer::FeatureVectorProducer (const KKStr&  _name,
                                              FileDescPtr   _fileDesc
                                             ):
    fileDesc (_fileDesc),
    name     (_name)
{
}



FeatureVectorProducer::~FeatureVectorProducer ()
{
}



void  FeatureVectorProducer::SetFileDesc (FileDescPtr  _fileDesc)
{
  fileDesc = _fileDesc;
}





kkuint32  FeatureVectorProducer::FeatureCount ()  const
{
  if  (!fileDesc)
    return 0;
  else
    return fileDesc->NumOfFields ();
}



const KKStr&  FeatureVectorProducer::FeatureName (kkuint32  fieldNum)  const
{
  if  (fileDesc == NULL)
    return KKStr::EmptyStr ();

  if  (fieldNum >= fileDesc->NumOfFields ())
    return KKStr::EmptyStr ();
  else
    return  fileDesc->FieldName (fieldNum);
}






bool  FeatureVectorProducer::atExitDefined = false;
