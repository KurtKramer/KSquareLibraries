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
using namespace  KKMLL;






FactoryFVProducer::FactoryFVProducer (const KKStr&  _name,
                                      const KKStr&  _fvClassName,
                                      const KKStr&  _description
                                     ):
     description (_description),
     fvClassName (_fvClassName),
     name        (_name)
{
}




FactoryFVProducer::~FactoryFVProducer ()
{
}


FeatureVectorPtr  FactoryFVProducer::ManufacturFeatureVector (kkint32  numOfFeatires,
                                                              RunLog&  runLog
                                                             )
{
  return  new FeatureVector (numOfFeatires);
}


FeatureVectorListPtr  FactoryFVProducer::ManufacturFeatureVectorList (bool     owner,
                                                                      RunLog&  runLog
                                                                     )
{
  return  new FeatureVectorList (FileDesc (), owner);
}


FeatureVectorProducerPtr  FactoryFVProducer::ManufactureInstance (const KKStr&  name,
                                                                  RunLog&       runLog
                                                                 )
{
  FactoryFVProducerPtr  factory = LookUpFactory (name);
  if  (factory)
    return factory->ManufactureInstance (runLog);
  else
    return NULL;
}




FactoryFVProducerPtr  FactoryFVProducer::LookUpFactory (const KKStr&  _name)
{
  FactoryFVProducerPtr  factory = NULL;

  GlobalGoalKeeper::StartBlock ();

  if  (!factories)
  {
    factories = new FactoryMap ();
    atexit (FactoryFVProducer::FinaleCleanUp);
    atExitDefined = true;
  }

  FactoryMap::iterator idx;

  idx = factories->find (_name);
  if  (idx != factories->end ())
    factory = idx->second;

  GlobalGoalKeeper::EndBlock ();
  return  factory;
}  /* LookUpFactory */





void  FactoryFVProducer::RegisterFactory (FactoryFVProducerPtr  factory,
                                          RunLog*               runLog
                                         )
{
  bool  weOwnRunLog = false;
  if  (runLog == NULL)
  {
    runLog = new RunLog ();
    weOwnRunLog = true;
  }

  if  (factory == NULL)
  {
    runLog->Level (-1) << "FactoryFVProducer::RegisterFactory   ***ERROR***   factory==NULL" << endl;
  }
  else
  {
    GlobalGoalKeeper::StartBlock ();

    runLog->Level (30) << "FactoryFVProducer::RegisterFactory  Name: " << factory->Name () << "  Description: " << factory->Description () << endl;

    if  (!factories)
    {
      factories = new FactoryMap ();
      atexit (FactoryFVProducer::FinaleCleanUp);
      atExitDefined = true;
    }

    FactoryMap::iterator idx;

    idx = factories->find (factory->Name ());
    if  (idx != factories->end ())
    {
      runLog->Level (-1) << "FactoryFVProducer::RegisterFactory   ***ERROR***   Factory With Name: " << factory->Name () << " already defined." << endl;
    }
    else
    {
      factories->insert (pair<KKStr,FactoryFVProducerPtr> (factory->Name (), factory));
    }

    GlobalGoalKeeper::EndBlock ();
  }

  if  (weOwnRunLog)
  {
    delete  runLog;
    runLog = NULL;
  }
  return;
}  /* RegisterFactory */





void  FactoryFVProducer::FinaleCleanUp ()
{
  GlobalGoalKeeper::StartBlock ();

  FactoryMap::iterator idx;

  for  (idx = factories->begin ();  idx != factories->end ();  ++idx)
  {
    delete idx->second;
    idx->second = NULL;
  }

  delete  factories;
  factories = NULL;

  atExitDefined = false;
  GlobalGoalKeeper::EndBlock ();
}  /* CleanUp */





bool  FactoryFVProducer::atExitDefined = false;

FactoryFVProducer::FactoryMap* FactoryFVProducer::factories = NULL;

