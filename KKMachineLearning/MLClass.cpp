#include "FirstIncludes.h"

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>

#include "MemoryDebug.h"

using namespace  std;


#include "KKBaseTypes.h"
#include "OSservices.h"
#include "RunLog.h"
using namespace  KKB;

#include "FileDesc.h"
#include "MLClass.h"
using namespace  KKMachineLearning;



MLClassListIndexPtr  MLClass::existingMLClasses = NULL;

GoalKeeperPtr    MLClass::blocker = NULL;
bool             MLClass::needToRunFinaleCleanUp = false;




// Will instantiate an instance of "GoalKeeper" if "blocker" does not already
// point one.
void  MLClass::CreateBlocker ()
{
  if  (!blocker)
  {
    GoalKeeper::Create ("GlobalMLClass", blocker);  // Will handle Race condition.
    blocker->StartBlock ();
    if  (!needToRunFinaleCleanUp)
    {
      needToRunFinaleCleanUp = true;
      atexit (MLClass::FinalCleanUp);
    }
    blocker->EndBlock ();
  }
}





MLClassListIndexPtr  MLClass::GlobalClassList ()
{
  if  (!existingMLClasses)
  {
    CreateBlocker ();
    blocker->StartBlock ();
    existingMLClasses = new MLClassListIndex ();
    existingMLClasses->Owner (true);
    blocker->EndBlock ();
  }
  return  existingMLClasses;
}



MLClassPtr  MLClass::CreateNewMLClass (const KKStr&  _name,
                                       kkint32       _classId
                                      )
{
  KKStr  upperName = _name.ToUpper ();

  MLClassListIndexPtr  globalList = existingMLClasses;
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
      mlClass = new MLClass (_name);
      mlClass->ClassId (_classId);
      existingMLClasses->AddMLClass (mlClass);
    }

    blocker->EndBlock ();
  }

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

  MLClassPtr  ancestor = NULL;
  MLClassPtr  startingAncestor = NULL;

  MLClassList::const_iterator  idx;

  for  (idx = existingMLClasses->begin ();  idx != existingMLClasses->end ();  idx++)
  {
    MLClassPtr ancestor = *idx;
    startingAncestor = *idx;
    ancestor = startingAncestor->Parent ();
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





void  MLClass::ChangeNameOfClass (MLClassPtr  mlClass, 
                                  const KKStr&   newName,
                                  bool&          changeSuccessful
                                 )
{
  changeSuccessful = false;
  if  (!existingMLClasses)
    return;

  KKStr  newNameUpper = newName.ToUpper ();
  if  (newNameUpper == mlClass->UpperName ())
  {
    // Name did not really change.
    changeSuccessful = true;
    mlClass->Name (newName);
    return;
  }

  existingMLClasses->ChangeNameOfClass (mlClass, mlClass->Name (), newName, changeSuccessful);
  if  (!changeSuccessful)
  {
    cerr << endl << endl 
         << " MLClass::ChangeNameOfClass  ***ERROR***   Could not change the name of the class,  possibly new name already in use." << endl
         << endl;
    return;
  }

  mlClass->Name (newName);
  changeSuccessful = true;
  return;
}  /* ChangeNameOfClass */




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
  if  (!needToRunFinaleCleanUp)
    return;

  blocker->StartBlock ();
  if  (needToRunFinaleCleanUp)
  {
    if  (!FileDesc::FinaleCleanUpRanAlready ())
    {
      //cerr << endl << "MLClass::FinalCleanUp   ***ERROR***   Need to run MLClass::FinaleCleanUp  before  FileDesc::FinaleCleanUp" << endl << endl;
      FileDesc::FinalCleanUp ();
    }

    if  (existingMLClasses)
    {
      delete  existingMLClasses;
      existingMLClasses = NULL;
    }

    needToRunFinaleCleanUp = false;
  }
  blocker->EndBlock ();

  GoalKeeper::Destroy (blocker);
  blocker = NULL;
}  /* FinalCleanUp */





MLClass::MLClass (const KKStr&  _name):
    classId          (-1),
    countFactor      (0.0f),
    description      (),
    name             (_name),
    parent           (NULL),
    storedOnDataBase (false)

