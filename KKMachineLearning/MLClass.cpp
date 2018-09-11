#include "FirstIncludes.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include "MemoryDebug.h"
using namespace  std;

#include "GlobalGoalKeeper.h"
#include "KKBaseTypes.h"
#include "KKException.h"
#include "KKStrParser.h"
#include "OSservices.h"
#include "RunLog.h"
using namespace  KKB;

#include "FileDesc.h"
#include "MLClass.h"
using namespace  KKMLL;


MLClassListPtr  MLClass::existingMLClasses = NULL;
map<MLClassListPtr,MLClassListPtr>  MLClass::existingClassLists;

GoalKeeperPtr    MLClass::blocker = NULL;
bool             MLClass::needToRunFinalCleanUp = false;


// Will instantiate an instance of "GoalKeeper" if "blocker" does not already
// point one.
void  MLClass::CreateBlocker ()
{
  if  (!blocker)
  {
    GoalKeeper::Create ("GlobalMLClass", blocker);  // Will handle Race condition.
    blocker->StartBlock ();
    if  (!needToRunFinalCleanUp)
    {
      needToRunFinalCleanUp = true;
      atexit (MLClass::FinalCleanUp);
    }
    blocker->EndBlock ();
  }
}



MLClassListPtr  MLClass::GlobalClassList ()
{
  if  (!existingMLClasses)
  {
    CreateBlocker ();
    blocker->StartBlock ();
    if  (!existingMLClasses)
    {
      existingMLClasses = new MLClassList ();
    existingMLClasses->Owner (true);
    }
    blocker->EndBlock ();
  }
  return  existingMLClasses;
}  /* GlobalClassList */



/** @brief  Called by MLClassList constructors to add to list of existing MLClassList instances. */
void  MLClass::AddImageClassList (MLClassListPtr  list)
{
  CreateBlocker ();
  blocker->StartBlock ();
  existingClassLists.insert (pair<MLClassListPtr,MLClassListPtr> (list, list));
  blocker->EndBlock ();
}



/** @brief  Called by MLClassList destructor to remove from list of existing MLClassList instances. */
void  MLClass::DeleteImageClassList (MLClassListPtr  list)
{
  blocker->StartBlock ();

  map<MLClassListPtr,MLClassListPtr>::iterator idx;
  idx = existingClassLists.find (list);
  if  (idx != existingClassLists.end ())
  {
    existingClassLists.erase (idx);
  }
  else
  {
    cerr << endl << endl << "MLClass::DeleteImageClassList   MLClassList instance was not found." << endl << endl;
  }

  blocker->EndBlock ();
}



MLClassPtr  MLClass::CreateNewMLClass (const KKStr&  _name,
                                       kkint32       _classId
                                      )
{
  KKStr  upperName = _name.ToUpper ();

  MLClassListPtr  globalList = existingMLClasses;
  if  (!globalList)
    globalList = GlobalClassList ();

  MLClassPtr  mlClass = globalList->LookUpByName (_name);
  if  (mlClass == NULL)
  {
    if  (!blocker)
      CreateBlocker ();
    blocker->StartBlock ();

    mlClass = globalList->LookUpByName (_name);
    if  (mlClass == NULL)
    {
      MLClassPtr  temp = new MLClass (_name);
      temp->ClassId (_classId);
      existingMLClasses->AddMLClass (temp);
      mlClass = temp;
    }

    blocker->EndBlock ();
  }

  if  ((mlClass->ClassId () < 0)  &&  (_classId >= 0))
    ((MLClassPtr)mlClass)->ClassId (_classId);

  return  mlClass;
} /* CreateNewMLClass */



MLClassPtr  MLClass::GetByClassId (kkint32  _classId)
{
  return  GlobalClassList ()->LookUpByClassId (_classId);
}  /* GetByClassId */




MLClassListPtr  MLClass::BuildListOfDecendents (MLClassPtr  parent)
{
  if  (parent == NULL)
    return NULL;

  GlobalClassList ();  // Make sure that 'existingMLClasses'  exists.

  MLClassListPtr  results = new MLClassList ();
  results->PushOnBack (parent);

  MLClassPtr  startingAncestor = NULL;

  for  (auto existingMLClass: *existingMLClasses)
  {
    if  (!existingMLClass)
      continue;
    startingAncestor = existingMLClass;
    MLClassPtr ancestor = startingAncestor->Parent ();
    while  (ancestor != NULL)
    {
      if  (ancestor == parent)
      {
        results->PushOnBack (startingAncestor);
        break;
      }
      ancestor = ancestor->Parent ();
      if  (ancestor == startingAncestor)
        break;
    }
  }

  return  results;
}  /* BuildListOfDecendents */



void  MLClass::ChangeNameOfClass (MLClassPtr    mlClass, 
                                  const KKStr&  newName,
                                  bool&         successful
                                 )
{
  successful = true;
  KKStr  oldName = mlClass->Name();

  if  (!blocker)
    CreateBlocker ();
  blocker->StartBlock ();

  existingMLClasses->ChangeNameOfClass (mlClass, oldName, newName, successful);
  if  (!successful)
  {
    cerr << endl 
         << "MLClass::ChangeNameOfClass   'existingMLClasses'  failed to vahne name." << endl
         << "                        NewName[" << newName << "]" << endl
         << "                        OldName[" << mlClass->Name () << "]" << endl
         << endl;
    successful = false;
  }
  else
  {
    // It is okay to change the name;  we will also need to inform all 
    // instances of MLClassList that the name changed so they can 
    // update their nameIndex structures.
    mlClass->Name (newName);

    map<MLClassListPtr,MLClassListPtr>::iterator  idx;
    for  (idx = existingClassLists.begin ();  idx != existingClassLists.end ();  ++idx)
  {
      MLClassListPtr  list = idx->first;
      auto  classInList = list->PtrToIdx (mlClass);
      if  (classInList >= 0)
      {
        bool  nameChangedInList = false;
        list->ChangeNameOfClass (mlClass, oldName, newName, nameChangedInList);
      }
    }
  }

  blocker->EndBlock ();

  return;
}  /* ChangeNameOfClass */



