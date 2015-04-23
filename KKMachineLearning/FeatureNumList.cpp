#include "FirstIncludes.h"
#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include "MemoryDebug.h"
using namespace std;


#include "KKBaseTypes.h"
#include "KKException.h"
#include "KKStrParser.h"
#include "OSservices.h"
#include "RunLog.h"
using namespace  KKB;

#include "FileDesc.h"
#include "FeatureNumList.h"
using namespace  KKMLL;



FeatureNumList::FeatureNumList ():
  featureNums              (NULL),
  featureNumsAllocatedSize (0),
  maxFeatureNum            (0),
  numOfFeatures            (0)
{
}




FeatureNumList::FeatureNumList (const FeatureNumList&  _featureNumList):
  featureNums              (NULL),
  featureNumsAllocatedSize (0),
  maxFeatureNum            (_featureNumList.MaxFeatureNum ()),
  numOfFeatures            (0)
{
  kkuint16*  otherFeatureNums   = _featureNumList.featureNums;
  kkuint16   otherNumOfFeatures = _featureNumList.numOfFeatures;
  AllocateArraySize (_featureNumList.numOfFeatures + 1);
  for  (kkuint16 x = 0;  x < otherNumOfFeatures; x++)
    AddFeature (otherFeatureNums[x]);
}




FeatureNumList::FeatureNumList (kkuint32  _maxFeatureNum):

  featureNums              (NULL),
  featureNumsAllocatedSize (0),
  maxFeatureNum            (_maxFeatureNum),
  numOfFeatures            (0)
{
  AllocateArraySize (_maxFeatureNum + 1);
}



FeatureNumList::FeatureNumList (FileDescPtr  _fileDesc):
  featureNums              (NULL),
  featureNumsAllocatedSize (0),
  maxFeatureNum            (0),
  numOfFeatures            (0)
{
  if  (_fileDesc)
    maxFeatureNum = _fileDesc->NumOfFields () - 1;
  AllocateArraySize (10);
}



FeatureNumList::FeatureNumList (const BitString&  bitString):
  featureNums              (NULL),
  featureNumsAllocatedSize (0),
  maxFeatureNum            (0),
  numOfFeatures            (0)

{
  maxFeatureNum = bitString.BitLen () - 1;

  VectorUint16  listOfSelectedFeatures;
  bitString.ListOfSetBits16 (listOfSelectedFeatures);
  AllocateArraySize ((kkint32)listOfSelectedFeatures.size ());
  for  (kkuint32 x = 0;  x < listOfSelectedFeatures.size ();  x++)
    AddFeature (listOfSelectedFeatures[x]);
}




FeatureNumList::FeatureNumList (const KKStr&  _featureListStr,
                                bool&         _valid
                               ):

  featureNums              (NULL),
  featureNumsAllocatedSize (0),
  maxFeatureNum            (0),
  numOfFeatures            (0)

{
  FeatureNumListPtr  selFeatures = ExtractFeatureNumsFromStr (_featureListStr);
  if  (selFeatures == NULL)
  {
    _valid = false;
    return;
  }

  maxFeatureNum = selFeatures->MaxFeatureNum ();
  AllocateArraySize (selFeatures->NumOfFeatures ());
  const kkuint16*  selFeatureNums = selFeatures->FeatureNums ();

  for  (kkuint16 x = 0;  x < selFeatures->NumOfFeatures ();  ++x)
    AddFeature (selFeatureNums[x]);

  delete  selFeatures;
  selFeatures = NULL;
  return;
}




FeatureNumList::~FeatureNumList ()
{
  delete [] featureNums;
}



kkint32  FeatureNumList::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = sizeof (FeatureNumList);
  if  (featureNums)
    memoryConsumedEstimated += sizeof (kkuint16) * featureNumsAllocatedSize;

  return  memoryConsumedEstimated;
}  /* MemoryConsumedEstimated */



void  FeatureNumList::AllocateArraySize (kkuint16 size)
{
  if  (featureNumsAllocatedSize >= size)
  {
    // We have already allocated at least  'size'  for featureNums.
    return;
  }

  if  (!featureNums)
  {
    featureNums = new kkuint16[size];
    featureNumsAllocatedSize = size;
    for  (kkuint16 x = 0;  x < size;  ++x)
      featureNums[x] = 0;
  }

  else
  {
    kkuint16*  newFeatureNums = new kkuint16[size];

    kkuint16  x;

    for  (x = 0;  x < numOfFeatures;  ++x)
      newFeatureNums[x] = featureNums[x];

    for  (x = numOfFeatures;  x < size;  ++x)
      newFeatureNums[x] = 0;

    delete  [] featureNums;
    featureNums = newFeatureNums;
    newFeatureNums = NULL;
    featureNumsAllocatedSize = size;
  }
}  /* AllocateArraySize */



