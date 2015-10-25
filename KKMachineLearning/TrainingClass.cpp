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
     countFactor     (0.0f),
     directories     (),
     featureFileName (),
     mlClass         (NULL),
     subClassifier   (NULL),
     weight          (0.0f)
{
}


TrainingClass::TrainingClass (const VectorKKStr&        _directories,
                              KKStr                     _name,
                              float                     _weight,
                              float                     _countFactor,
                              TrainingConfiguration2Ptr _subClassifier,
                              MLClassList&              _mlClasses
                             ):
                countFactor     (_countFactor),
                directories     (_directories),
                featureFileName (_name),
                mlClass         (NULL),
                weight          (_weight),
                subClassifier   (_subClassifier)
{
  featureFileName << ".data";   // Will be equal to ClassName + ".data".     ex:  "Copepods.data"

  mlClass = _mlClasses.GetMLClassPtr (_name);
  mlClass->CountFactor (_countFactor);
}



TrainingClass::TrainingClass (const TrainingClass&  tc): 
    countFactor     (tc.countFactor),
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

  return osSubstituteInEnvironmentVariables (rootDirWithSlash + directory);
}  /* ExpandedDirectory */



void  TrainingClass::WriteXML (const KKStr&  varName,
                               ostream&      o
                              )  const
{
  WriteXML (varName, "", o);
}

void  TrainingClass::WriteXML (const KKStr&  varName,
                               const KKStr&  rootDir,
                               ostream&      o
                              )  const
{
  bool  noDirectories = false;
  bool  onlyOneDirectory = false;

  VectorKKStr  tempDirectories;
  for  (auto  idx: directories)
  {
    if  (idx.StartsWith (rootDir))
    {
      KKStr  rootPart = idx.SubStrPart (rootDir.Len ());
      rootPart.ChopFirstChar ();
      tempDirectories.push_back (rootPart);
    }
    else
    {
      tempDirectories.push_back (idx);
    }
  }

  if  (tempDirectories.size () == 1)  
  {
    if  ((tempDirectories[0].EqualIgnoreCase (mlClass->Name ()))  ||  (tempDirectories[0].Empty ()))
      noDirectories = true;
    else
      onlyOneDirectory = true;
  }

  else  if  (tempDirectories.size () < 1)
    noDirectories = true;

  XmlTag::TagTypes  startTagType = XmlTag::TagTypes::tagEmpty;
  if  (!noDirectories)
    startTagType = XmlTag::TagTypes::tagStart;

  XmlTag  startTag ("TrainingClass", startTagType);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);

  if  (mlClass)
    startTag.AddAtribute ("mlClass",  mlClass->Name ());
  
  if  (!featureFileName.Empty ())
    startTag.AddAtribute ("FeatureFileName",  featureFileName);

  if  (weight  != 0.0f)
    startTag.AddAtribute ("Weight",  weight);

  if  (countFactor != 0.0f)
    startTag.AddAtribute ("CountFactor",  countFactor);

  if  (subClassifier)
    startTag.AddAtribute ("SubClassifier", SubClassifierName ());

  startTag.WriteXML (o);
  o << endl;

  if  (startTagType == XmlTag::TagTypes::tagStart)
  {
    for  (auto idx: tempDirectories)
      idx.WriteXML ("Directory", o);

    XmlTag  endTag ("TrainingClass", XmlTag::TagTypes::tagEnd);
    endTag.WriteXML (o);
    o << endl;
  }
}  /* WriteXML */



void  TrainingClass::ReadXML (XmlStream&      s,
                              XmlTagConstPtr  tag,
                              VolConstBool&   cancelFlag,
                              RunLog&         log
                             )
{
  directories.clear ();
  for  (auto idx:  tag->Attributes ())
  {
    const KKStr&  n = idx->Name ();
    const KKStr&  v = idx->Value ();

    if  (n.EqualIgnoreCase ("MLClass"))
      MLClass (MLClass::CreateNewMLClass (v));

    else if  (n.EqualIgnoreCase ("FeatureFileName"))
       FeatureFileName (v);

    else if  (n.EqualIgnoreCase ("Weight"))
      Weight (v.ToFloat ());

    else if  (n.EqualIgnoreCase ("CountFactor"))
      CountFactor (v.ToFloat ());

    else if  (n.EqualIgnoreCase ("SubClassifier"))
      SubClassifierName (v);
  }

  XmlTokenPtr  t = s.GetNextToken (cancelFlag, log);
  while  (t  &&  (!cancelFlag))
  {
    if  ((t->VarName ().EqualIgnoreCase ("Directory"))  &&  (typeid (*t) ==  typeid (XmlElementKKStr)))
    {
      XmlElementKKStrPtr xmlS = dynamic_cast<XmlElementKKStrPtr> (t);
      if  (xmlS  &&  xmlS->Value ())
      {
        directories.push_back (*(xmlS->Value ()));
      }
    }

    delete  t;
    t = s.GetNextToken (cancelFlag, log);
  }
  delete  t;
  t = NULL;
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

  KKStr  rootDirExpanded = osSubstituteInEnvironmentVariables (rootDir);

  TrainingClassList::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  ++idx)
  {
    TrainingClassPtr  tc = *idx;
    tc->WriteXML ("TrainingClass", rootDirExpanded, o);
    //XmlElementTrainingClass::WriteXML (*tc, KKStr::EmptyStr (), o);
  }

  XmlTag  tagEnd ("TrainingClassList", XmlTag::TagTypes::tagEnd);
  tagEnd.WriteXML (o);
  o << endl;
}



void  TrainingClassList::ReadXML (XmlStream&     s,
                                  XmlTagPtr      tag,
                                  VolConstBool&  cancelFlag,
                                  RunLog&        log
                                 )
{
  XmlTokenPtr t = s.GetNextToken (cancelFlag, log);
  while  (t  &&  (!cancelFlag))
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
              RootDir (*(s->Value ()));
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

    delete  t;
    t = s.GetNextToken (cancelFlag, log);
  }
  delete  t;
  t = NULL;
}  /* ReadXML */


  class  XmlFactoryTrainingClass: public XmlFactory
  {
  public:
    XmlFactoryTrainingClass (): XmlFactory ("TrainingClass") {}

    virtual  XmlElementTrainingClass*  ManufatureXmlElement (XmlTagPtr      tag,
                                                             XmlStream&     s,
                                                             VolConstBool&  cancelFlag,
                                                             RunLog&        log
                                                            )
    {
      return new XmlElementTrainingClass(tag, s, cancelFlag, log);
    }

    static   XmlFactoryTrainingClass*   factoryInstance;

    static   XmlFactoryTrainingClass*   FactoryInstance ()
    {
      if  (factoryInstance == NULL)
      {                            
        GlobalGoalKeeper::StartBlock ();
        if  (!factoryInstance)
        {
          factoryInstance = new XmlFactoryTrainingClass ();
          XmlFactory::RegisterFactory (factoryInstance);   
        }                                                  
        GlobalGoalKeeper::EndBlock ();                     
       }                                                   
      return  factoryInstance;                             
    }                                                      
  };
 

  XmlFactoryTrainingClass*   XmlFactoryTrainingClass::factoryInstance
                  = XmlFactoryTrainingClass::FactoryInstance ();


XmlFactoryMacro(TrainingClassList)