{
  if  (name.Empty ())
  {
    cerr << endl
         << "MLClass::MLClass   *** ERROR ***   Empty Name" << endl
         << endl;
  }

  upperName = name;
  upperName.Upper ();
  unDefined = upperName.Empty ()           ||  
             (upperName == "UNKNOWN")      ||  
             (upperName == "UNDEFINED")    ||  
             (upperName == "NOISE")        ||
             (upperName == "NOISY")        ||
             (upperName == "NONPLANKTON");
}





MLClass::MLClass (const MLClass&  mlClass)
{
  cerr << endl
       << "MLClass::MLClass (const MLClass&  mlClass)            *** ERROR ***" << endl
       << "Should never ever call this method." << endl
       << endl;
  osWaitForEnter ();
  exit (-1);
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




void  MLClass::ProcessRawData (KKStr&  _data)
{
  name = _data.ExtractToken (" ,\n\t\r");
  name.TrimRight ();
  name.TrimLeft ();

  upperName = name;
  upperName.Upper ();

  unDefined = upperName.Empty ()           ||  
             (upperName == "UNKNOWN")      ||  
             (upperName == "UNDEFINED")    ||  
             (upperName == "NOISE")        ||
             (upperName == "NOISY")        ||
             (upperName == "NONPLANKTTON");

} /* ProcessRawData */



KKStr  MLClass::GetClassNameFromDirName (const KKStr&  subDir)
{
  KKStr  className = osGetRootNameOfDirectory (subDir);
  kkint32 x = className.LocateLastOccurrence ('_');
  if  (x > 0)
  {
    // Now lets eliminate any seqence number in name
    // We are assuming that a underscore{"_") character seperates the class name from the seq number.
    // So if there is an underscore character,  and all the characters to the right of it are
    // numeric charcters,  then we will remove the underscore and the following numbers.

    kkint32  y = x + 1;

    bool  allFollowingCharsAreNumeric = true;
    while  ((y < className.Len ()) &&  (allFollowingCharsAreNumeric))
    {
      char  ch = className[y];
      allFollowingCharsAreNumeric = ((ch >= '0')  &&  (ch <= '9'));
      y++;
    }

    if  (allFollowingCharsAreNumeric)
    {
      className = className.SubStrPart (0, x - 1);
    }
  }

  return  className;
}  /* GetClassNameFromDirName */




bool  MLClass::IsAnAncestor (MLClassPtr  c)   // will return true if 'c' is an ancestor
{
  if  (c == this)
    return true;

  if  (parent == NULL)
    return false;

  return  parent->IsAnAncestor (c);
}




kkuint32 MLClass::NumHierarchialLevels ()  const
{
  return  (kkuint32)name.InstancesOfChar ('_') + 1;
}




void  MLClass::WriteXML (ostream& o)  const
{
  o << "<MLClass "
    <<   "Name="             << name.QuotedStr ()                             << ","
    <<   "ClassId="          << classId                                       << ","
    <<   "UnDefined="        << (unDefined ? "Y":"N")                         << ","
    <<   "Paremt="           << ((parent == NULL) ? "\"\"" : parent->Name ()) << ","
    <<   "StoredOnDataBase=" << (storedOnDataBase ? "Y" : "N")                << ","
    <<   "Description="      << description.QuotedStr ()                      << ","
    <<   "CountFactor="      << countFactor
    << " />"
    << endl;
}  /* WriteXML */





MLClassPtr   MLClass::MLClassForGivenHierarchialLevel (kkuint32 level)  const
{
  if  (level < 0)
    level = 0;

  VectorKKStr  levelNames = name.Split ('_');
  KKStr fullLevelName = "";

  kkuint32  curLevel = 0;
  while  ((curLevel <= level)  &&  (curLevel < levelNames.size ()))
  {
    if  (curLevel < 1)
      fullLevelName = levelNames[curLevel];
    else
      fullLevelName << "_" << levelNames[curLevel];

    curLevel++;
  }

  return  MLClass::CreateNewMLClass (fullLevelName);
}  /* MLClassForGivenHierarchialLevel*/




MLClassList::MLClassList ():
     KKQueue<MLClass> (false),
     undefinedLoaded (false)
{
}




MLClassList::MLClassList (const MLClassList&  _mlClasses):
  KKQueue<MLClass> (false)
{
  kkuint32  numOfClasses = _mlClasses.QueueSize ();
  kkuint32  x;
  
  for  (x = 0; x < numOfClasses; x++)
  {
    AddMLClass (_mlClasses.IdxToPtr (x));  
  }
}





MLClassList::MLClassList (KKStr   _fileName,
                                bool&    _successfull
                               ):
     KKQueue<MLClass> (false),
     undefinedLoaded (false)
{
  Load (_fileName, _successfull);

  if  (!undefinedLoaded)
  {
    // We have to make sure that there is a UnDefined MLClass.
    AddMLClass (new MLClass (KKStr ("UNKNOWN")));
    undefinedLoaded = true;
  }
}



     
MLClassList::~MLClassList ()
{
}


kkint32  MLClassList::MemoryConsumedEstimated ()  const
{
  return  sizeof (MLClassList) + sizeof (MLClassPtr) * size ();
}



kkuint32 MLClassList::NumHierarchialLevels ()  const
{
  kkuint32  numHierarchialLevels = 0;
  MLClassList::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  idx++)
    numHierarchialLevels = Max (numHierarchialLevels, (*idx)->NumHierarchialLevels ());
  return  numHierarchialLevels;
}  /* NumHierarchialLevels*/