void   FeatureNumList::ToBitString (BitString&  bitStr)  const
{
  bitStr.ReSet ();
  kkint32  x;

  for  (x = 0;  x < NumOfFeatures ();  x++)
    bitStr.Set (featureNums[x]);
}  /* ToBitString */



kkuint16*  FeatureNumList::CreateFeatureNumArray ()  const
{
  kkuint16*  newList = new kkuint16[numOfFeatures];
  for  (kkint32 x = 0;  x < numOfFeatures;  x++)
    newList[x] = featureNums[x];
  return  newList;
}  /* CreateFeatureNumArray */



bool  FeatureNumList::AllFeaturesSelected (FileDescPtr  fileDesc)  const
{
  if  (numOfFeatures >= (kkint32)fileDesc->NumOfFields ())
    return true;
  return false;
}  /* AllFeaturesSelected */



void  FeatureNumList::UnSet ()
{
  if  (featureNums)
  {
    kkint32 x;
    for  (x = 0;  x < featureNumsAllocatedSize;  x++)
      featureNums[x] = 0;
    numOfFeatures = 0;
  }
  numOfFeatures = 0;
}  /* UnSet */



void  FeatureNumList::UnSet (kkuint16  featureNum)
{
  if  (!featureNums)
    return;

  kkint32  indexToDel = numOfFeatures - 1;  // Starting with last index

  while  ((indexToDel >= 0)  &&  (featureNums[indexToDel] > featureNum))
    indexToDel--;

  if  (indexToDel >= 0)
  {
    if  (featureNums[indexToDel] == featureNum)
    {
      // We found the index to delete.
      kkint32  x;
      for  (x = indexToDel;  x < (numOfFeatures - 1);  x++)
        featureNums[x] = featureNums[x + 1];
    }
 
    numOfFeatures--;
    featureNums[numOfFeatures] = 0;
  }
}  /* UnSet */



void  FeatureNumList::AddFeature (kkuint16  featureNum)
{
  if  (!featureNums)
  {
    featureNums = new kkuint16[10];
    featureNumsAllocatedSize = 10;
  }

  if  (numOfFeatures >= featureNumsAllocatedSize)
  {
    // Expand the featureNums array
    kkint32  newFeatureNumsAllocatedSize = numOfFeatures + 10;
    kkuint16*  newFeatureNums = new kkuint16[newFeatureNumsAllocatedSize];

    kkint32 x = 0;
    for  (x = 0;  x < numOfFeatures;  ++x)
      newFeatureNums[x] = featureNums[x];

    while  (x < newFeatureNumsAllocatedSize)
    {
      newFeatureNums[x] = 0;
      ++x;
    }

    delete  [] featureNums;
    featureNums = newFeatureNums;
    featureNumsAllocatedSize = newFeatureNumsAllocatedSize;
  }
  
  if  (numOfFeatures == 0)
  {
    featureNums[0] = featureNum;
    numOfFeatures = 1;
  }

  else
  {
    kkint32  x = numOfFeatures - 1; // Setting x to end of list
    if  (featureNums[x] < featureNum)
    {
      // Adding feature to end of list.
      featureNums[numOfFeatures] = featureNum;
      numOfFeatures++;
    }

    else if  (!InList (featureNum))
    {
      while  ((x >= 0)  &&  (featureNums[x] > featureNum))
      {
        featureNums[x + 1] = featureNums[x];
        featureNums[x]     = featureNum;
        x--;
      }

      numOfFeatures++;
    }
  }

  return;
}  /* AddFeature */


/**
 *@details  A static method that will return a instance of 'FeatureNumList' that will have all non 'IgnoreAttribute' features 
 * in '_fileDesc' selected.
 */
FeatureNumList   FeatureNumList::AllFeatures (FileDescPtr  _fileDesc)
{
  kkuint16  maxFeatureNum = _fileDesc->NumOfFields () - 1;
  FeatureNumList  features (maxFeatureNum);

  const AttributeTypeVector&   attributeTypes = _fileDesc->AttributeVector ();
  for  (kkuint16 fn = 0;  fn <= maxFeatureNum;  ++fn)
  {
    if  (attributeTypes[fn] != AttributeType::IgnoreAttribute)
      features.AddFeature (fn);
  }

  return  features;
}  /* AllFeatures */




