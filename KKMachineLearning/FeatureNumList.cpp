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
#include "Option.h"
#include "OSservices.h"
#include "RunLog.h"
using namespace  KKB;

#include "FileDesc.h"
#include "FeatureNumList.h"
using namespace  KKMLL;


kkuint32 const  FeatureNumList::maxIntType = uint16_max;


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
  IntType*  otherFeatureNums   = _featureNumList.featureNums;
  IntType   otherNumOfFeatures = _featureNumList.numOfFeatures;
  AllocateArraySize (_featureNumList.numOfFeatures);
  for  (IntType x = 0;  x < otherNumOfFeatures; x++)
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



FeatureNumList::FeatureNumList (IntType  _maxFeatureNum):

  featureNums              (NULL),
  featureNumsAllocatedSize (0),
  maxFeatureNum            (_maxFeatureNum),
  numOfFeatures            (0)
{
  AllocateArraySize ((IntType)(_maxFeatureNum + 1));
}



FeatureNumList::FeatureNumList (FileDescConstPtr  _fileDesc):
  featureNums              (NULL),
  featureNumsAllocatedSize (0),
  maxFeatureNum            (0),
  numOfFeatures            (0)
{
  if  (_fileDesc)
    maxFeatureNum = (IntType)(_fileDesc->NumOfFields () - 1);
  AllocateArraySize ((IntType)10);
}



FeatureNumList::FeatureNumList (const BitString&  bitString):
  featureNums              (NULL),
  featureNumsAllocatedSize (0),
  maxFeatureNum            (0),
  numOfFeatures            (0)