void  MLClass::ResetAllParentsToAllClasses ()
{
  CreateBlocker ();
  blocker->StartBlock ();
  
  MLClassListPtr  globalList = GlobalClassList ();

  MLClassPtr  allClasses = globalList->LookUpByName ("AllClasses");

  MLClassList::iterator  idx;
  for  (idx = globalList->begin ();  idx != globalList->end ();  ++idx)
  {
    MLClassPtr  ic = *idx;
    if  (ic == allClasses)
      ic->Parent (NULL);
    else
      ic->Parent (allClasses);
  }
  blocker->EndBlock ();
}  /* ResetAllParentsToNull */



void  MLClass::Name (const KKStr&  _name)
{
  name = _name;
  upperName = name.ToUpper ();
}



MLClassPtr  MLClass::GetUnKnownClassStatic ()
{
  return  CreateNewMLClass ("UNKNOWN");
}  /* GetUnKnownClassStatic */



/** @brief  Call this at very end of program to clean up existingMLClasses. */
void  MLClass::FinalCleanUp ()
{
  if  (!needToRunFinalCleanUp)
    return;

  blocker->StartBlock ();
  if  (needToRunFinalCleanUp)
  {
    if  (!FileDesc::FinalCleanUpRanAlready ())
    {
      //cerr << endl << "MLClass::FinalCleanUp   ***ERROR***   Need to run MLClass::FinalCleanUp  before  FileDesc::FinalCleanUp" << endl << endl;
      FileDesc::FinalCleanUp ();
    }

    if  (existingMLClasses)
    {
      delete  existingMLClasses;
      existingMLClasses = NULL;
    }

    needToRunFinalCleanUp = false;
  }
  blocker->EndBlock ();

  GoalKeeper::Destroy (blocker);
  blocker = NULL;
}  /* FinalCleanUp */



MLClass::MLClass (const KKStr&  _name):
    classId          (-1),
    countFactor      (0.0f),
    description      (),
    mandatory        (false),
    name             (_name),
    parent           (NULL),
    storedOnDataBase (false),
    summarize        (false)

{
  if  (name.Empty ())
  {
    cerr << endl
         << "MLClass::MLClass   *** ERROR ***   Empty Name" << endl
         << endl;
  }

  upperName = name.ToUpper ();
  KKStr  topLevel = upperName;
  auto x = upperName.LocateCharacter ('_');
  if  (x)
    topLevel = upperName.SubStrPart (0, x.value () - 1);

  unDefined = upperName.Empty ()           ||  
             (upperName == "UNKNOWN")      ||  
             (upperName == "UNDEFINED")    ||  
             (upperName == "NONPLANKTON")  ||
             (topLevel  == "NOISE");
}



MLClass::MLClass (const MLClass&  mlClass)
{
  KKStr  errMsg (256);
  errMsg << endl
         << "MLClass::MLClass (const MLClass&  mlClass)       *** ERROR ***     classNane: " << mlClass.Name () << endl
         << "Should never ever call this method." << endl
         << endl;
  cerr << errMsg;
  throw KKException (errMsg);
}



MLClass::~MLClass ()
{
}



const KKStr&  MLClass::ParentName () const
{
  if  (parent)
    return  parent->Name ();
  return  KKStr::EmptyStr ();
}



KKStr  MLClass::ToString ()  const
{
  KKStr  str (name);
  return str;
}



KKStr  MLClass::GetClassNameFromDirName (const KKStr&  subDir)
{
  KKStr  className = osGetRootNameOfDirectory (subDir);
  auto x = className.LocateLastOccurrence ('_');
  if  (x  &&  (x.value () > 0))
  {
    // Now lets eliminate any sequence number in name
    // We are assuming that a underscore{"_") character separates the class name from the sequence number.
    // So if there is an underscore character,  and all the characters to the right of it are
    // numeric characters,  then we will remove the underscore and the following numbers.

    kkStrUint  y = x.value () + 1;

    bool  allFollowingCharsAreNumeric = true;
    while  ((y < className.Len ()) &&  (allFollowingCharsAreNumeric))
    {
      char  ch = className[y];
      allFollowingCharsAreNumeric = ((ch >= '0')  &&  (ch <= '9'));
      y++;
    }

    if  (allFollowingCharsAreNumeric)
    {
      className = className.SubStrPart (0, x.value () - 1);
    }
  }

  return  className;
}  /* GetClassNameFromDirName */



bool  MLClass::IsAnAncestor (MLClassPtr  c) const
{
  if  (c == this)
    return true;

  if  (parent == NULL)
    return false;

  return  parent->IsAnAncestor (c);
}



kkuint16  MLClass::NumHierarchialLevels ()  const
{
  return  (kkuint16)(name.InstancesOfChar ('_') + 1);
}