/**
 *@details  Using 'fileDesc' as the guide as to how many features there are and which ones are to 
 * be ignored will set all features that are not 'IgnoreAttribute' to on.
 */
void   FeatureNumList::SetAllFeatures (FileDescPtr  fileDesc)
{
  for  (kkuint16 x = 0; x <= maxFeatureNum;  ++x)
  {
    if  (fileDesc->Type (x) != AttributeType::IgnoreAttribute)
      AddFeature (kkuint16 (x));
  }
  return;
}  /* SetAllFeatures */




/**
 *@brief  Returns true if the FeatureNumList instance 'z' is a subset of this instance.
 */
bool  FeatureNumList::IsSubSet (const FeatureNumList&  z)
{
  bool  isSubSet = true;

  kkint32  idx = 0;
  while  ((idx < z.NumSelFeatures ())  &&  isSubSet)
  {
    kkint32 fn = z[idx];
    isSubSet = InList (fn);
    idx++;
  }
  return  isSubSet;
}



bool  FeatureNumList::InList (kkuint16 _featureNum)  const
{
  bool  found = false;
  kkint32  x = 0;

  while  ((x < numOfFeatures)  && (!found))
  {
    found = (_featureNum == featureNums[x]);
    if  (!found)
      x++;
  }

  return  found;
}  /* InList */



bool  FeatureNumList::Test (kkuint16 _featureNum)  const
{
  return InList (_featureNum);
}  /* Test */



kkuint16  FeatureNumList::operator[] (kkint32  _idx)  const
{
  if  (_idx >= numOfFeatures)
  {
    KKStr  errMsg (100);
    errMsg << "FeatureNumList::operator[]  ***ERROR***   Invalid Index[" << _idx << "] requested.";
    cerr << endl << errMsg << endl << endl;
    throw  KKException (errMsg);
  }
  else
  {
    return featureNums[_idx];
  }
}



KKStr  FeatureNumList::ToString ()  const
{
  KKStr  featureNumStr (numOfFeatures * 6);
  
  if  (numOfFeatures <= 0)
    return featureNumStr;

  kkint32  nextIdx = 0;

  while  (nextIdx < numOfFeatures)
  {
    kkint32  startOfGroup = nextIdx;
    kkint32  endOfGroup   = nextIdx;

    while  ((endOfGroup < (numOfFeatures - 1))  &&  
            (featureNums[endOfGroup] == (featureNums[endOfGroup + 1] - 1))
           )
    {
      endOfGroup++;
    }

    if  ((endOfGroup - startOfGroup) < 3)
    {
      kkint32  x;
      for  (x = startOfGroup;  x <= endOfGroup; x++)
      {
        if  (!featureNumStr.Empty ())
          featureNumStr << ",";
        featureNumStr << featureNums[x];
      }
    }
    else
    {
      if  (!featureNumStr.Empty ())
        featureNumStr << ",";
      featureNumStr << featureNums[startOfGroup] << "-" << featureNums[endOfGroup];
    }

    nextIdx = endOfGroup + 1;
  }

  return  featureNumStr;
}  /* ToString */



KKStr   FeatureNumList::ToHexString ()  const
{
  BitString  bs (maxFeatureNum + 1);
  ToBitString (bs);
  return  bs.HexStr ();
}  /* ToHexString */



KKStr   FeatureNumList::ToHexString (FileDescPtr  fileDesc)  const
{
  BitString  bs (fileDesc->NumOfFields ());
  ToBitString (bs);
  return  bs.HexStr ();
}  /* ToHexString */




