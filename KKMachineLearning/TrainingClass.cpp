#include "FirstIncludes.h"
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include "MemoryDebug.h"
using namespace  std;


#include "KKBaseTypes.h"
#include "OSservices.h"
using namespace  KKB;


#include "TrainingClass.h"
#include "MLClass.h"
using namespace  KKMLL;



TrainingClass::TrainingClass (const VectorKKStr&        _directories,
                              KKStr                     _name,
                              float                     _weight,
                              float                     _countFactor,
                              TrainingConfiguration2Ptr _subClassifier,
                              MLClassList&              _mlClasses
                             ):
                directories     (_directories),
                featureFileName (_name),
                weight          (_weight),
                countFactor     (_countFactor),
                subClassifier   (_subClassifier),
                mlClass         (NULL)
{
  featureFileName << ".data";   // Will be equal to ClassName + ".data".     ex:  "Copepods.data"

  mlClass = _mlClasses.GetMLClassPtr (_name);
  mlClass->CountFactor (_countFactor);
}



TrainingClass::TrainingClass (const TrainingClass&  tc): 
    directories     (tc.directories),
    featureFileName (tc.featureFileName),
    mlClass         (tc.mlClass),
    subClassifier   (NULL),
    weight          (tc.weight)
{
}




const 
KKStr&    TrainingClass::Name () const
{
  return  mlClass->Name ();
}




const KKStr&  TrainingClass::Directory  (kkuint32 idx) const
{
  if  (idx >= directories.size ())
    return KKStr::EmptyStr();
  else
    return  directories[idx];
}


kkuint32  TrainingClass::DirectoryCount () const
{
  return  directories.size ();
}




void  TrainingClass::AddDirectory (const KKStr&  _directory)
{
  directories.push_back (_directory);
}



void  TrainingClass::Directory (kkuint32      idx, 
                                const KKStr&  directory
                               )
{
  while  (idx >= directories.size ())
    directories.push_back ("");
  directories[idx] = directory;
}



KKStr  TrainingClass::ExpandedDirectory (const KKStr&  rootDir,
                                         kkuint32      idx
                                        )
{
  KKStr  rootDirWithSlash = "";
  if  (!rootDir.Empty ())
    rootDirWithSlash = osAddSlash (rootDir);

  KKStr  directory = "";

  if  (idx >= directories.size ())
    return  KKStr::EmptyStr ();

  directory = directories[idx];

  if  (directory.Empty ())
  {
    directory = rootDirWithSlash + mlClass->Name ();
    return  directory;
  }

  if  (directory.LocateStr (rootDirWithSlash) == 0)
    return  directory;
  else
    return rootDirWithSlash + SipperVariables::SubstituteInEvironmentVariables (directory);
}  /* ExpandedDirectory */




TrainingClassList::TrainingClassList (const KKStr&  _rootDir,
                                      bool          owner
                                     ):
    KKQueue<TrainingClass> (owner),
             rootDir                (_rootDir)
{
}



TrainingClassList::TrainingClassList (const TrainingClassList&  tcl):
           KKQueue<TrainingClass> (tcl),
           rootDir                (tcl.rootDir)
{
}



TrainingClassList::TrainingClassList (const TrainingClassList&  tcl,
                                      bool                      _owner
                                     ):
           KKQueue<TrainingClass> (tcl, _owner),
           rootDir                (tcl.rootDir)
{
}





void   TrainingClassList::AddTrainingClass (TrainingClassPtr  trainingClass)
{
   PushOnBack (trainingClass);
}



TrainingClassPtr  TrainingClassList::LocateByMLClass (MLClassPtr  _mlClass)  const
{
  TrainingClassList::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  idx++)
  {
    TrainingClassPtr  tc = *idx;
    if  (tc->MLClass () == _mlClass)
      return tc;
  }
  return  NULL;
}  /* LocateByMLClass */



TrainingClassPtr  TrainingClassList::LocateByMLClassName (const KKStr&  className)
{
  kkint32  size = QueueSize ();

  kkint32 idx = 0;

  TrainingClassPtr  traningClass = NULL;

  for  (idx = 0; ((idx < size)  &&  (!traningClass)); idx++)
  {
    TrainingClassPtr  temp = IdxToPtr (idx);

    if  (temp->Name () == className)
        traningClass = temp;
  }

  return  traningClass;
}  /* LocateByMLClassName */




TrainingClassPtr  TrainingClassList::LocateByDirectory (const KKStr&  directory)
{
  iterator  idx;

  for  (idx = begin ();  idx != end ();  idx++)
  {
    TrainingClassPtr tc = *idx;
    for  (kkuint32 zed = 0;  zed < tc->DirectoryCount ();  ++ zed)
    {
      if  (tc->ExpandedDirectory (rootDir, zed).EqualIgnoreCase (directory))
        return  tc;
    }
  }

  return  NULL;
}  /* LocateByDirectory */



TrainingClassListPtr   TrainingClassList::DuplicateListAndContents ()  const
{
  TrainingClassListPtr dupList = new TrainingClassList (rootDir, true);

  for  (const_iterator  idx = begin ();  idx != end ();  idx++)
  {
    const TrainingClassPtr  tc = *idx;
    dupList->PushOnBack (new TrainingClass (*tc));
  }

  return  dupList;
}  /* TrainingClassListPtr */