MLClassPtr   MLClass::MLClassForGivenHierarchialLevel (kkuint16 level)  const
{
  VectorKKStr  levelNames = name.Split ('_');
  KKStr fullLevelName = "";

  kkuint16 curLevel = 0;
  while  ((curLevel < level)  &&  (curLevel < levelNames.size ()))
  {
    if  (curLevel < 1)
      fullLevelName = levelNames[curLevel];
    else
      fullLevelName << "_" << levelNames[curLevel];

    curLevel++;
  }

  return  MLClass::CreateNewMLClass (fullLevelName);
}  /* MLClassForGivenHierarchialLevel*/



void  MLClass::WriteXML (const KKStr&    varName,
                         ostream&        o
                       )  const
{
  XmlTag startTag ("MLClass", XmlTag::TagTypes::tagEmpty);
  if  (!varName.Empty())
    startTag.AddAtribute ("VarName", varName);
  
  startTag.AddAtribute ("Name",  name);
  if  (parent != NULL)
    startTag.AddAtribute ("Parent", parent->Name ());

  if  (classId >= 0)
    startTag.AddAtribute ("ClassId", classId);

  if  (countFactor != 0.0f)
    startTag.AddAtribute ("CountFactor", countFactor);

  if  (description.Empty ())
    startTag.AddAtribute ("Description", description);

  startTag.AddAtribute ("Mandatory",        mandatory);
  startTag.AddAtribute ("StoredOnDataBase", storedOnDataBase);
  startTag.AddAtribute ("Summarize",        summarize);
  startTag.WriteXML (o);
  o << endl;
}  /* WriteXML */



XmlElementMLClass::XmlElementMLClass (XmlTagPtr      tag,
                                      XmlStream&     s,
                                      VolConstBool&  cancelFlag,
                                      RunLog&        log
                                     ):
  XmlElement (tag, s, log),
  value (NULL)
{
  KKStrConstPtr  className = tag->AttributeValueByName ("Name");
  if  (!className)
  {
    // We are missing a mandatory field.
    value = NULL;
    return;
  }

  value = MLClass::CreateNewMLClass (*className);

  kkint32  c = tag->AttributeCount ();
  for  (kkint32 x = 0;  x < c;  ++x)
  {
    KKStrConstPtr n = tag->AttributeNameByIndex (x);
    KKStrConstPtr v = tag->AttributeValueByIndex (x);
    if  (v != NULL)
    {
      if  ((n->EqualIgnoreCase ("Parent"))  &&  (!v->Empty ()))
        value->Parent (MLClass::CreateNewMLClass (*v));

      else if  (n->EqualIgnoreCase ("CountFactor"))
        value->CountFactor (v->ToFloat ());

      else if  (n->EqualIgnoreCase ("Description"))
        value->Description (*v);

      else if  (n->EqualIgnoreCase ("Mandatory"))
        value->Mandatory (v->ToBool ());

      else if  (n->EqualIgnoreCase ("StoredOnDataBase"))
        value->StoredOnDataBase (v->ToBool ());

      else if  (n->EqualIgnoreCase ("Summarize"))
        value->Summarize (v->ToBool ());
    }
  }

  XmlTokenPtr  t = s.GetNextToken (cancelFlag, log);
  while  (t  &&  (!cancelFlag))
    t = s.GetNextToken (cancelFlag, log);
}

        

XmlElementMLClass::~XmlElementMLClass ()
{
  value = NULL;
}



MLClassPtr  XmlElementMLClass::Value ()  const
{
  return value;
}



void  XmlElementMLClass::WriteXML (const MLClass&  mlClass,
                                   const KKStr&    varName,
                                   ostream&        o
                                  )
{
  mlClass.WriteXML (varName, o);
}


XmlFactoryMacro(MLClass)



MLClassList::MLClassList ():
     KKQueue<MLClass> (false),
     undefinedLoaded (false)
{
  MLClass::AddImageClassList (this);
}



MLClassList::MLClassList (const MLClassList&  _mlClasses):
  KKQueue<MLClass> (false)
{
  MLClass::AddImageClassList (this);

  kkuint32  numOfClasses = _mlClasses.QueueSize ();
  kkuint32  x;
  
  for  (x = 0; x < numOfClasses; x++)
  {
    AddMLClass (_mlClasses.IdxToPtr (x));  
  }
}



MLClassList::MLClassList (const KKStr&  _fileName,
                          bool&         _successfull
                         ):
     KKQueue<MLClass> (false),
     undefinedLoaded (false)
{
  MLClass::AddImageClassList (this);

  Load (_fileName, _successfull);

  if  (!undefinedLoaded)
  {
    // We have to make sure that there is a UnDefined MLClass.
    AddMLClass (MLClassList::GetMLClassPtr ("UNKNOWN"));
    undefinedLoaded = true;
  }
}


     
MLClassList::~MLClassList ()
{
  MLClass::DeleteImageClassList (this);
}



kkMemSize  MLClassList::MemoryConsumedEstimated ()  const
{
  return  sizeof (MLClassList) + sizeof (MLClassPtr) * size ();
}



void  MLClassList::Clear ()
{
  clear ();
  nameIndex.clear ();
}