VectorUint16*  FeatureNumList::StrToUInt16Vetor (const KKStr&  s)
{
  bool  valid = true;
  VectorUint16*  results = new VectorUint16 ();

  KKStrParser parser (s);

  KKStr  field = parser.GetNextToken (",\t \n\r");
  while  (!field.Empty ())
  {
    kkint32 dashPos = field.LocateCharacter ('-');
    if  (dashPos < 0)
    {
      kkint32 n = field.ToInt32 ();
      if  ((n < 0)  ||  (n > uint16_max))
      {
        valid = false;
        break;
      }
      results->push_back (n);
    }
    else
    {
      // We are looking at a range
      kkint32  startNum = field.SubStrPart (0, dashPos - 1).ToInt32 ();
      kkint32  endNum   = field.SubStrPart (dashPos + 1).ToInt32 ();

      if  ((startNum > endNum)  ||  (startNum < 0)  ||  (endNum > uint16_max))
      {
        valid = false;
        break;
      }

      for  (kkuint16 z = startNum;   z <= endNum;  ++z)
        results->push_back (z);
    }
    field = parser.GetNextToken (",\t \n\r");
  }

  if  (!valid)
  {
    delete  results;
    results = NULL;
  }
  else
  {
    sort (results->begin (), results->end ());
  }

  return  results;
}  /* StrToUInt16Vetor */



/**
 *@brief  A static method that will return a new instance 'FeatureNum2List' where the features listed in
 * '_featureListStr' will be turned on.  'maxFeatureNum' will be set to the highest feature number listed in 
 * '_featureListStr'.
 */
FeatureNumListPtr  FeatureNumList::ExtractFeatureNumsFromStr (const KKStr&  _featureListStr)  
{
  bool  valid = true;

  if  (_featureListStr.EqualIgnoreCase ("NONE"))
    return new FeatureNumList (1);

  VectorUint16*  list = StrToUInt16Vetor (_featureListStr);
  if  (!list)
  {
    delete  list;
    list= NULL;
    return NULL;
  }

  if  (list->size () < 1)
  {
    delete  list;
    list = NULL;
    return new FeatureNumList (1);
  }

  kkuint16  firstNum = (*list)[0];
  kkuint16  lastNum  = (*list)[list->size () - 1];

  FeatureNumListPtr  result = new FeatureNumList (lastNum);
  VectorUint16::const_iterator  idx;
  for  (idx = list->begin ();  idx != list->end ();  ++idx)
    result->AddFeature (*idx);

  delete  list;
  list = NULL;
  return  result;
}  /* ExtractFeatureNumsFromStr */






void  FeatureNumList::Load (const KKStr&  _fileName,
                             bool&         _successful,
                             RunLog&       _log
                            )
{
  _log.Level (20) << "FeatureNumList::Load - File[" << _fileName << "]." << endl;

  delete  featureNums;
  featureNums = NULL;
  featureNumsAllocatedSize = 0;
  numOfFeatures = 0;
  maxFeatureNum = 0;

  FILE*  inputFile = osFOPEN (_fileName.Str (), "r");
  if  (!inputFile)
  {
    _log.Level (-1) << endl << "FeatureNumList::Load      ***ERROR***  Could Not Open File[" << _fileName << "]." << endl << endl;
    _successful = false;
    return;
  }

  kkuint16  mfn = 0;
  kkuint16  nof = 0;
  KKStr     featureNumStr = "";

  kkint32 fileDescNumOfFields = 0;
  char  buff [102400];

  while  (fgets (buff, sizeof (buff), inputFile))
  {
    KKStr  line (buff);
    KKStr  fieldName = line.ExtractToken2 ("\n\t\r").Trim ();

    if  (fieldName.EqualIgnoreCase ("MaxFeatureNum"))
      mfn = (kkuint16)line.ExtractTokenUint ("\t\n\r");

    else if  (fieldName.EqualIgnoreCase ("NumOfFeatures"))
      nof = (kkuint16)line.ExtractTokenUint ("\t\n\r");

    else if  (fieldName.EqualIgnoreCase ("FeatureNums"))
      featureNumStr = (kkuint16)line.ExtractTokenUint ("\t\n\r");
  }

  fclose (inputFile);
  
  if  (mfn < 1)
  {
    _log.Level (-1) << endl << "FeatureNumList::Load   ***ERROR***   'maxFeatureNum'  was not defined."  << endl << endl;
    _successful = false;
  }

  if  (featureNumStr.Empty ())
  {
    _log.Level (-1) << endl << "FeatureNumList::Load   ***ERROR***   'featureNums'  was not specified."  << endl << endl;
    _successful = false;
    return;
  }

  VectorUint16*  list = StrToUInt16Vetor (featureNumStr);
  if  (!list)
  {
    _log.Level (-1) << endl << "FeatureNumList::Load   ***ERROR***   'featureNums'  included invalid features."  << endl << endl;
    _successful = false;
    return;
  }

  kkuint16  firstNum = (*list)[0];
  kkuint16  lastNum  = (*list)[list->size () - 1];
  if  (lastNum > mfn)
  {
    _log.Level (-1) << endl << "FeatureNumList::Load   ***ERROR***   'featureNums'  included features[" << lastNum << "]  which is larger than maxFeaureNum[" << maxFeatureNum << "]" << endl << endl;
    _successful = false;
  }

  else if  (list->size () != nof)
  {
    _log.Level (-1) << endl << "FeatureNumList::Load   ***ERROR***   numOfFeatures[" << nof << "] did not agree with number of features specified in FeatureNums" << endl << endl;
    _successful = false;
  }

  if  (_successful)
  {
    maxFeatureNum = mfn;
    AllocateArraySize (nof + 1);
    VectorUint16::const_iterator  idx;
    for  (idx = list->begin ();  idx != list->end ();  ++idx)
      AddFeature (*idx);
  }
  delete  list;
  list = NULL;
  return;
}  /* Load */