void  MLClassList::Load (KKStr  _fileName,
                            bool&  _successfull
                           )
{
  char   buff[20480];
  kkint32  lineCount = 0;

  FILE*  inputFile = osFOPEN (_fileName.Str (), "r");
  if  (!inputFile)
  {
    cerr << "MLClassList::ReadInData    *** ERROR ***" << endl
         << "                Input File[" << _fileName << "] not Valid." << endl;
    _successfull = false;
    return;
  }

  while  (fgets (buff, sizeof (buff), inputFile))
  {
    KKStr  dataRow (buff);

    dataRow.TrimRight ();

    MLClassPtr  oneRow = new MLClass (dataRow);

    AddMLClass (oneRow);

    if  (oneRow->UnDefined ())
      undefinedLoaded = true;

    lineCount++;
  }

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




void  MLClassList::AddMLClass (MLClassPtr  _mlClass)
{
  if  (_mlClass->Name ().Empty ())
  {
    cerr << "MLClassList::AddMLClass   Class Name Empty" << endl;
  }
  PushOnBack (_mlClass);
}



MLClassPtr  MLClassList::LookUpByName (const KKStr&  _name)  const
{
  kkint32      idx;
  kkint32      qSize = QueueSize ();
  MLClassPtr   mlClass = NULL;
  MLClassPtr   tempMLClass = NULL;

  for  (idx = 0;  (idx < qSize);  idx++)
  {
    tempMLClass = IdxToPtr (idx);
    if  (_name.EqualIgnoreCase (tempMLClass->Name ()))
    {
      mlClass = tempMLClass;
      break;
    }
  }

  return  mlClass;
} /* LookUpByName */




MLClassPtr  MLClassList::LookUpByClassId (kkint32  _classId)
{
  MLClassList::iterator  idx;
  MLClassPtr  mlClass;
  for  (idx = begin ();  idx != end ();  idx++)
  {
    mlClass = *idx;
    if  (mlClass->ClassId () == _classId)
      return  mlClass;
  }

  return  NULL;
}  /* LookUpClassId */




MLClassPtr  MLClassList::GetMLClassPtr (const  KKStr& _name)
{
  MLClassPtr mlClass = LookUpByName (_name);
  if  (!mlClass)
  {
    mlClass = MLClass::CreateNewMLClass (_name);
    AddMLClass (mlClass);
  }

  return  mlClass;
}  /* GetMLClassPtr */




MLClassPtr  MLClassList::GetNoiseClass ()  const
{
  kkint32     count      = QueueSize ();
  MLClassPtr  noiseClass = NULL;
  kkint32     x;

  for  (x = 0; ((x < count)  &&  (!noiseClass)); x++)
  {
    MLClassPtr  mlClass = IdxToPtr (x);
    if  (mlClass->UnDefined ())
       noiseClass = mlClass;
  }

  return  noiseClass;
}  /* GetNoiseClass */





MLClassPtr  MLClassList::GetUnKnownClass ()
{
  MLClassPtr  unKnownClass = LookUpByName (KKStr ("UNKNOWN"));
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
  KKStr s (10 * QueueSize ());
  for (kkint32 i = 0;  i < QueueSize ();  i++)
  {
    if  (i > 0)  s << ",";
    s << IdxToPtr (i)->Name ();
  }

  return  s;
}  /* ToCommaDelimitedStr */






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
    kkint32  y = className.LocateCharacter ('_');
    if  (y < 0)
    {
      titleLine2 << className;
    }
    else
    {
      titleLine1 << className.SubStrPart (0, y - 1);
      titleLine2 << className.SubStrPart (y + 1);
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

  kkuint32 x;

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



MLClassListPtr  MLClassList::ExtractListOfClassesForAGivenHierarchialLevel (kkint32 level)
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
  VectorKKStr  names = s.Split (',');
  MLClassListPtr  classList = new MLClassList ();

  VectorKKStr::iterator  idx;
  for  (idx = names.begin ();  idx != names.end ();  idx++)
    MLClassPtr c = classList->GetMLClassPtr (*idx);

  return  classList;
}  /* BuildListFromCommaDelimtedStr */







void   MLClassList::WriteXML (std::ostream&  o)  const
{
  o << "<MLClassList  NumOfClasses=" << QueueSize () << " >" << endl;

  MLClassList::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  idx++)
  {
    const MLClassPtr  ic = *idx;
    ic->WriteXML (o);
  }

  o << "</MLClassList>" << endl;
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
    const MLClassPtr  ic = *idx;
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

  while  (QueueSize () > 0)
    PopFromBack ();

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



ostream&  operator<< (      ostream&          os, 
                      const MLClassList&   classList
                     )
{
  os << classList.ToString ();
  return  os;
}


KKStr&  operator<< (      KKStr&           str, 
                    const MLClassList&   classList
                   )
{
  str << classList.ToString ();
  return  str;
}





ClassIndexList::ClassIndexList ():
   largestIndex (-1)
{
}


ClassIndexList::ClassIndexList (const  ClassIndexList&  _list)
{
  const_iterator  idx;
  bool  dupEntry = false;
  for  (idx = _list.begin ();  idx != _list.end ();  idx++)
  {
    AddClassIndexAssignment (idx->first, idx->second, dupEntry);
  }
}  



ClassIndexList::ClassIndexList (const MLClassList&  classes):
    largestIndex(-1)
{
  MLClassList::const_iterator  idx;
  for  (idx = classes.begin ();  idx != classes.end ();  idx++)
  {
    largestIndex++;
    insert   (pair<MLClassPtr, kkint16> (*idx, largestIndex));
    shortIdx.insert (pair<kkint16, MLClassPtr> (largestIndex, *idx));
  }
}


kkint32  ClassIndexList::MemoryConsumedEstimated ()  const
{
  return sizeof (ClassIndexList) + (shortIdx.size () * (sizeof (kkint16) + sizeof (MLClassPtr) + 10));  // added 10- bytes per entry for overhead.
}



void  ClassIndexList::AddClass (MLClassPtr  _ic,
                                bool&          _dupEntry
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

  kkint32  index = largestIndex + 1;
  largestIndex = index;

  insert (pair<MLClassPtr, kkint16> (_ic, index));
  shortIdx.insert (pair<kkint16, MLClassPtr> (index, _ic));
}  /* AddClass */



void  ClassIndexList::AddClassIndexAssignment (MLClassPtr _ic,
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



kkint16  ClassIndexList::GetClassIndex (MLClassPtr  c)
{
  kkint32  index = -1;
  map<MLClassPtr, kkint16>::iterator p;
  p = find (c);
  if  (p == end ())
  {
    largestIndex++;
    insert (pair<MLClassPtr, kkint16> (c, largestIndex));
    shortIdx.insert (pair<kkint16, MLClassPtr> (largestIndex, c));
    index = largestIndex;
  }
  else
  {
    index = p->second;
  } 

  return  index;
}  /* GetClassIndex */



MLClassPtr  ClassIndexList::GetMLClass (kkint16  classIndex)
{
  map<kkint16, MLClassPtr>::iterator p;
  p = shortIdx.find (classIndex);
  if  (p == shortIdx.end ())
    return NULL;
  else
    return p->second;
}  /* GetMLClass */



void  ClassIndexList::ParseClassIndexList (const KKStr&  s)
{
  clear ();
  shortIdx.clear ();
  largestIndex = 0;

  bool   duplicate = false;
  kkint32  index  = 0;
  KKStr  name   = "";

  VectorKKStr  pairs = s.Split (",");
  for  (kkuint32 x = 0;  x < pairs.size ();  x++)
  {
    KKStr  pair = pairs[x];
    name  = pair.ExtractToken2 (":");
    index = pair.ExtractTokenInt ("\n\t\r");
    AddClassIndexAssignment (MLClass::CreateNewMLClass (name), index, duplicate);
  }
}  /* ParseClassIndexList */



KKStr  ClassIndexList::ToCommaDelString ()
{
  KKStr  delStr (255);
  map<kkint16, MLClassPtr>::const_iterator  idx;
  for  (idx = shortIdx.begin ();  idx != shortIdx.end ();  idx++)
  {
    if  (!delStr.Empty ())
      delStr << ",";
    delStr << idx->second->Name () << ":" << idx->first;
  }
  return  delStr;
}  /* ToCommaDelString */





MLClassListIndex::MLClassListIndex ():
  MLClassList ()
{
}


/**
 *@brief  Copy constructor; will make a duplicate list of mlClass pointers 
 *        but will not be owner of them.
 */
MLClassListIndex::MLClassListIndex (const MLClassListIndex&  _mlClasses):
  MLClassList (_mlClasses)
{
  MLClassListIndex::const_iterator  idx;
  for  (idx = _mlClasses.begin ();  idx != _mlClasses.end ();  ++idx)
  {
    MLClassPtr  ic = *idx;
    nameIndex.insert (pair<KKStr,MLClassPtr> (ic->UpperName (), ic));
  }
}
  

/**
 *@brief  Conversion constructor;  will convert a MLClassList instance to a 'MLClassListIndex' instance.
 */
MLClassListIndex::MLClassListIndex (const MLClassList&  _mlClasses):
  MLClassList (_mlClasses)
{
  MLClassListIndex::const_iterator  idx;
  for  (idx = _mlClasses.begin ();  idx != _mlClasses.end ();  ++idx)
  {
    MLClassPtr  ic = *idx;
    nameIndex.insert (pair<KKStr,MLClassPtr> (ic->UpperName (), ic));
  }
}
  


MLClassListIndex::~MLClassListIndex ()
{
}



void   MLClassListIndex::AddMLClass (MLClassPtr  _mlClass)
{
  MLClassList::AddMLClass  (_mlClass);

  NameIndex::iterator  idx;
  idx = nameIndex.find (_mlClass->UpperName ());
  if  (idx == nameIndex.end ())
    nameIndex.insert (pair<KKStr,MLClassPtr>(_mlClass->UpperName (), _mlClass));
}  /* AddMLClass */





MLClassPtr  MLClassListIndex::LookUpByName (const KKStr&  _name)  const
{
  NameIndex::const_iterator  idx;
  idx = nameIndex.find (_name.ToUpper ());
  if  (idx == nameIndex.end ())
    return NULL;
  else
    return idx->second;
}  /* LookUpByName */



MLClassPtr  MLClassListIndex::GetMLClassPtr (const KKStr& _name)
{
  MLClassPtr ic = LookUpByName (_name);
  if  (ic == NULL)
  {
    ic = MLClass::CreateNewMLClass (_name);
    AddMLClass (ic);
  }
  return  ic;
}  /* GetMLClassPtr */



void  MLClassListIndex::ChangeNameOfClass (MLClassPtr  mlClass, 
                                              const KKStr&   oldName,
                                              const KKStr&   newName,
                                              bool&          successful
                                             )
{
  successful = true;
  if  (newName.EqualIgnoreCase (oldName))
  {
    // Nothing to do;  name has not really changed.
    return;
  }

  MLClassPtr  exitingClass = LookUpByName (newName);
  if  (exitingClass)
  {
    successful = false;
    cerr << endl << endl 
         << "MLClassListIndex::ChangeNameOfClass  ***ERROR***   newName[" << newName << "]  already in use." << endl
         << endl;
    return;
  }

  NameIndex::iterator  idx = nameIndex.find (oldName.ToUpper ());
  if  (idx == nameIndex.end ())
  {
    successful = false;
    cerr << endl << endl 
         << "MLClassListIndex::ChangeNameOfClass  ***ERROR***   oldName[" << oldName << "]  was not in the index." << endl
         << endl;
    return;
  }
  
  nameIndex.erase (idx);

  nameIndex.insert (pair<KKStr,MLClassPtr> (newName.ToUpper (), mlClass));
  return;
}  /* ChangeNameOfClass */