/** @brief  Update the nameIndex structure about a class name change. */
void  MLClassList::ChangeNameOfClass (MLClassPtr    mlClass, 
                                      const KKStr&  oldName,
                                      const KKStr&  newName,
                                      bool&         successful
                                     )
{
  MLClassPtr  existingClass = this->LookUpByName (newName);
  if  ((existingClass != NULL)  &&  (existingClass != mlClass))
  {
    cerr << endl
      << "MLClassList::ChangeNameOfClass   NewName[" << newName << "]  alreadty used." << endl
      << endl;
    successful = false;
  }
  else
  {
    NameIndex::iterator idx = nameIndex.find (oldName.ToUpper ());
    if  (idx == nameIndex.end ())
    {
      cerr << endl
        << "MLClassList::ChangeNameOfClass   NewName[" << oldName << "]  not found." << endl
        << endl;
      successful = false;
    }
    else
    {
      nameIndex.erase (idx);
      nameIndex.insert (pair<KKStr,MLClassPtr> (newName.ToUpper (), mlClass));
      successful = true;
    }
  }
}  /* ChangeNameOfClass */



kkuint16 MLClassList::NumHierarchialLevels ()  const
{
  kkuint16 numHierarchialLevels = 0;
  MLClassList::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  idx++)
    numHierarchialLevels = Max (numHierarchialLevels, (*idx)->NumHierarchialLevels ());
  return  numHierarchialLevels;
}  /* NumHierarchialLevels*/



void  MLClassList::Load (const KKStr&  _fileName,
                         bool&         _successfull
                        )
{
  kkint32  lineCount = 0;

  FILE*  inputFile = osFOPEN (_fileName.Str (), "r");
  if  (!inputFile)
  {
    cerr << "MLClassList::ReadInData    *** ERROR ***" << endl
         << "                Input File[" << _fileName << "] not Valid." << endl;
    _successfull = false;
    return;
  }

  bool eof = false;
  KKStrPtr  dataRow = KKB::osReadRestOfLine (inputFile, eof);
  while  (!eof)
  {
    dataRow->TrimRight ();

    MLClassPtr  oneRow = MLClass::CreateNewMLClass (*dataRow);

    AddMLClass (oneRow);

    if  (oneRow->UnDefined ())
      undefinedLoaded = true;

    lineCount++;
    delete  dataRow;
    dataRow = KKB::osReadRestOfLine (inputFile, eof);
  }
  delete  dataRow;
  dataRow = NULL;

  fclose (inputFile);
  _successfull = true;
}  /* ReadInData */
    


void  MLClassList::Save (KKStr   _fileName,
                            bool&    _successfull
                           )
{
  ofstream outFile (_fileName.Str ());

  kkint32         idx;
  kkint32         qSize = QueueSize ();
  MLClassPtr   mlClass = NULL;

  for  (idx = 0; idx < qSize; idx++)
  {
    mlClass = IdxToPtr (idx);
    outFile << mlClass->ToString ().Str () << endl;
  }

  _successfull = true;
  return;
}  /* WriteOutData */



void  MLClassList::AddMLClassToNameIndex (MLClassPtr  _mlClass)
{
  NameIndex::iterator  idx;
  idx = nameIndex.find (_mlClass->UpperName ());
  if  (idx == nameIndex.end ())
    nameIndex.insert (pair<KKStr,MLClassPtr>(_mlClass->UpperName (), _mlClass));
}



void  MLClassList::AddMLClass (MLClassPtr  _mlClass)
{
  if  (_mlClass->Name ().Empty ())
  {
    cerr << "MLClassList::AddMLClass   Class Name Empty" << endl;
  }
  KKQueue<MLClass>::PushOnBack (_mlClass);
  AddMLClassToNameIndex (_mlClass);
}



MLClassPtr  MLClassList::PopFromBack ()
{
  NameIndex::iterator  idx;
  MLClassPtr ic = KKQueue<MLClass>::PopFromBack ();
  if  (!ic)
    return NULL;

  idx = nameIndex.find (ic->UpperName ());
  if  (idx != nameIndex.end ())
    nameIndex.erase (idx);
  return  ic;
}  /* PopFromBack*/



MLClassPtr  MLClassList::PopFromFront ()
{
  NameIndex::iterator  idx;
  MLClassPtr ic = KKQueue<MLClass>::PopFromFront ();
  if  (!ic)
    return NULL;

  idx = nameIndex.find (ic->UpperName ());
  if  (idx != nameIndex.end ())
    nameIndex.erase (idx);
  return  ic;
}  /* PopFromFront*/



void  MLClassList::PushOnBack (MLClassPtr  _mlClass)
  {
  if  (_mlClass->Name ().Empty ())
    {
    cerr << "MLClassList::PushOnBack   Class Name Empty" << endl;
    }

  KKQueue<MLClass>::PushOnBack (_mlClass);
  AddMLClassToNameIndex (_mlClass);
  }



void  MLClassList::PushOnFront (MLClassPtr  _mlClass)
{
  if  (_mlClass->Name ().Empty ())
  {
    cerr << "MLClassList::PushOnFront   Class Name Empty" << endl;
  }

  KKQueue<MLClass>::PushOnBack (_mlClass);
  AddMLClassToNameIndex (_mlClass);
}



/**                 
 *@brief  Returns a pointer of MLClass object with name (_name);  if none 
 *        in list will then return NULL.
 *@param[in]  _name  Name of MLClass to search for.
 *@return  Pointer to MLClass or NULL  if not Found.
 */
MLClassPtr  MLClassList::LookUpByName (const KKStr&  _name)  const
{
  NameIndex::const_iterator  idx;
  idx = nameIndex.find (_name.ToUpper ());
  if  (idx == nameIndex.end ())
    return NULL;
  else
    return idx->second;
} /* LookUpByName */