{
  auto bitStringLen = bitString.BitLen ();
  if (bitStringLen > maxIntType)
  {
    KKStr errMsg (256);
    errMsg << "FeatureNumList (const BitString&  bitString)  bitString.BitLen()[" << bitStringLen << "] >  maxIntType[" << maxIntType << "].";
    cerr << errMsg << endl;
    throw KKException (errMsg);
  }

  maxFeatureNum = (IntType)(bitStringLen - 1);

  VectorIntType  listOfSelectedFeatures;
  bitString.ListOfSetBits16 (listOfSelectedFeatures);
  AllocateArraySize ((IntType)listOfSelectedFeatures.size ());
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



kkMemSize  FeatureNumList::MemoryConsumedEstimated ()  const
{
  kkMemSize  memoryConsumedEstimated = sizeof (FeatureNumList);
  if  (featureNums)
    memoryConsumedEstimated += sizeof (IntType) * featureNumsAllocatedSize;

  return  memoryConsumedEstimated;
}  /* MemoryConsumedEstimated */



void  FeatureNumList::AllocateArraySize (IntType size)
{
  if  (featureNumsAllocatedSize >= size)
  {
    // We have already allocated at least  'size'  for featureNums.
    return;
  }

  if  (!featureNums)
  {
    featureNums = new IntType[size];
    featureNumsAllocatedSize = size;
    for  (IntType x = 0;  x < size;  ++x)
      featureNums[x] = 0;
  }

  else
  {
    IntType*  newFeatureNums = new IntType[size];

    for  (IntType x = 0;  x < numOfFeatures;  ++x)
      newFeatureNums[x] = featureNums[x];

    for  (IntType x = numOfFeatures;  x < size;  ++x)
      newFeatureNums[x] = 0;

    delete[] featureNums;
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



FeatureNumList::IntType*  FeatureNumList::CreateFeatureNumArray ()  const
{
  IntType*  newList = new IntType[numOfFeatures];
  for  (kkint32 x = 0;  x < numOfFeatures;  x++)
    newList[x] = featureNums[x];
  return  newList;
}  /* CreateFeatureNumArray */



bool  FeatureNumList::AllFeaturesSelected (FileDescConstPtr  fileDesc)  const
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



void  FeatureNumList::UnSet (IntType  featureNum)
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



void  FeatureNumList::AddFeature (IntType  featureNum)
{
  if  (!featureNums)
  {
    featureNums = new IntType[10];
    featureNumsAllocatedSize = 10;
  }

  if  (numOfFeatures >= featureNumsAllocatedSize)
  {
    // Expand the featureNums array
    IntType  newFeatureNumsAllocatedSize = (IntType)(numOfFeatures + 10);
    IntType*  newFeatureNums = new kkuint16[newFeatureNumsAllocatedSize];

    IntType x = 0;
    for  (x = 0;  x < numOfFeatures;  ++x)
      newFeatureNums[x] = featureNums[x];

    while  (x < newFeatureNumsAllocatedSize)
    {
      newFeatureNums[x] = 0;
      ++x;
    }

    delete[] featureNums;
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
FeatureNumList   FeatureNumList::AllFeatures (FileDescConstPtr  _fileDesc)
{
  IntType  maxFeatureNum = (IntType)(_fileDesc->NumOfFields () - 1);
  FeatureNumList  features (maxFeatureNum);

  const AttributeTypeVector&   attributeTypes = _fileDesc->AttributeVector ();
  for  (IntType fn = 0;  fn <= maxFeatureNum;  ++fn)
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
void   FeatureNumList::SetAllFeatures (FileDescConstPtr  fileDesc)
{
  for  (IntType x = 0; x <= maxFeatureNum;  ++x)
  {
    if  (fileDesc->Type (x) != AttributeType::Ignore)
      AddFeature (IntType (x));
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
    IntType fn = z[idx];
    isSubSet = InList (fn);
    ++idx;
  }
  return  isSubSet;
}



bool  FeatureNumList::InList (IntType _featureNum)  const
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



bool  FeatureNumList::Test (IntType _featureNum)  const
{
  return InList (_featureNum);
}  /* Test */



FeatureNumList::IntType  FeatureNumList::operator[] (kkint32  _idx)  const
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



KKStr   FeatureNumList::ToHexString (FileDescConstPtr  fileDesc)  const
{
  BitString  bs (fileDesc->NumOfFields ());
  ToBitString (bs);
  return  bs.HexStr ();
}  /* ToHexString */



FeatureNumList::VectorIntType*  FeatureNumList::StrToUInt16Vetor (const KKStr&  s)
{
  bool  valid = true;
  VectorIntType*  results = new VectorUint16 ();

  KKStrParser parser (s);
  parser.TrimWhiteSpace (" ");

  while  (parser.MoreTokens ())
  {
    KKStr  field = parser.GetNextToken (",\t");
    if  (field.Empty ())
      continue;
    auto dashPos = field.LocateCharacter ('-');
    if  (!dashPos)
    {
      kkint32 n = field.ToInt32 ();
      if ((n < 0) || ((kkuint32)n > maxIntType))
      {
        valid = false;
        break;
      }
      results->push_back ((IntType)n);
    }
    else
    {
      // We are looking at a range
      kkint32  startNum = field.SubStrSeg (0, dashPos).ToInt32 ();
      kkint32  endNum   = field.SubStrPart (dashPos + 1).ToInt32 ();

      if  ((startNum > endNum)  ||  (startNum < 0)  ||  ((kkuint32)endNum > maxIntType))
      {
        valid = false;
        break;
      }

      for  (kkint32 z = startNum;  z <= endNum;  ++z)
        results->push_back ((IntType)z);
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
    AllocateArraySize ((IntType)(maxFeatureNum + 1));
    return;
  }

  VectorIntType*  list = StrToUInt16Vetor (_str);
  if  (list)
  {
    sort(list->begin (), list->end ());
    maxFeatureNum = list->back ();
    AllocateArraySize ((IntType)list->size ());
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
                               VolConstBool&   cancelFlag,
                               RunLog&         log
                             )
{
  maxFeatureNum = (IntType)tag->AttributeValueInt32 ("MaxFeatureNum");
  auto expectedNumOfFeatures = (IntType)tag->AttributeValueInt32 ("NumOfFeatures");
  numOfFeatures = 0;

  XmlTokenPtr  t = s.GetNextToken (cancelFlag, log);
  while  (t  &&  (!cancelFlag))
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
    t = s.GetNextToken (cancelFlag, log);
  }

  delete  t;
  t = NULL;

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
  featureNums = new IntType[numOfFeatures];
  featureNumsAllocatedSize = numOfFeatures;

  for  (kkint32 x = 0;  x < numOfFeatures;  ++x)
    featureNums[x] = _features.featureNums[x];

  return  *this;
}  /* operator= */



FeatureNumList&  FeatureNumList::operator= (FeatureNumList&&  _features)
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
  auto mfn = Max (maxFeatureNum, (IntType)rightSide.MaxFeatureNum ());
  auto bestCaseNof = Min (numOfFeatures, (IntType)rightSide.NumOfFeatures ());
  FeatureNumList  result (mfn);
  result.AllocateArraySize (bestCaseNof);

  kkint32  l = 0;
  kkint32  r = 0;

  while  ((l < numOfFeatures)  &&  (r < rightSide.numOfFeatures))
  {
    if  (featureNums[l] < rightSide.featureNums[r])
      ++l;

    else if  (featureNums[l] > rightSide.featureNums[r])
      ++r;

    else
    {
      result.AddFeature (featureNums[l]);
      ++l;
      ++r;
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
  const IntType*  rightFeatureNums = rightSide.FeatureNums ();
  auto  rightNumOfFeatures = rightSide.NumOfFeatures ();

  for  (IntType  x = 0;  x < rightNumOfFeatures;  ++x)
    AddFeature (rightFeatureNums[x]);

  return  *this;
}  /* operator+= */



/** @brief Adds the feature 'featureNum' to the selected list of features. */
FeatureNumList&  FeatureNumList::operator+= (IntType  featureNum)
{
  AddFeature (featureNum);
  return *this;
}  /* operator+= */



/** @brief Returns a new FeatureNumList instance that will consists of the left FeatureNumList instance with 'rightSide' feature added in. */
FeatureNumList  FeatureNumList::operator+ (IntType  rightSide) const
{
  FeatureNumList  result (*this);
  result.AddFeature (rightSide);
  return  result;
}  /* operator+ */



/** @brief Returns a new FeatureNumList instance that will consists of the left FeatureNumList instance with 'rightSide' removed from it. */
FeatureNumList  FeatureNumList::operator-  (IntType  rightSide) const
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
FeatureNumList&  FeatureNumList::operator-= (IntType  rightSide)
{
  UnSet (rightSide);
  return  *this;
}  /* operator-= */



FeatureNumListPtr   FeatureNumList::RandomlySelectFeatures (IntType  numToKeep, KKB::RandomNumGenerator& rng)  const
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

  // Initialize Selected Features to the currently selected features in featureNums
  IntType*  selectedFeatures = new IntType[numOfFeatures];
  std::copy (featureNums, featureNums + numOfFeatures, selectedFeatures);
  for (IntType i = (IntType)(numOfFeatures - 1); i > 1; --i)
  {
    IntType j = (IntType)(rng.Next () % (i + 1));
    std::swap (selectedFeatures[i], selectedFeatures[j]);
  }

  // Assign the first 'numToKeep'  featured from the random order of selected features
  auto randomlySelectedFeatures = new FeatureNumList (maxFeatureNum);
  for  (IntType x = 0;  x < numToKeep;  ++x)
    randomlySelectedFeatures->AddFeature (selectedFeatures[x]);

  delete  [] selectedFeatures;
  selectedFeatures = NULL;

  return  randomlySelectedFeatures;
}  /* RandomlySelectFeatures */



FeatureNumListPtr   FeatureNumList::RandomlySelectFeatures (IntType  numToKeep)  const
{
  RandomNumGenerator rng (LRand48 ());
  return RandomlySelectFeatures (numToKeep, rng);
}



FeatureNumList  FeatureNumList::Complement ()  const
{
  FeatureNumList  result (maxFeatureNum);
  IntType  x = 0;
  IntType  fni = 0;
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