void  FeatureNumList::Save (const KKStr&  fileName)
{
  ofstream outFile (fileName.Str ());
  outFile << "MaxFeatureNum" << "\t" << maxFeatureNum << endl;
  outFile << "NumOfFeatures" << "\t" << numOfFeatures << endl;
  outFile << "FeatureNums"   << "\t" << ToString ()   << endl;
  outFile.close ();
}  /* Save */




void  FeatureNumList::SaveXML (ostream&  o)
{
  o << "<FeatureNumList>" << "\t"
    << "MaxFeatureNum"    << "\t"  << maxFeatureNum << "\t"
    << "NumOfFeatures"    << "\t"  << numOfFeatures << "\t"
    << "FeatureNums"      << "\t"  << ToString ()
    << "</FeatureNumList>";
}  /* Save */



void  FeatureNumList::WriteXML (const KKStr&  varName,
                                ostream&      o
                               )  const
{
  XmlTag  startTag ("FeatureNumList", XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  o << endl;

  XmlElementInt32::WriteXML (maxFeatureNum, "MaxFeatureNum", o);
  XmlElementInt32::WriteXML (numOfFeatures, "NumOfFeatures", o);
  XmlElementArrayUint16::WriteXML (numOfFeatures, featureNums, "FeatureNums", o);

  XmlTag  endTag ("FeatureNumList", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
}



void  FeatureNumList::ReadXML (XmlStream&      s,
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
      const KKStr&  className = e->Name ();
      const KKStr&  varName = e->VarName ();
      if  (varName.EqualIgnoreCase ("MaxFeatureNum"))
      {
        XmlElementInt32Ptr  mfn = dynamic_cast<XmlElementInt32Ptr>(e);
        maxFeatureNum = mfn->Value ();
      }

      else if  (varName.EqualIgnoreCase ("NumOfFeatures"))
      {
        XmlElementInt32Ptr  nof = dynamic_cast<XmlElementInt32Ptr>(e);
        numOfFeatures = nof->Value ();
      }

      else if  (varName.EqualIgnoreCase ("FeatureNums"))
      {
        XmlElementArrayUint16Ptr  nof = dynamic_cast<XmlElementArrayUint16Ptr>(e);
        featureNums = nof->TakeOwnership ();
        featureNumsAllocatedSize = nof->Count ();
      }
    }

    delete t;
    t = s.GetNextToken (log);
  }
}




FeatureNumList&  FeatureNumList::operator= (const FeatureNumList&  _features)
{
  delete featureNums;
  featureNums = NULL;

  numOfFeatures = _features.NumOfFeatures ();
  maxFeatureNum = _features.MaxFeatureNum ();
  featureNums = new kkuint16[numOfFeatures];
  featureNumsAllocatedSize = numOfFeatures;
  const kkuint16*  rightFeatureNums = _features.FeatureNums ();

  for  (kkint32 x = 0;  x < numOfFeatures;  ++x)
    featureNums[x] = _features[x];

  return  *this;
}  /* operator= */



FeatureNumList&  FeatureNumList::operator=  (const FeatureNumListPtr  _features)
{
  *this = *_features;
  return  *this;
}



kkint32  FeatureNumList::Compare (const FeatureNumList&  _features)  const
{
  kkint32  x = 0;

  while  ((x < numOfFeatures)  &&  (x < _features.NumOfFeatures ()))
  {
    if  (featureNums[x] < _features.featureNums[x])
      return  -1;

    else if  (featureNums[x] > _features.featureNums[x])
      return   1;

    x++;
  }

  if  (x < numOfFeatures)
    return  1;

  else if  (x < _features.numOfFeatures)
    return   -1;

  return  0;
}  /* Compare */