MLClassPtr  MLClassList::LookUpByClassId (kkint32 _classId)  const
{
  MLClassList::const_iterator  idx;
  MLClassPtr  mlClass;
  for  (idx = begin ();  idx != end ();  idx++)
  {
    mlClass = *idx;
    if  (mlClass->ClassId () == _classId)
      return  mlClass;
  }

  return  NULL;
}  /* LookUpClassId */



/** @brief  return pointer to instance with '_name';  if none exists, create one and add to list. */
MLClassPtr  MLClassList::GetMLClassPtr (const  KKStr& _name)
{
  MLClassPtr ic = LookUpByName (_name);
  if  (ic == NULL)
  {
    ic = MLClass::CreateNewMLClass (_name);
    AddMLClass (ic);
  }
  return  ic;
}  /* GetMLClassPtr */



MLClassPtr  MLClassList::GetNoiseClass ()  const
{
  MLClassPtr  noiseClass = NULL;
  for  (auto idx: *this)  
  {
    if  (idx->UnDefined ())
    {
     noiseClass = idx;
     break;
    }
  }
  return  noiseClass;
}  /* GetNoiseClass */



MLClassPtr  MLClassList::GetUnKnownClass ()
{
  MLClassPtr  unKnownClass = LookUpByName ("UNKNOWN");
  if  (!unKnownClass)
  {
    unKnownClass = MLClass::CreateNewMLClass ("UNKNOWN");
    PushOnBack (unKnownClass);
  }

  return  unKnownClass;
}  /* GetUnKnownClass */



class  MLClassList::MLClassNameComparison
{
public:
  MLClassNameComparison ()  {}

  bool  operator () (MLClassPtr  p1,
                     MLClassPtr  p2
                    )
  {
    return  (p1->UpperName () < p2->UpperName ());
  }
};  /* MLClassNameComparison */



void  MLClassList::SortByName ()
{
  MLClassNameComparison* c = new MLClassNameComparison ();
  sort (begin (), end (), *c);
  delete  c;
}  /* SortByName */



KKStr  MLClassList::ToString ()  const
{
  KKStr s (10 * QueueSize ());

  for (kkint32 i = 0;  i < QueueSize ();  i++)
  {
    if  (i > 0)
      s << "\t";

    s << IdxToPtr (i)->Name ();
  }

  return  s;
}  /* ToString */



KKStr  MLClassList::ToTabDelimitedStr ()  const
{
  KKStr s (10 * QueueSize ());
  for (kkint32 i = 0;  i < QueueSize ();  i++)
  {
    if  (i > 0)  s << "\t";
    s << IdxToPtr (i)->Name ();
  }

  return  s;
}  /* ToTabDelimitedStr */



KKStr  MLClassList::ToCommaDelimitedStr ()  const
{
  if  (this == NULL)
    return "NULL";

  KKStr s (10 * QueueSize ());
  for (kkint32 i = 0;  i < QueueSize ();  i++)
  {
    if  (i > 0)  s << ",";
    s << IdxToPtr (i)->Name ();
  }

  return  s;
}  /* ToCommaDelimitedStr */



KKStr  MLClassList::ToCommaDelimitedQuotedStr ()  const
{
  KKStr s (10 * QueueSize ());
  for (kkint32 i = 0;  i < QueueSize ();  i++)
  {
    if  (i > 0)  s << ",";
    s << IdxToPtr (i)->Name ().QuotedStr ();
  }

  return  s;
}  /* ToCommaDelimitedQuotedStr */



MLClassListPtr  MLClassList::ExtractSummarizeClasses ()  const
{
  MLClassListPtr  result = new MLClassList ();

  const_iterator  idx;
  for  (idx = begin();  idx != end ();  ++idx)
  {
    if  ((*idx)->Summarize ())
      result->PushOnBack (*idx);
  }

  return  result;
}  /* ExtractSummarizeClasses */



void  MLClassList::DeleteContents ()
{
  KKQueue<MLClass>::DeleteContents ();
  nameIndex.clear ();
  undefinedLoaded = false;
}



MLClassListPtr  MLClassList::ExtractMandatoryClasses ()  const
{
  MLClassListPtr  result = new MLClassList ();

  const_iterator  idx;
  for  (idx = begin();  idx != end ();  ++idx)
  {
    if  ((*idx)->Mandatory ())
      result->PushOnBack (*idx);
  }

  return  result;
}  /* ExtractMandatoryClasses */



void  MLClassList::ExtractTwoTitleLines (KKStr&  titleLine1,
                                            KKStr&  titleLine2 
                                           ) const
{
  titleLine1 = "";
  titleLine2 = "";

  kkint32 x;
  for  (x = 0;  x < QueueSize ();  x++)
  {
    if  (x > 0)
    {
      titleLine1 << "\t";
      titleLine2 << "\t";
    }

    KKStr  className = IdxToPtr (x)->Name ();
    auto  y = className.LocateCharacter ('_');
    if  (!y)
    {
      titleLine2 << className;
    }
    else
    {
      titleLine1 << className.SubStrPart (0, y.value () - 1);
      titleLine2 << className.SubStrPart (y.value () + 1);
    }
  }
}  /* ExtractTwoTitleLines */



