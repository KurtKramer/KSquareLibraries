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



void  TrainingClass::AddDirectories  (const VectorKKStr&  _directories)
{
  VectorKKStr::const_iterator  idx;
  for  (idx = _directories.begin ();  idx != _directories.end ();  ++idx)
    AddDirectory (*idx);
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

  directory = osSubstituteInEnvironmentVariables (directory);

  char  firstChar = directory.FirstChar ();
  if  ((firstChar == '/')  ||  (firstChar == '\\'))
    return  directory;

  if  (directory[1] == ':')
    return directory;

  return rootDirWithSlash + directory;
}  /* ExpandedDirectory */



void  TrainingClass::WriteXML (const KKStr&  varName,
                               ostream&      o
                              )  const
{
  XmlTag  startTag ("TrainingClass", XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);

  if  (mlClass)
    mlClass->Name ().WriteXML ("MLClass", o);
  
  if  (!featureFileName.Empty ())
    featureFileName.WriteXML ("FeatureFileName", o);

  if  (weight  != 0.0f)
    XmlElementFloat::WriteXML (weight, "Weight", o);

  if  (countFactor != 0.0f)
    XmlElementFloat::WriteXML (countFactor, "CountFactor", o);

  if  (subClassifier)
    subClassifier->ConfigRootName ().WriteXML ("SubClassifierName", o);

  if  (directories.size () > 0)
    XmlElementVectorKKStr::WriteXML (directories, "Directories", o);

  XmlTag  endTag ("TrainingClass", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}  /* WriteXML */



void  TrainingClass::ReadXML (XmlStream&      s,
                              XmlTagConstPtr  tag,
                              RunLog&         log
                             )
{
  XmlTokenPtr  t = s.GetNextToken (log);
  while  (t)
  {
    if  (t->TokenType () == XmlToken::TokenTypes::tokElement)
    {
      XmlElementPtr  e = dynamic_cast<XmlElementPtr> (t);
      const KKStr&  secrtionName = e->SectionName ();
      const KKStr&  varName      = e->VarName ();
      if  (varName.EqualIgnoreCase ("MLClass"))
      {
        XmlElementKKStrPtr  eKKStr = dynamic_cast<XmlElementKKStrPtr>(e);
        if  (eKKStr)
          MLClass (MLClass::CreateNewMLClass (*(eKKStr->Value ())));
      }

      else if  (varName.EqualIgnoreCase ("FeatureFileName"))
      {
        XmlElementKKStrPtr  eKKStr = dynamic_cast<XmlElementKKStrPtr>(e);
        if  (eKKStr  &&  (eKKStr->Value ()))
          FeatureFileName (*(eKKStr->Value ()));
      }

      else if  (varName.EqualIgnoreCase ("Weight"))
      {
        XmlElementFloatPtr  f = dynamic_cast<XmlElementFloatPtr>(e);
        if  (f)
          Weight (f->Value ());
      }

      else if  (varName.EqualIgnoreCase ("CountFactor"))
      {
        XmlElementFloatPtr  cf = dynamic_cast<XmlElementFloatPtr>(e);
        if  (cf)
          CountFactor (cf->Value ());
      }

      else if  (varName.EqualIgnoreCase ("SubClassifierName"))
      {
        XmlElementKKStrPtr  scn = dynamic_cast<XmlElementKKStrPtr>(e);
        if  (scn  &&  (scn->Value ()))
          FeatureFileName (*(scn->Value ()));
      }

      else if  (varName.EqualIgnoreCase ("Directories"))
      {
        XmlElementVectorKKStrPtr  d = dynamic_cast<XmlElementVectorKKStrPtr>(e);
        if  (d  &&  (d->Value () != NULL))
          AddDirectories (*(d->Value ()));
      }
    }
    XmlTokenPtr t = s.GetNextToken (log);
  }
}  /* ReadXML */



TrainingClassList::TrainingClassList ():
    KKQueue<TrainingClass> (true),
    rootDir                ()
{
}



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




void  TrainingClassList::WriteXML (const KKStr&  varName,
                                   ostream&      o
                                  )  const
{
  XmlTag  tagStart ("TrainingClassList", XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    tagStart.AddAtribute ("VarName", varName);
  tagStart.AddAtribute ("Count", (kkint32)size ());
  tagStart.WriteXML (o);
  o << endl;

  if  (!rootDir.Empty ())
    rootDir.WriteXML ("RootDir", o);

  TrainingClassList::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  ++idx)
  {
    TrainingClassPtr  tc = *idx;
    XmlElementTrainingClass::WriteXML (*tc, KKStr::EmptyStr (), o);
  }

  XmlTag  tagEnd ("TrainingClassList", XmlTag::TagTypes::tagEnd);
  tagEnd.WriteXML (o);
}



void  TrainingClassList::ReadXML (XmlTagPtr   tag,
                                  XmlStream&  s,
                                  RunLog&     log
                                 )
{
  XmlTokenPtr t = s.GetNextToken (log);
  while  (t)
  {
    if  (t->TokenType () == XmlToken::TokenTypes::tokElement)
    {
      XmlElementPtr e = dynamic_cast<XmlElementPtr> (t);
      if  (e)
      {
        KKStr varName = e->VarName ();
        if  (typeid (*e) == typeid (XmlElementKKStr))
        {
          XmlElementKKStrPtr  s = dynamic_cast<XmlElementKKStrPtr> (e);
          if  (s)
          {
            if  (varName.EqualIgnoreCase ("RootDir"))
              RootDir (s->Value ());
          }
        }
        else if   (typeid (*t)  ==  typeid (XmlElementTrainingClass))
        {
          XmlElementTrainingClassPtr tokenTrainingClass = dynamic_cast<XmlElementTrainingClassPtr> (t);
          TrainingClassPtr  tc = tokenTrainingClass->TakeOwnership ();
          if  (tc)
            AddTrainingClass (tc);
           tc = NULL;
        }
        else
        {
          log.Level (-1) << "XmlElementTrainingClassList   ***ERROR***   Un-expected Section Element[" << e->SectionName () << "]" << endl;
        }
      }
    }

    t = s.GetNextToken (log);
  }
}  /* ReadXML */


XmlFactoryMacro(TrainingClass)
XmlFactoryMacro(TrainingClassList)



