#include "FirstIncludes.h"
#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include "MemoryDebug.h"
using namespace std;


#include "GlobalGoalKeeper.h"
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




FeatureNumList::FeatureNumList (FeatureNumList  &&featureNumList):
  featureNums               (featureNumList.featureNums),
  featureNumsAllocatedSize  (featureNumList.featureNumsAllocatedSize),
  maxFeatureNum             (featureNumList.maxFeatureNum),
  numOfFeatures             (featureNumList.numOfFeatures)

{
  featureNumList.featureNums              = NULL;
  featureNumList.featureNumsAllocatedSize = 0;
  featureNumList.maxFeatureNum            = 0;
  featureNumList.numOfFeatures            = 0;
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
  ParseToString (_featureListStr, _valid);
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

  if  (featureNum > maxFeatureNum)
    maxFeatureNum = featureNum;

  return;
}  /* AddFeature */




/**
 *@details  A static method that will return a instance of 'FeatureNumList' that will have all non 'Ignore' features 
 * in '_fileDesc' selected.
 */
FeatureNumList   FeatureNumList::AllFeatures (FileDescPtr  _fileDesc)
{
  kkuint16  maxFeatureNum = _fileDesc->NumOfFields () - 1;
  FeatureNumList  features (maxFeatureNum);

  const AttributeTypeVector&   attributeTypes = _fileDesc->AttributeVector ();
  for  (kkuint16 fn = 0;  fn <= maxFeatureNum;  ++fn)
  {
    if  (attributeTypes[fn] != AttributeType::Ignore)
      features.AddFeature (fn);
  }

  return  features;
}  /* AllFeatures */




/**
 *@details  Using 'fileDesc' as the guide as to how many features there are and which ones are to 
 * be ignored will set all features that are not 'Ignore' to on.
 */
void   FeatureNumList::SetAllFeatures (FileDescPtr  fileDesc)
{
  for  (kkuint16 x = 0; x <= maxFeatureNum;  ++x)
  {
    if  (fileDesc->Type (x) != AttributeType::Ignore)
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
  parser.TrimWhiteSpace (" ");

  while  (parser.MoreTokens ())
  {
    KKStr  field = parser.GetNextToken (",\t");
    if  (field.Empty ())
      continue;
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






void  FeatureNumList::ParseToString (const KKStr&  _str,
                                     bool&         _valid
                                    )  
{
  _valid = true;
  delete  featureNums;

  featureNumsAllocatedSize = 0;
  maxFeatureNum            = 0;
  numOfFeatures            = 0;
  featureNums              = NULL;

  if  (_str.EqualIgnoreCase ("NONE"))
  {
    maxFeatureNum = 1;
    AllocateArraySize (maxFeatureNum + 1);
    return;
  }

  VectorUint16*  list = StrToUInt16Vetor (_str);
  if  (list)
  {
    sort(list->begin (), list->end ());
    maxFeatureNum = list->back ();
    AllocateArraySize (list->size ());
    for  (auto idx: *list)
      AddFeature (idx);
  }
  else
  {
    _valid = false;
  }
  delete  list;
  list = NULL;
}  /* ParseToString */




void  FeatureNumList::WriteXML (const KKStr&  varName,
                                ostream&      o
                               )  const
{
  XmlTag  startTag ("FeatureNumList", XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.AddAtribute ("MaxFeatureNum", maxFeatureNum);
  startTag.AddAtribute ("NumOfFeatures", numOfFeatures);
  startTag.WriteXML (o);

  o << ToString ();

  XmlTag  endTag ("FeatureNumList", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}



void  FeatureNumList::ReadXML (XmlStream&      s,
                               XmlTagConstPtr  tag,
                               RunLog&         log
                             )
{
  maxFeatureNum = tag->AttributeValueInt32 ("MaxFeatureNum");
  kkuint32 expectedNumOfFeatures = tag->AttributeValueInt32 ("NumOfFeatures");
  numOfFeatures = 0;

  kkuint32 featureCountRead = 0;

  XmlTokenPtr  t = s.GetNextToken (log);
  while  (t)
  {
    if  (typeid (*t) == typeid(XmlContent))
    {
      XmlContentPtr c = dynamic_cast<XmlContentPtr> (t);
      if  (c  &&  (c->Content ()))
      {
        bool  valid = false;
        ParseToString (*(c->Content ()), valid);
      }
    }

    delete  t;
    t = s.GetNextToken (log);
  }

  if  (expectedNumOfFeatures != numOfFeatures)
  {
    log.Level (-1) << endl
      << "FeatureNumList::ReadXML   ***ERROR***   expectedNumOfFeatures["  << expectedNumOfFeatures << "]  not equal  numOfFeatures[" << numOfFeatures << "]" << endl
      << endl;
  }
}  /* ReadXML */




FeatureNumList&  FeatureNumList::operator= (const FeatureNumList&  _features)
{
  delete featureNums;
  featureNums = NULL;

  numOfFeatures = _features.NumOfFeatures ();
  maxFeatureNum = _features.MaxFeatureNum ();
  featureNums = new kkuint16[numOfFeatures];
  featureNumsAllocatedSize = numOfFeatures;

  for  (kkint32 x = 0;  x < numOfFeatures;  ++x)
    featureNums[x] = _features.featureNums[x];

  return  *this;
}  /* operator= */





FeatureNumList&  FeatureNumList::operator=  (FeatureNumList&&  _features)
{
  delete featureNums;
  featureNums              = _features.featureNums;
  numOfFeatures            = _features.numOfFeatures;
  maxFeatureNum            = _features.maxFeatureNum;
  featureNumsAllocatedSize = _features.featureNumsAllocatedSize;

  _features.featureNums              = NULL;
  _features.numOfFeatures            = 0;
  _features.maxFeatureNum            = 0;
  _features.featureNumsAllocatedSize = 0;

  return  *this;
}





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



XmlFactoryMacro(FeatureNumList)