void  MLClassList::ExtractThreeTitleLines (KKStr&  titleLine1,
                                           KKStr&  titleLine2, 
                                           KKStr&  titleLine3 
                                          ) const
{
  titleLine1 = "";
  titleLine2 = "";
  titleLine3 = "";

  kkint32 x;
  for  (x = 0;  x < QueueSize ();  x++)
  {
    if  (x > 0)
    {
      titleLine1 << "\t";
      titleLine2 << "\t";
      titleLine3 << "\t";
    }

    KKStr  part1, part2, part3;
    part1 = part2 = part3 = "";
    kkint32  numOfParts = 0;

    KKStr  className = IdxToPtr (x)->Name ();
    className.TrimLeft ();
    className.TrimRight ();
    
    numOfParts = 1;
    part1 = className.ExtractToken ("_");
    if  (!className.Empty ())
    {
      numOfParts = 2;
      part2 = className.ExtractToken ("_");

      if  (!className.Empty ())
      {
        numOfParts = 3;
        part3 = className;
      }
    }


    switch  (numOfParts)
    {
      case 1: titleLine3 << part1;
              break;

      case 2: titleLine2 << part1;
              titleLine3 << part2;
              break;

      case 3: titleLine1 << part1;
              titleLine2 << part2;
              titleLine3 << part3;
              break;
    }
  }
}  /* ExtractThreeTitleLines */



void  MLClassList::ExtractThreeTitleLines (KKStr&  titleLine1,
                                           KKStr&  titleLine2, 
                                           KKStr&  titleLine3,
                                           kkint32 fieldWidth
                                          ) const
{
  titleLine1 = "";
  titleLine2 = "";
  titleLine3 = "";

  KKStr blankField;
  blankField.RightPad (fieldWidth);

  kkint32 x;
  for  (x = 0;  x < QueueSize ();  x++)
  {
    KKStr  part1, part2, part3;
    part1 = part2 = part3 = "";
    kkint32  numOfParts = 0;

    KKStr  className = IdxToPtr (x)->Name ();
    className.TrimLeft ();
    className.TrimRight ();
    
    numOfParts = 1;
    part1 = className.ExtractToken ("_");
    if  (!className.Empty ())
    {
      numOfParts = 2;
      part2 = className.ExtractToken ("_");

      if  (!className.Empty ())
      {
        numOfParts = 3;
        part3 = className;
      }
    }

    part1.LeftPad (fieldWidth);
    part2.LeftPad (fieldWidth);
    part3.LeftPad (fieldWidth);

    switch  (numOfParts)
    {
      case 1: titleLine1 << blankField;
              titleLine2 << blankField;
              titleLine3 << part1;
              break;

      case 2: titleLine1 << blankField;
              titleLine2 << part1;
              titleLine3 << part2;
              break;

      case 3: titleLine1 << part1;
              titleLine2 << part2;
              titleLine3 << part3;
              break;
    }
  }
}  /* ExtractThreeTitleLines */



KKStr   MLClassList::ExtractHTMLTableHeader () const
{
  KKStr  header (QueueSize () * 50);

  kkuint16 x;

  MLClassList::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  idx++)
  {
    MLClassPtr  ic = *idx;

    VectorKKStr  parts = ic->Name ().Split ('_');

    header << "<th>";
    for  (x = 0;  x < parts.size ();  x++)
    {
      if  (x > 0)
        header << "<br />";
      header << parts[x];
    }
    header << "</th>";
  }

  return  header;
}  /* ExtractHTMLTableHeader */



MLClassListPtr  MLClassList::ExtractListOfClassesForAGivenHierarchialLevel (kkint16 level)
{
  MLClassListPtr  newList = new MLClassList ();

  MLClassList::iterator  idx;
  for  (idx = begin ();  idx != end ();  idx++)
  {
    MLClassPtr c = *idx;
    MLClassPtr classForLevel = c->MLClassForGivenHierarchialLevel (level);

    if  (newList->PtrToIdx (classForLevel) < 0)
      newList->AddMLClass (classForLevel);
  }

  newList->SortByName ();
  return  newList;
}  /* ExtractListOfClassesForAGivenHierarchialLevel */



MLClassListPtr  MLClassList::MergeClassList (const MLClassList&  list1,
                                             const MLClassList&  list2
                                            )
{
  MLClassListPtr  result = new MLClassList (list1);
  MLClassList::const_iterator idx;

  for  (idx = list2.begin ();  idx != list2.end ();  idx++)
  {
    MLClassPtr  ic = *idx;
    if  (result->PtrToIdx (ic) < 0)
    {
      // This entry (*idx) from list2 was not in list 1
      result->AddMLClass (ic);
    }
  }

  return  result;
}  /* MergeClassList */



MLClassListPtr  MLClassList::BuildListFromDelimtedStr (const KKStr&  s,
                                                       char          delimiter
                                                      )
{
  VectorKKStr  names = s.Split (delimiter);
  MLClassListPtr  classList = new MLClassList ();

  VectorKKStr::iterator  idx;
  for  (idx = names.begin ();  idx != names.end ();  idx++)
    classList->GetMLClassPtr (*idx);

  return  classList;
}  /* BuildListFromCommaDelimtedStr */



void  MLClassList::WriteXML (const KKStr&  varName,
                             ostream&      o
                            )  const
{
  XmlTag startTag ("MLClassList", XmlTag::TagTypes::tagStart);
  if  (!varName.Empty())
    startTag.AddAtribute ("VarName", varName);
  startTag.AddAtribute ("Count", (kkint32)size ());
  startTag.WriteXML (o);
  o << endl;

  MLClassList::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  idx++)
  {
    MLClassPtr  c = *idx;
    c->WriteXML ("", o);
  }

  XmlTag  endtag ("MLClassList", XmlTag::TagTypes::tagEnd);
  endtag.WriteXML (o);
  o << endl;
}  /* WriteXML */



