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
#include "ImageIO.h"
#include "KKBaseTypes.h"
using namespace  KKB;



#include "FeatureVectorProducer.h"
#include "FeatureVector.h"
using namespace  KKMachineLearning;



FeatureVectorProducer::FeatureVectorProducer (const KKStr&          _name,
                                              FactoryFVProducerPtr  _factory  /**<  Pointer to factory that instantiated this instance. */
                                              ):
    factory  (_factory),
    fileDesc (NULL),
    name     (_name)
{
}




FeatureVectorProducer::~FeatureVectorProducer ()
{
}




FeatureVectorPtr  FeatureVectorProducer::ComputeFeatureVectorFromImage (const KKStr&      fileName,
                                                                        const MLClassPtr  knownClass,
                                                                        RasterListPtr     intermediateImages,
                                                                        RunLog&           runLog
                                                                       )
{
  FeatureVectorPtr  fv = NULL;

  RasterPtr  i = KKB::ReadImage (fileName);
  if  (i == NULL)
  {
    runLog.Level (-1) << "FeatureVectorProducer::ComputeFeatureVectorFromImage   ***ERROR***   Error loading ImageFile: " << fileName << endl << endl;
  }
  else
  {
    fv = ComputeFeatureVector (*i, knownClass, intermediateImages, runLog);
    delete  i;
    i = NULL;
  }

  return  fv;
}  /* ComputeFeatureVectorFromImage */





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
  return  fileDesc;
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