/**  @brief  Indicates if the two FeatureNumList instances have the same features selected. */
bool  FeatureNumList::operator== (const FeatureNumList&  _features)  const
{
  if  (numOfFeatures != _features.numOfFeatures)
  {
    // No point even comparing the list of feature numbers, if the lengths are 
    // different, then they can not be equal.
    return false;
  }

  return  Compare (_features) == 0;
}  /* operator== */



/**
 * @brief  Indicates if the Left FeatureNumLiost instances is less than the right one.
 * @see Compare
 */
bool  FeatureNumList::operator< (const FeatureNumList&  _features)  const
{
  return  Compare (_features) < 0;
}  /* operator< */



/**
 * @brief  Indicates if the Left FeatureNumLiost instances is greater than the right one.
 * @see Compare
 */
bool  FeatureNumList::operator> (const FeatureNumList&  _features)  const
{
  return  Compare (_features) > 0;
}  /* operator> */



namespace  KKMLL
{
  ostream& operator<< (      ostream&          os, 
                       const FeatureNumList&   features
                      )
  {
    os << features.ToString ();
    return  os;
  }


  ostream& operator<< (      ostream&            os, 
                       const FeatureNumListPtr&  features
                      )
  {
    os << features->ToString ();
    return  os;
  }
}



/**
 *@brief Returns the intersection of the two FeatureNumList instances.
 *@details Will return a new FeatureNumList instance that will consist of a list of features that are selected in both the left and right FeatureNumList
 * instances.
 */
FeatureNumList  FeatureNumList::operator*  (const FeatureNumList&  rightSide)  const
{
  kkuint16 mfn = Max (maxFeatureNum, rightSide.MaxFeatureNum ());
  kkuint16 bestCaseNof = Min (numOfFeatures, rightSide.NumOfFeatures ());
  FeatureNumList  result (mfn);
  result.AllocateArraySize (bestCaseNof);

  kkint32  l = 0;
  kkint32  r = 0;

  while  ((l < numOfFeatures)  &&  (r < rightSide.numOfFeatures))
  {
    if  (featureNums[l] < rightSide.featureNums[r])
      l++;

    else if  (featureNums[l] > rightSide.featureNums[r])
      r++;

    else
    {
      result.AddFeature (featureNums[l]);
      l++;
      r++;
    }
  }

  return  result;
}  /* operator* */




/**
 * @brief Performs a logical 'OR'  operation on the two FeatureNumList instances.
 * @details Will return a new FeatureNumList instance that will consist of a list of features that are in either
 * left and right FeatureNumList instances.  Both FeatureNumList objects must be referencing the same FileDesc instance
 * otherwise an exception will be thrown.
 */
FeatureNumList  FeatureNumList::operator+ (const FeatureNumList&  rightSide) const
{
  FeatureNumList  result (rightSide);

  kkint32  l = 0;
  for  (l = 0;  l < numOfFeatures; l++)
    result.AddFeature (featureNums[l]);

  return  result;
}  /* operator+ */



/**
 * @brief Adds the features that are selected in the right FeatureNumList instance to the left instance.
 * @details Both FeatureNumList objects must be referencing the same FileDesc instance otherwise an exception will be thrown.
 */
FeatureNumList&  FeatureNumList::operator+= (const FeatureNumList&  rightSide)
{
  const kkuint16*  rightFeatureNums = rightSide.FeatureNums ();
  kkuint16  rightNumOfFeatures = rightSide.NumOfFeatures ();

  for  (kkuint16  x = 0;  x < rightNumOfFeatures;  ++x)
    AddFeature (rightFeatureNums[x]);

  return  *this;
}  /* operator+= */




/** @brief Adds the feature 'featureNum' to the selected list of features. */
FeatureNumList&  FeatureNumList::operator+= (kkuint16  featureNum)
{
  AddFeature (featureNum);
  return *this;
}  /* operator+= */





/** @brief Returns a new FeatureNumList instance that will consists of the left FeatureNumList instance with 'rightSide' feature added in. */
FeatureNumList  FeatureNumList::operator+ (kkuint16  rightSide) const
{
  FeatureNumList  result (*this);
  result.AddFeature (rightSide);
  return  result;
}  /* operator+ */