bool  MLClassList::operator== (const MLClassList&  right)  const
{
  if  (QueueSize () != right.QueueSize ())
    return  false;

  for  (kkint32 i = 0;  i < QueueSize ();  i++)
  {
    MLClassPtr  mlClass = IdxToPtr (i);

    if  (right.LookUpByName (mlClass->Name ()) == NULL)
      return  false;
  }

  return  true;
}  /* operator== */




bool  MLClassList::operator!= (const MLClassList&  right)  const
{
  return  (!operator== (right));
}  /* operator== */



MLClassList&  MLClassList::operator-= (const MLClassList&  right) 
{
  if  (&right == this)
  {
    while  (QueueSize () > 0)
      PopFromBack ();
    return  *this;
  }


  MLClassList::const_iterator  idx;
  for  (idx = right.begin ();  idx != right.end ();  idx++)
  {
    MLClassPtr  ic = *idx;
    DeleteEntry (ic);  // if  'ic'  exists in our list it will be deleted.
  }

  return  *this;
}  /* operator=-  */



MLClassList&  MLClassList::operator+= (const MLClassList&  right)  // add all classes that are in the 'right' parameter
{
  if  (this == &right)
    return  *this;

  MLClassList::const_iterator  idx;
  for  (idx = right.begin ();  idx != right.end ();  idx++)
  {
    MLClassPtr  ic = *idx;
    if  (PtrToIdx (ic) < 0)
      PushOnBack (ic);
  }

  return  *this;
}



MLClassList&  MLClassList::operator= (const MLClassList&  right)
{
  if  (&right == this)
    return *this;

  Clear ();

  MLClassList::const_iterator  idx;
  for  (idx = right.begin ();  idx != right.end ();  idx++)
  {
    MLClassPtr  ic = *idx;
    PushOnBack (ic);
  }

  return  *this;
}  /* operator= */



MLClassList  MLClassList::operator- (const MLClassList&  right)  const
{
  MLClassList  result;

  MLClassList::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  idx++)
  {
    MLClassPtr  ic = *idx;
    if  (right.PtrToIdx (ic) < 0)
       result.PushOnBack (ic);
  }

  return  result;
}  /* operator- */



ostream&  KKMLL::operator<< (      ostream&          os, 
                             const MLClassList&   classList
                            )
{
  os << classList.ToString ();
  return  os;
}



KKStr&  KKMLL::operator<< (      KKStr&            str, 
                    const MLClassList&   classList
                   )
{
  str << classList.ToString ();
  return  str;
}




XmlElementMLClassNameList::XmlElementMLClassNameList (XmlTagPtr      tag,
                                                      XmlStream&     s,
                                                      VolConstBool&  cancelFlag,
                                                      RunLog&        log
                                                     ):
    XmlElement (tag, s, log),
    value (NULL)
{
  value = new MLClassList ();
  XmlTokenPtr t = s.GetNextToken (cancelFlag, log);
  while  (t  &&  (!cancelFlag))
  {
    if  (typeid (*t) == typeid(XmlContent))
    {
      XmlContentPtr  contentToken = dynamic_cast<XmlContentPtr> (t);
      KKStrConstPtr  contentStr = contentToken->Content ();
      if  (contentStr)
      {
        KKStrParser p (*contentStr);
        p.TrimWhiteSpace (" ");
        while  (p.MoreTokens ())
        {
          KKStr className = p.GetNextToken (",\t");
          if  (!className.Empty ())
            value->PushOnBack (MLClass::CreateNewMLClass (className));
        }
      }
    }

    delete  t;
    t = s.GetNextToken (cancelFlag, log);
  }
  delete  t;
  t = NULL;
}
 


XmlElementMLClassNameList::~XmlElementMLClassNameList ()
{
  delete  value;
  value = NULL;
}



MLClassListPtr  XmlElementMLClassNameList::Value ()  const
{
  return value;
}



MLClassListPtr  XmlElementMLClassNameList::TakeOwnership ()
{
  MLClassListPtr  v = value;
  value = NULL;
  return  v;
}


