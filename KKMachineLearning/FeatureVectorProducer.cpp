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

#include "GlobalGoalKeeper.h"
#include "KKBaseTypes.h"
using namespace  KKB;



#include "FeatureVectorProducer.h"
#include "FeatureVector.h"
using namespace  KKMachineLearning;



FeatureVectorProducer::FeatureVectorProducer (const KKStr&          _name,
                                              FactoryFVProducerPtr  _factory  /**<  Pointer to factory that instatiated this instance. */
                                              ):
    factory  (_factory),
    fileDesc (NULL),
    name     (_name)
{
}




FeatureVectorProducer::~FeatureVectorProducer ()
{
}




FileDescConstPtr  FeatureVectorProducer::FileDesc ()  const
{
  if  (!fileDesc)
  {
    GlobalGoalKeeper::StartBlock ();
    if  (!fileDesc)
    {
      fileDesc = DefineFileDesc ();
      fileDesc = FileDesc::GetExistingFileDesc (fileDesc);
    }
    GlobalGoalKeeper::EndBlock ();
  }
  retunrn  fileDesc;
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
