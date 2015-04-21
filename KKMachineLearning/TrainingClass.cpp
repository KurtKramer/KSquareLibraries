#include "FirstIncludes.h"
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include "MemoryDebug.h"
using namespace  std;

#include "GlobalGoalKeeper.h"
#include "KKBaseTypes.h"
#include "OSservices.h"
#include "XmlStream.h"
using namespace  KKB;


#include "TrainingClass.h"
#include "TrainingConfiguration2.h"
#include "MLClass.h"
using namespace  KKMLL;



TrainingClass::TrainingClass ():
     directories     (),
     featureFileName (),
     weight          (0.0f),
     countFactor     (0.0f),
     subClassifier   (NULL),
     mlClass         (NULL)
{
}


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
    return rootDirWithSlash + osSubstituteInEnvironmentVariables (directory);
}  /* ExpandedDirectory */





void  XmlElementTrainingClass::WriteXML (const TrainingClass&  tc,
                                         const KKStr&          varName,
                                         ostream&              o
                                        )
{
  XmlTag  t ("TrainingClass", XmlTag::tagEmpty);
  if  (!varName.Empty ())
    t.AddAtribute ("VarName", varName);

  t.AddAtribute ("MlClass", tc.MLClass ()->Name ());

  if  (!tc.FeatureFileName ().Empty ())
    t.AddAtribute ("FeatureFileName", tc.FeatureFileName ());

  if  (tc.Weight () != 0.0f)

  if  (tc.CountFactor () != 0.0f)
    t.AddAtribute ("CountFactor", tc.CountFactor ());

  if  (tc.SubClassifier ())
    t.AddAtribute ("SubClassifier", tc.SubClassifier ()->ConfigRootName ());

  kkint32  x = 0;
  VectorKKStr::const_iterator  idx;
  for  (idx = tc.Directories ().begin (); idx != tc.Directories ().end ();  ++idx)
  {
    if  (!idx->Empty ())
      t.AddAtribute ("Dir_" + StrFormatInt (x, "00"), *idx);
    ++x;
  }

  t.WriteXML (o);
}





XmlElementTrainingClass::XmlElementTrainingClass (XmlTagPtr   tag,
                                                  XmlStream&  s,
                                                  RunLog&     log
                                                 ):
  XmlElement (tag, s, log),
  value (NULL)
{
  value = new TrainingClass ();
  kkuint32  c = tag->AttributeCount ();
  for  (kkuint32 x = 0;  x < c;  ++x)
  {
    const KKStr&  n = tag->AttributeName (x);
    const KKStr&  v = tag->AttributeValue (x);
    if  (n.EqualIgnoreCase ("MlClass"))
      value->MLClass (MLClass::CreateNewMLClass (n, -1));

    else if  (n.EqualIgnoreCase ("FeatureFileName"))
      value->FeatureFileName (v);

    else if  (n.EqualIgnoreCase ("Weight"))
      value->Weight (v.ToFloat ());

    else if  (n.EqualIgnoreCase ("CountFactor"))
      value->CountFactor (v.ToFloat ());

    else if  (n.EqualIgnoreCase ("SubClassifier"))
      value->SubClassifierName (v);

    else if  (n.StartsWith ("Dir_"))
      value->AddDirectory (v);
  }
  
  XmlTokenPtr t = s.GetNextToken (log);
  while  (t)
    t = s.GetNextToken (log);
}



XmlElementTrainingClass::~XmlElementTrainingClass ()
{
  delete  value;
  value = NULL;
}



TrainingClassPtr  XmlElementTrainingClass::Value ()  const
{
  return value;
}


TrainingClassPtr  XmlElementTrainingClass::TakeOwnership () 
{
  TrainingClassPtr t = value;
  value = NULL;
  return t;
}


XmlFactoryMacro(TrainingClass)



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



void  XmlElementTrainingClassList::WriteXML (const TrainingClassList&  tcl,
                                             const KKStr&              varName,
                                             ostream&                  o
                                           ) 

{
  XmlTag  tagStart ("TrainingClassList", XmlTag::tagStart);
  if  (!varName.Empty ())
    tagStart.AddAtribute ("VarName", varName);
  tagStart.AddAtribute ("Count", (kkint32)tcl.size ());
  if  (!tcl.RootDir ().Empty ())
    tagStart.AddAtribute ("RootDir", tcl.RootDir ());

  tagStart.WriteXML (o);

  TrainingClassList::const_iterator  idx;
  for  (idx = tcl.begin ();  idx != tcl.end ();  ++idx)
  {
    TrainingClassPtr  tc = *idx;
    XmlElementTrainingClass::WriteXML (*tc, KKStr::EmptyStr (), o);
  }

  XmlTag  tagEnd ("TrainingClassList", XmlTag::tagEnd);
  tagEnd.WriteXML (o);
}  /* WriteXML */





XmlElementTrainingClassList::XmlElementTrainingClassList (XmlTagPtr   tag,
                                                          XmlStream&  s,
                                                          RunLog&     log
                                                         ):
  XmlElement (tag, s, log),
  value (NULL)
{
  KKStr  name;
  KKStr  rootDir;
  kkuint32  count = 0;
  kkuint32  c = tag->AttributeCount ();
  for  (kkuint32 x = 0;  x < c;  ++x)
  {
    const KKStr&  n = tag->AttributeName (x);
    const KKStr&  v = tag->AttributeValue (x);
    if  (n.EqualIgnoreCase ("Name"))
      name = v;

    else if  (n.EqualIgnoreCase ("RootDir"))
      rootDir = v;

    else if  (n.EqualIgnoreCase ("Count"))
      count = v.ToUint32 ();
  }
  
  value = new TrainingClassList (rootDir, true);

  XmlTokenPtr t = s.GetNextToken (log);
  while  (t)
  {
    if  (t->TokenType () != XmlToken::tokElement)
      continue;

    XmlElementTrainingClassPtr tokenTrainingClass = dynamic_cast<XmlElementTrainingClassPtr> (t);
    TrainingClassPtr  tc = tokenTrainingClass->TakeOwnership ();
    if  (tc)
      value->AddTrainingClass (tc);
    tc = NULL;

    t = s.GetNextToken (log);
  }

  return;
}



XmlElementTrainingClassList::~XmlElementTrainingClassList ()
{
  delete  value;
  value = NULL;
}



TrainingClassListPtr  XmlElementTrainingClassList::Value ()  const
{
  return value;
}


TrainingClassListPtr  XmlElementTrainingClassList::TakeOwnership () 
{
  TrainingClassListPtr t = value;
  value = NULL;
  return t;
}


XmlFactoryMacro(TrainingClassList)