void  XmlElementMLClassNameList::WriteXML (const MLClassList&  mlClassList,
                                           const KKStr&        varName,
                                           ostream&            o
                                          )
{
  XmlTag  startTag ("MLClassNameList", XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);

  XmlContent::WriteXml (mlClassList.ToCommaDelimitedQuotedStr (), o);

  XmlTag  endTag ("MLClassNameList", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}
 

XmlFactoryMacro(MLClassNameList)


MLClassIndexList::MLClassIndexList ():
   largestIndex (-1),
   shortIdx ()
{
}



MLClassIndexList::MLClassIndexList (const  MLClassIndexList&  _list):
  std::map<MLClassPtr, kkint16>(),
  largestIndex (-1),
  shortIdx ()
{
  bool  dupEntry = false;
  for  (auto idx = _list.begin ();  idx != _list.end ();  idx++)
    AddClassIndexAssignment (idx->first, idx->second, dupEntry);
}  



MLClassIndexList::MLClassIndexList (const MLClassList&  classes):
    largestIndex (-1),
    shortIdx ()
{
  MLClassList::const_iterator  idx;
  for  (idx = classes.begin ();  idx != classes.end ();  idx++)
  {
    ++largestIndex;
    insert   (pair<MLClassPtr, kkint16> (*idx, largestIndex));
    shortIdx.insert (pair<kkint16, MLClassPtr> (largestIndex, *idx));
  }
}



void  MLClassIndexList::Clear ()
{
  clear ();
  shortIdx.clear ();
  largestIndex = 0;
}



kkMemSize  MLClassIndexList::MemoryConsumedEstimated ()  const
{
  return sizeof (MLClassIndexList) + (shortIdx.size () * (sizeof (kkint16) + sizeof (MLClassPtr) + 10));  // added 10- bytes per entry for overhead.
}



void  MLClassIndexList::AddClass (MLClassPtr  _ic,
                                  bool&       _dupEntry
                                 )
{
  _dupEntry = false;
  map<MLClassPtr, kkint16>::iterator p;
  p = find (_ic);
  if  (p != end ())
  {
    _dupEntry = true;
    return;
  }

  kkint16  index = (kkint16)(largestIndex + 1);
  largestIndex = index;

  insert (pair<MLClassPtr, kkint16> (_ic, index));
  shortIdx.insert (pair<kkint16, MLClassPtr> (index, _ic));
}  /* AddClass */



void  MLClassIndexList::AddClassIndexAssignment (MLClassPtr _ic,
                                                 kkint16    _classIndex,
                                                 bool&      _dupEntry
                                                )
{
  _dupEntry = false;
  map<MLClassPtr, kkint16>::iterator p;
  p = find (_ic);
  if  (p != end ())
  {
    _dupEntry = true;
    return;
  }

  insert (pair<MLClassPtr, kkint16> (_ic, _classIndex));
  shortIdx.insert (pair<kkint16, MLClassPtr> (_classIndex, _ic));

  if  (_classIndex > largestIndex)
    largestIndex = _classIndex;
}  /* AddClassIndexAssignment */



kkint16  MLClassIndexList::GetClassIndex (MLClassPtr  c)
{
  kkint16  index = -1;
  map<MLClassPtr, kkint16>::iterator p;
  p = find (c);
  if  (p == end ())
    index = -1;
  else
    index = p->second;

  return  index;
}  /* GetClassIndex */



MLClassPtr  MLClassIndexList::GetMLClass (kkint16  classIndex)
{
  map<kkint16, MLClassPtr>::iterator p;
  p = shortIdx.find (classIndex);
  if  (p == shortIdx.end ())
    return NULL;
  else
    return p->second;
}  /* GetMLClass */



MLClassPtr  MLClassIndexList::GetMLClass (kkint32 classIndex)
{
  if (classIndex > int16_max)
  {
    KKStr errMsg (256);
    errMsg << "MLClassIndexList::GetMLClass   classIndex[" << classIndex << "] exceeds max kjint16[" << int16_max << "] supported.";
    throw KKException (errMsg);
  }
  return GetMLClass ((kkint16)classIndex);
}


void  MLClassIndexList::ParseClassIndexList (const KKStr&  s,
                                             RunLog&       log
                                            )
{
  Clear ();
  largestIndex = -1;
  KKStrParser  parser (s);
  parser.TrimWhiteSpace (" ");
  while  (parser.MoreTokens ())
  {
    kkint16  classIndex = -1;
    KKStr className = parser.GetNextToken (",:");
    if  (parser.LastDelimiter () == ',')
    {
      // we should have encountered a ':' character but came across a ',' instead;  we are missing the Index Assignment
      classIndex = -1;
      log.Level (-1) << endl 
        << "MLClassIndexList::ParseClassIndexList   ***ERROR***   Missing index for Class: " << className << endl
        << endl;
    }
    else
    {
      classIndex = parser.GetNextToken (",").ToInt16 ();
    }

    bool  duplicate = false;
    AddClassIndexAssignment (MLClass::CreateNewMLClass (className), classIndex, duplicate);
    if  (duplicate)
    {
      log.Level (-1) << endl
        << "MLClassIndexList::ParseClassIndexList   ***ERROR***   Duplicate ClassIndex[" << classIndex << "] specified for class[" << className << "]" << endl
        << endl;
    }
  }
}  /* ParseClassIndexList */



KKStr  MLClassIndexList::ToCommaDelString ()  const
{
  KKStr  delStr (255);
  map<kkint16, MLClassPtr>::const_iterator  idx;
  for  (idx = shortIdx.begin ();  idx != shortIdx.end ();  idx++)
  {
    if  (!delStr.Empty ())
      delStr << ",";
    delStr << idx->second->Name ().QuotedStr () << ":" << idx->first;
  }
  return  delStr;
}  /* ToCommaDelString */



void  MLClassIndexList::ReadXML (XmlStream&      s,
                                 XmlTagConstPtr  tag,
                                 VolConstBool&   cancelFlag,
                                 RunLog&         log
                                )
{
  log.Level (30) << "MLClassIndexList::ReadXML  tag->Name: " << tag->Name () << endl;
  largestIndex = -1;
  shortIdx.clear ();

  XmlTokenPtr  t = s.GetNextToken (cancelFlag, log);
  while  (t  &&  (!cancelFlag))
  {
    if  (typeid (*t) == typeid (XmlContent))
    {
      XmlContentPtr c = dynamic_cast<XmlContentPtr> (t);
      if  (c  &&  c->Content ())
      {
        KKStrConstPtr  text = c->Content ();
        ParseClassIndexList (*text, log);
      }
    }

    delete  t;
    t = s.GetNextToken (cancelFlag, log);
  }
  delete  t;
  t = NULL;
}  /* ReadXML */



void  MLClassIndexList::WriteXML (const KKStr&  varName,
                                  ostream&      o
                                 )  const
{
  XmlTag  startTag ("MLClassIndexList", XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);

  XmlContent::WriteXml (ToCommaDelString (), o);

  XmlTag  endTag ("MLClassIndexList", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}


XmlFactoryMacro(MLClassIndexList)
