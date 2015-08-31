#include "FirstIncludes.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include "MemoryDebug.h"
using namespace  std;


#include "KKBaseTypes.h"
#include "OSservices.h"
using namespace  KKB;


#include "ClassAssignments.h"
#include "MLClass.h"
using namespace  KKMLL;





ClassAssignments::ClassAssignments ():
    multimap<kkint16, MLClassPtr> ()
{
}



ClassAssignments::ClassAssignments (const MLClassList&  classes):
    multimap<kkint16, MLClassPtr> ()
{
  kkint32  x = 0;
  for  (MLClassList::const_iterator idx = classes.begin ();  idx != classes.end ();  idx++)
  {
   insert (pair<kkint16, MLClassPtr> (x, *idx));
   classLookUp.insert (pair<MLClassPtr, kkint16> (*idx, x));
   x++;
  }
}



kkint32  ClassAssignments::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = sizeof (ClassAssignments) 
    +  (classLookUp.size () * (sizeof (MLClassPtr) + sizeof (kkint16)));
  return  memoryConsumedEstimated;
}




void  ClassAssignments::AddMLClass (MLClassPtr  mlClass,
                                    kkint16     num,
                                    RunLog&     log
                                   )
{
  ClassLookUpIterator  idx;
  idx = classLookUp.find (mlClass);
  if  (idx != classLookUp.end ())
  {
    log.Level (-1) << endl << endl 
                   << "ClassAssignments::AddMLClass     ***ERROR***       Duplicate Class Being Added[" << mlClass->Name () <<  "]." << endl
                   << "              Num Found           [" << idx->second << "]" << endl
                   << "              New Num Being Added [" << num         << endl
                   << endl;
    return;
  }

  insert (pair<kkint16, MLClassPtr> (num, mlClass));
  classLookUp.insert (pair<MLClassPtr, kkint16> (mlClass, num));
}  /* AddMLClass */





MLClassPtr  ClassAssignments::GetMLClass (kkint16 num)  const
{
  multimap<kkint16, MLClassPtr>::const_iterator p;
  p = find (num);
  if  (p == end ())
    return NULL;
  return p->second;
}  /* GetMLClass */





MLClassList  ClassAssignments::GetMLClasses (kkint16 num)  const
{
  ClassAssignments::const_iterator  idx;

  MLClassList  results;
  for  (idx = begin ();  idx != end ();  idx++)
  {
    if  (idx->first == num)
      results.PushOnBack (idx->second);
  }

  return  results;
}  /* GetMLClasses */




VectorShort   ClassAssignments::GetUniqueListOfAssignments ()  const
{
  VectorShort  nums;

  ClassAssignments::const_iterator  idx;

  for  (idx = begin ();  idx != end ();  idx++)
  {
    nums.push_back (idx->first);
  }

  sort (nums.begin (), nums.end ());

  VectorShort  results;

  kkint16  lastNum = -999;
  VectorShort::const_iterator  idx2;
  for  (idx2 = nums.begin (); idx2 != nums.end ();  idx2++)
  {
    if  (*idx2 != lastNum)
    {
      lastNum = *idx2;
      results.push_back (lastNum);
    }
  }

  return  results;
}  /* GetUniqueListOfAssignments */





MLClassPtr  ClassAssignments::GetMLClassByIndex (size_t idx)
{
  if  ((idx < 0)  ||  (idx >= (size_t)size ()))
  {
    cerr << endl
         << endl
         << "ClassAssignments::GetMLClassByIndex   *** ERROR ***" << endl
         << "                                        idx[" << idx << "] is out of range." << endl
         << endl;
    return NULL;
  }

  iterator i = begin ();
  for  (size_t x = 0; x < idx;  x++)
    i++;

  return i->second;
}  /* GetMLClassByIndex */





kkint16  ClassAssignments::GetNumForClass (MLClassPtr  mlClass)  const
{
  ClassLookUp::const_iterator  idx;
  idx = classLookUp.find (mlClass);
  if  (idx == classLookUp.end ())
    return -1;
  else
    return idx->second;
}  /* GetNumForClass */






void  ClassAssignments::Load (const KKStr&  fileName,
                              bool&         successful,
                              RunLog&       log
                             )
{
  log.Level (10) << "ClassAssignments::Load -  File[" << fileName << "]." << endl;

  char   buff[20480];

  FILE*  inputFile = osFOPEN (fileName.Str (), "r");
  if  (!inputFile)
  {
    log.Level (-1) << endl
                   << "ClassAssignments::Load    *** ERROR ***" << endl
                   << "                Input File[" << fileName << "] not Valid." << endl
                   << endl;
    successful = false;
    return;
  }

  while  (fgets (buff, sizeof (buff), inputFile))
  {
    log.Level (70) << "ReadInData - [" << buff << "]" << endl;

    KKStr  dataRow (buff);
    dataRow.TrimRight ();
    dataRow.TrimLeft ();

    KKStr  className = dataRow.ExtractToken2 ("\n\r\t,");
    kkint16  classNum  = (kkint16)dataRow.ExtractTokenInt ("\n\r\t,");

    MLClassPtr mlClass = MLClass::CreateNewMLClass (className);
    kkint32  existingAssignmentNum = GetNumForClass (mlClass);
    if  (existingAssignmentNum >= 0)
    {
      // We already have this class Loaded,  for now i am only going to
      // display a warning.
      log.Level (-1) << endl
                     << endl
                     << endl
                     << "ClassAssignments::Load    *** ERROR ***" << endl
                     << "MLClass[" << className << "] being loaded more than once." << endl
                     << endl;
    }
    else
    {
      AddMLClass (mlClass, classNum, log);
    }
  }

  fclose (inputFile);
  successful = true;
}  /* Load */



void  ClassAssignments::Save (const KKStr&  fileName,
                              bool&          successful
                             )
{
  ofstream outFile (fileName.Str ());

  for  (iterator  idx = begin (); idx != end ();  idx++)
  {
    outFile << idx->second->Name () << "\t" << idx->first << endl;
  }

  successful = true;
  return;
}




KKStr  ClassAssignments::ToString ()  const
{
  KKStr  result ((kkint32)(size () * 20));

  result << kkint32 (size ());

  ClassAssignments::const_iterator idx;
  for (idx = begin ();  idx != end ();  idx++)
  {
    result << "\t" << idx->first << "\t" << idx->second->Name ();
  }

  return  result;
}  /* ToString */





/**
* @brief Loads class assignments from string that was originally generated by ToString
* @param[in] _toString,  KKStr containing class assignments info, will expect to be formated
*                        the way ToString() creates them.
*/
void   ClassAssignments::ParseToString (const KKStr&  _toString,
                                        RunLog&       _log
                                       )
{
  erase (begin (), end ());

  KKStr toString (_toString);

  // Remove Class count from string
  toString.ExtractTokenInt ("\t\n\r");

  while  (!toString.Empty ())
  {
    kkint32 assignmentNum = toString.ExtractTokenInt ("\t\n\r");
    KKStr  className = toString.ExtractToken ("\t\n\r");

    if  (!className.Empty ())
    {
      MLClassPtr  mlClass = MLClass::CreateNewMLClass (className);
      AddMLClass (mlClass, assignmentNum, _log);
    }
  }
}  /* ParseToString */