/** @brief Returns a new FeatureNumList instance that will consists of the left FeatureNumList instance with 'rightSide' removed from it. */
FeatureNumList  FeatureNumList::operator-  (kkuint16  rightSide) const
{
  FeatureNumList  result (*this);
  result.UnSet (rightSide);
  return  result;
}  /* operator- */



/**
 * @brief Returns a new FeatureNumList instance that consists of the left side instance with the
 *  selected features in the right side removed.
 * @details Both FeatureNumList objects must be referencing the same FileDesc instance otherwise
 *  an exception will be thrown.
 */
FeatureNumList  FeatureNumList::operator- (const FeatureNumList&  rightSide) const
{
  FeatureNumList  result (maxFeatureNum);

  kkint32  l = 0;
  kkint32  r = 0;

  while  ((l < numOfFeatures)  &&  (r < rightSide.numOfFeatures))
  {
    if  (featureNums[l] < rightSide.featureNums[r])
    {
      result.AddFeature (featureNums[l]);
      l++;
    }

    else if  (featureNums[l] > rightSide.featureNums[r])
      r++;

    else
    {
      l++;
      r++;
    }
  }

  return  result;
}  /* operator- */




/** @brief removes the feature specified on the right side from the FeatureNumList on the left side. */
FeatureNumList&  FeatureNumList::operator-= (kkuint16  rightSide)
{
  UnSet (rightSide);
  return  *this;
}  /* operator-= */



FeatureNumListPtr   FeatureNumList::RandomlySelectFeatures (kkint32  numToKeep)  const
{
  if  (numToKeep > numOfFeatures)
  {
    cerr << endl
         << endl
         << "FeatureNumList::RandomlySelectFeatures    *** ERROR ***" << endl
         << endl
         << "NumToKeep[" << numToKeep << "]  Is greater than  NumOfFeatures[" << numOfFeatures << "]" << endl
         << endl
         << endl;
    numToKeep = numOfFeatures;
  }

  FeatureNumListPtr  randomlySelectedFeatures = new FeatureNumList (maxFeatureNum);

  kkint32 x, y, z;

  // Initialize Selected Features to the currently selected features in featureNums
  kkuint16*  selectedFeatures = new kkuint16[numOfFeatures];
  for  (x = 0; x < numOfFeatures; x++)
    selectedFeatures[x] = featureNums[x];

  // Randomize the order of the selected featured
  //for (x = 0; x < numOfFeatures; x++)
  for (x = 0; x < numToKeep; x++)
  {
    y = LRand48() % numOfFeatures;
    z = selectedFeatures[x];
    selectedFeatures[x] = selectedFeatures[y];
    selectedFeatures[y] = z;
  }

  // Assign the first 'numToKeep'  featured from the random order of selected features
  for  (x = 0;  x < numToKeep;  x++)
    randomlySelectedFeatures->AddFeature (selectedFeatures[x]);

  delete  [] selectedFeatures;

  return  randomlySelectedFeatures;
}  /* RandomlySelectFeatures */




FeatureNumList  FeatureNumList::Complement ()  const
{
  FeatureNumList  result (maxFeatureNum);
  kkuint16  x = 0;
  kkuint16  fni = 0;
  while  (fni < numOfFeatures)
  {
    while  (x < featureNums[fni])
    {
      result.AddFeature (x);
      ++x;
    }
    ++fni;
  }

  while  (x < maxFeatureNum)
  {
    result.AddFeature (x);
    ++x;
  }
  return  result;
}  /* Complement */





XmlElementFeatureNumList::XmlElementFeatureNumList (XmlTagPtr   tag,
                                                    XmlStream&  s,
                                                    RunLog&     log
                                                   ):
  XmlElement (tag, s, log),
  value (NULL)
{
  value = new FeatureNumList ();
  value->ReadXML (s, tag, log);

}
 


XmlElementFeatureNumList::~XmlElementFeatureNumList ()
{
  delete  value;
  value = NULL;
}



FeatureNumListPtr  XmlElementFeatureNumList::TakeOwnership ()
{
  FeatureNumListPtr  v = value;
  value = NULL;
  return  v;
}



void  XmlElementFeatureNumList::WriteXML (const FeatureNumList&  fnl,
                                          const KKStr&           varName,
                                          ostream&               o
                                         )
{
  fnl.WriteXML (varName, o);
}
