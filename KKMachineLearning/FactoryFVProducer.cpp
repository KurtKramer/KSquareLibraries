#include "FirstIncludes.h"

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "MemoryDebug.h"

using namespace  std;

#include "GlobalGoalKeeper.h"
#include "KKStr.h"
#include "RunLog.h"
using namespace  KKB;

#include "FeatureVector.h"
#include "FeatureVectorProducer.h"
#include "FeatureFileIO.h"

#include "FactoryFVProducer.h"
using namespace  KKMachineLearning;





FactoryFVProducer::FactoryFVProducer (const FactoryFVProducer&  factory):
     defaultFeatureFileIO (factory.defaultFeatureFileIO),
     description          (factory.description),
     fileDesc             (factory.fileDesc),
     name                 (factory.name)
{
}



FactoryFVProducer::FactoryFVProducer (const KKStr&      _name,
                                      const KKStr&      _description,
                                      FileDescPtr       _fileDesc,
                                      FeatureFileIOPtr  _defaultFeatureFileIO
                                     ):
     description (_description),
     fileDesc    (_fileDesc),
     name        (_name)
{
}




FactoryFVProducer::~FactoryFVProducer ()
{
}



FeatureVectorListPtr  FactoryFVProducer::ManufacturFeatureVectorList (RunLog&  runLog)
{
  return  new FeatureVectorList (fileDesc, true, runLog);
}




FeatureVectorProducerPtr  FactoryFVProducer::ManufacturInstance (const KKStr&  _name,
                                                                 RunLog&       runLog
                                                                )
{
  FactoryFVProducerPtr  factory = LookUpFactory (_name);
  if  (factory)
    return factory->ManufacturInstance (runLog);
  else
    return NULL;
}




FactoryFVProducerPtr  FactoryFVProducer::LookUpFactory (const KKStr&  _name)
{
  FactoryFVProducerPtr  factory = NULL;

  GlobalGoalKeeper::StartBlock ();

  factoriesIdx = factories.find (_name);
  if  (factoriesIdx != factories.end ())
    factory = factoriesIdx->second;

  GlobalGoalKeeper::EndBlock ();
  return  factory;
}  /* LookUpFactory */





void  FactoryFVProducer::RegisterFactory (FactoryFVProducerPtr  factory,
                                          RunLog&               runLog
                                         )
{
  if  (factory == NULL)
  {
    runLog.Level (-1) << "FactoryFVProducer::RegisterFactory   ***ERROR***   factory==NULL" << endl;
    return;
  }

  GlobalGoalKeeper::StartBlock ();

  runLog.Level (30) << "FactoryFVProducer::RegisterFactory  Name: " << factory->Name () << "  Description: " << factory->Description () << endl;

  if  (!atExitDefined)
  {
    atexit (FactoryFVProducer::FinaleCleanUp);
    atExitDefined = true;
  }

  factoriesIdx = factories.find (factory->Name ());
  if  (factoriesIdx != factories.end ())
  {
    runLog.Level (-1) << "FactoryFVProducer::RegisterFactory   ***ERROR***   Factory With Name: " << factory->Name () << " alreadt defined." << endl;
  }
  else
  {
    factories.insert (pair<KKStr,FactoryFVProducerPtr> (factory->Name (), factory));
  }

  GlobalGoalKeeper::EndBlock ();
  return;
}  /* RegisterFactory */





void  FactoryFVProducer::FinaleCleanUp ()
{
  GlobalGoalKeeper::StartBlock ();

  for  (factoriesIdx = factories.begin ();  factoriesIdx != factories.end ();  ++factoriesIdx)
  {
    delete factoriesIdx->second;
    factoriesIdx->second = NULL;
  }

  atExitDefined = false;
  GlobalGoalKeeper::EndBlock ();
}  /* CleanUp */





bool  FactoryFVProducer::atExitDefined = false;

map<KKStr,FactoryFVProducerPtr>  FactoryFVProducer::factories;

map<KKStr,FactoryFVProducerPtr>::iterator  FactoryFVProducer::factoriesIdx;

