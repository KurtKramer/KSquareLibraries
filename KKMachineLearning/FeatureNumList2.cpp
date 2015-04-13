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
#include "OSservices.h"
#include "RunLog.h"
using namespace  KKB;


#include "FeatureNumList2.h"
using namespace  KKMLL;


FeatureNumList2::FeatureNumList2 (const FeatureNumList2&  _featureNumList):
  featureNums              (NULL),
  featureNumsAllocatedSize (0),
  maxFeatureNum            (_featureNumList.MaxFeatureNum ()),
  numOfFeatures            (_featureNumList.NumOfFeatures ())
{
  if  ((numOfFeatures < 0)  ||  (numOfFeatures >= maxFeatureNum))
  {
    KKStr  errMsg (100);
    errMsg << "FeatureNumList2::FeatureNumList2     numOfFeatures[" << numOfFeatures << "]  is out of range of maxFeatureNum[" < maxFeatureNum << "]";
    cerr << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  kkuint16*  otherFeatureNums = _featureNumList=->featureNums;
  featureNums = new kkuint16[numOfFeatures + 1];
  featureNumsAllocatedSize = numOfFeatures + 1;
  for  (kkint32 x = 0; x < numOfFeatures; x++)
  {
    kkuint16  fn = otherFeatureNums[x];
    if  (fn > maxFeatureNum)
    {
      KKStr  errMsg (100);
      errMsg << "FeatureNumList2::FeatureNumList2   Selected feature[" << fn << "]  is out of range of maxFeatureNum[" < maxFeatureNum << "]";
      cerr << endl << errMsg << endl << endl;
      throw KKException (errMsg);
    }
    featureNums[x] = fn;
  }
}




FeatureNumList2::FeatureNumList2 (kkint32  _maxFeatureNum):

  featureNums              (NULL),
  featureNumsAllocatedSize (0),
  maxFeatureNum            (_maxFeatureNum),
  numOfFeatures            (0)
{
  AllocateArraySize (_maxFeatureNum + 1);
}



FeatureNumList2::FeatureNumList2 (const BitString&  bitString):
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




FeatureNumList2::FeatureNumList2 (const KKStr&  _featureListStr,
                                  bool&         _valid
                                 ):

  featureNums              (NULL),
  featureNumsAllocatedSize (0),
  maxFeatureNum            (0),
  numOfFeatures            (0)

{
  FeatureNumListPttr  selFeatures = ExtractFeatureNumsFromStr (_featureListStr, _valid);
  if  (selFeatures == NULL)
  {
    _valid = false;
    return;
  }

  maxFeatureNum = selFeatures->MaxFeatureNum ();
  AllocateArraySize (selFeatures->NumOfFeatures ());

  for  (kkuint16 x = 0;  x < selFeatures->NumOfFeatures ();  ++x)
    AddFeature (selFeatures[x]);

  delete  selFeatures;
  selFeatures = NULL;
  return;
}





FeatureNumList2::FeatureNumList2 ():
  featureNums              (NULL),
  featureNumsAllocatedSize (0),
  numOfFeatures            (0)
{
}



FeatureNumList2::~FeatureNumList2 ()
{
  delete [] featureNums;
}



kkint32  FeatureNumList2::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = sizeof (FeatureNumList2);
  if  (featureNums)
    memoryConsumedEstimated += sizeof (kkuint16) * featureNumsAllocatedSize;

  return  memoryConsumedEstimated;
}  /* MemoryConsumedEstimated */



void  FeatureNumList2::AllocateArraySize (kkint32 size)
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
    for  (kkint32 x = 0;  x < size;  x++)
      featureNums[x] = 0;
  }

  else
  {
    kkuint16*  newFeatureNums = new kkuint16[size];

    kkint32  x;

    for  (x = 0;  x < numOfFeatures; x++)
      newFeatureNums[x] = featureNums[x];

    for  (x = numOfFeatures;  x < size;  x++)
      newFeatureNums[x] = 0;


    delete  [] featureNums;
    featureNums = newFeatureNums;
    featureNumsAllocatedSize = size;
  }
}  /* AllocateArraySize */



void   FeatureNumList2::ToBitString (BitString&  bitStr)  const
{
  bitStr.ReSet ();
  kkint32  x;

  for  (x = 0;  x < NumOfFeatures ();  x++)
    bitStr.Set (featureNums[x]);
}  /* ToBitString */



kkuint16*  FeatureNumList2::CreateFeatureNumArray ()  const
{
  kkuint16*  newList = new kkuint16[numOfFeatures];
  for  (kkint32 x = 0;  x < numOfFeatures;  x++)
    newList[x] = featureNums[x];
  return  newList;
}  /* CreateFeatureNumArray */



bool  FeatureNumList2::AllFeaturesSelected (FileDescPtr  fileDesc)  const
{
  if  (numOfFeatures >= (kkint32)fileDesc->NumOfFields ())
    return true;
  return false;
}  /* AllFeaturesSelected */



void  FeatureNumList2::UnSet ()
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



void  FeatureNumList2::UnSet (kkuint16  featureNum)
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



void  FeatureNumList2::AddFeature (kkuint16  featureNum)
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
      ++x
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



FeatureNumList2   FeatureNumList2::AllFeatures (FileDescPtr  _fileDesc)
{
  bool  valid;

  kkuint16  maxFeatureNum = _fileDesc->NumOfFields () - 1;
  FeatureNumList2  features (maxFeatureNum);

  const AttributeTypeVector&   attributeTypes = _fileDesc->AttributeVector ();
  for  (kkuint16 fn = x;  x <= maxFeatureNum;  ++x)
  {
    if  (attributeTypes[fn] != IgnoreAttribute)
      features.AddFeature (x);
  }

  return  features;
}  /* AllFeatures */





void   FeatureNumList2::SetAllFeatures (FileDescPtr  fileDesc)
{
  for  (kkuint16 x = 0; x <= maxFeatureNum;  ++x)
  {
    if  (fileDesc->Type (x) != IgnoreAttribute)
      AddFeature (kkuint16 (x));
  }
  return;
}  /* SetAllFeatures */




/**
 *brief  Returns true if the FeatureNumList2 'z' is a subset of myself.
 */
bool  FeatureNumList2::IsSubSet (const FeatureNumList2&  z)
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



bool  FeatureNumList2::InList (kkuint16 _featureNum)  const
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



bool  FeatureNumList2::Test (kkuint16 _featureNum)  const
{
  return InList (_featureNum);
}  /* Test */



kkuint16  FeatureNumList2::operator[] (kkint32  _idx)  const
{
  if  (_idx >= numOfFeatures)
  {
    KKStr  errMsg (100);
    errMsg << "FeatureNumList2::operator[]  ***ERROR***   Invalid Index[" << _idx << "] requested.";
    cerr << endl << errMsg << endl << endl;
    throw  KKException (errMsg);
  }
  else
  {
    return featureNums[_idx];
  }
}



KKStr  FeatureNumList2::ToString ()  const
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




KKStr   FeatureNumList2::ToHexString ()  const
{
  BitString  bs (fileDesc->NumOfFields ());
  ToBitString (bs);
  return  bs.HexStr ();
}  /* ToHexString */



FeatureNumListPtr  FeatureNumList2::ExtractFeatureNumsFromStr (KKStr  _featureListStr,
                                                               bool&  _valid
                                                              )
                                                              const
{
  _valid = true;

  FeatureNumListPtr  results = new FeatureNumList2 (2);

  KKStr     field;
  kkuint16  featureNum;

  _featureListStr.Upper ();

  numOfFeatures = 0;

  if  (_featureListStr == "NONE")
  {
    return results;
  }

  field = _featureListStr.ExtractToken (", \n\r\t");
  while  (!field.Empty ())
  {
    _featureListStr.TrimLeft ();
   
    kkint32 dashPos = field.LocateCharacter ('-');
    if  (dashPos < 0)
    {
      // This is not a range

      featureNum = kkuint16 (atoi (field.Str ()));
      {
        if  (featureNum < 0)
        {
          _valid = false;
          return  results;
        }
        else
        {
          bool  alreadyInList = results->InList (featureNum);
          if  (!alreadyInList)
            results->AddFeature (featureNum);
        }
      }
    }
    else
    {
      // We are looking at a range
      kkuint16 startFeatureNum = kkuint16 (field.ExtractTokenInt (" -"));
      kkuint16 endFeatureNum   = kkuint16 (field.ExtractTokenInt (" -"));

      if  (startFeatureNum > endFeatureNum)
      {
         _valid = false;
      }
      else
      {
        for  (featureNum = startFeatureNum; featureNum <= endFeatureNum;  ++featureNum)
        {
          bool  alreadyInList = results->InList (featureNum);
          if  (!alreadyInList)
            results->AddFeature (featureNum);
        }
      }
    }
    field = _featureListStr.ExtractToken (", \n\r\t");
  }
  return  results;
}  /* ExtractFeatureNumsFromStr */



void  FeatureNumList2::Load (const KKStr&  _fileName,
                             bool&         _successful,
                             RunLog&       _log
                            )
{
  _log.Level (20) << "FeatureNumList2::Load - File[" << _fileName << "]." << endl;

  FILE*  inputFile = osFOPEN (_fileName.Str (), "r");
  if  (!inputFile)
  {
    _log.Level (-1) << "FeatureNumList2::Load      *** ERROR ***" << endl;
    _log.Level (-1) << "                 Could Not Open File[" << _fileName << "]." << endl;
    _successful = false;
    return;
  }

  kkint32 fileDescNumOfFields  = 0;
  char  buff [102400];

  if  (fgets (buff, sizeof (buff), inputFile))
  {
    KKStr  firstLine (buff);
    fileDescNumOfFields = atoi (firstLine.Str ());
  }
  else
  {
    _log.Level (-1) << endl
                    << "FeatureNumList2::Load      *** ERROR ***" << endl
                    << "                Missing Data from File" << endl
                    << endl;
    _successful = false;
    fclose (inputFile);
    return;
  }

  if  (fgets (buff, sizeof (buff), inputFile))
  {
    _log.Level (50) << "Load - FeatureList = [" << buff << "]." << endl;
    bool  valid;
    ExtractFeatureNumsFromStr (buff, valid);
    if  (!valid)
      _successful = false;
  }
  else
  {
    _log.Level (-1) << "FeatureNumList2::Load      *** ERROR ***" << endl;
    _log.Level (-1) << "                No Data in File[" << _fileName << "]." << endl;
    _successful = false;
  }

  fclose (inputFile);
  _successful = true;
}  /* Load */



void  FeatureNumList2::Save (const KKStr&  _fileName,
                            bool&         _successful,
                            RunLog&       _log
                           )
{
  _log.Level (20) << "FeatureNumList2::Save - File["
                  << _fileName << "]."
                  << endl;

  _successful = true;

  ofstream outFile (_fileName.Str ());

  outFile << fileDesc->NumOfFields () << endl;
  outFile << ToString ()              << endl;

  outFile.close ();
}  /* Save */




void  FeatureNumList2::Save (ostream&  o)
{
  o << "<FeatureNumList2>" << "\t"
    << "NumOfFeatures"    << "\t"  << numOfFeatures
    << "FeatureNums"      << "\t"  << ToString ()
    << "</FeatureNumList2>";
}  /* Save */



FeatureNumList2&  FeatureNumList2::operator= (const FeatureNumList2&  _features)
{
  if  (featureNums)
    delete  [] featureNums;

  fileDesc      = _features.fileDesc;
  numOfFeatures = _features.NumOfFeatures ();

  featureNums = new kkuint16[numOfFeatures];
  featureNumsAllocatedSize = numOfFeatures;

  for  (kkint32 fn = 0;  fn < numOfFeatures;  fn++)
    featureNums[fn] = _features[fn];

  return  *this;
}  /* operator= */



FeatureNumList2&  FeatureNumList2::operator=  (const FeatureNumList2Ptr  _features)
{
  *this = *_features;
  return  *this;
}



kkint32  FeatureNumList2::Compare (const FeatureNumList2&  _features)  const
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



/**  @brief  Indicates if the two FeatureNumList2 instances have the same features selected. */
bool  FeatureNumList2::operator== (const FeatureNumList2&  _features)  const
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
bool  FeatureNumList2::operator< (const FeatureNumList2&  _features)  const
{
  return  Compare (_features) < 0;
}  /* operator< */



/**
 * @brief  Indicates if the Left FeatureNumLiost instances is greater than the right one.
 * @see Compare
 */
bool  FeatureNumList2::operator> (const FeatureNumList2&  _features)  const
{
  return  Compare (_features) > 0;
}  /* operator> */



namespace  KKMLL
{
  ostream& operator<< (      ostream&          os, 
                       const FeatureNumList2&   features
                      )
  {
    os << features.ToString ();
    return  os;
  }


  ostream& operator<< (      ostream&            os, 
                       const FeatureNumList2Ptr&  features
                      )
  {
    os << features->ToString ();
    return  os;
  }
}



/**
 *@brief Performs a logical 'AND' operation on the two FeatureNumList2 instances.
 *@details Will return a new FeatureNumList2 instance that will consist of a list of features that are selected in both the left and right FeatureNumList2
 * instances.  Both FeatureNumList2 objects must be referencing the same FileDesc instance otherwise an exception will be thrown.
 */
FeatureNumList2  FeatureNumList2::operator*  (const FeatureNumList2&  rightSide)  const
{
  if  (fileDesc != rightSide.FileDesc ())
  {
    KKStr errMsg = "FeatureNumList2::operator*  ***ERROR***   Incompatible FileDesc's";
    errMsg << endl
           << "      The associated FileDesc instances are not the same.";
    cerr << endl << endl << errMsg << endl <<endl;
    throw KKException (errMsg);
  }

  FeatureNumList2  result (fileDesc);
  result.AllocateArraySize (numOfFeatures);

  kkint32  l = 0;
  kkint32  r = 0;

  while  ((l < numOfFeatures)  &&  (r < rightSide.numOfFeatures))
  {
    if  (featureNums[l] < rightSide.featureNums[r])
    {
      l++;
    }

    else if  (featureNums[l] > rightSide.featureNums[r])
    {
      r++;
    }

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
 * @brief Performs a logical 'OR'  operation on the two FeatureNumList2 instances.
 * @details Will return a new FeatureNumList2 instance that will consist of a list of features that are in either
 * left and right FeatureNumList2 instances.  Both FeatureNumList2 objects must be referencing the same FileDesc instance
 * otherwise an exception will be thrown.
 */
FeatureNumList2  FeatureNumList2::operator+ (const FeatureNumList2&  rightSide) const
{
  FeatureNumList2  result (rightSide);

  kkint32  l = 0;
  for  (l = 0;  l < numOfFeatures; l++)
    result.AddFeature (featureNums[l]);

  return  result;
}  /* operator+ */



/**
 * @brief Adds the features that are selected in the right FeatureNumList2 instance to the left instance.
 * @details Both FeatureNumList2 objects must be referencing the same FileDesc instance otherwise an exception will be thrown.
 */
FeatureNumList2&  FeatureNumList2::operator+= (const FeatureNumList2&  rightSide)
{
  kkint32    newFeatureNumsAllocatedSize = Min (numOfFeatures + rightSide.numOfFeatures, (kkint32)fileDesc->NumOfFields ());
  kkuint16*  newFeatureNums = new kkuint16[newFeatureNumsAllocatedSize];
  kkint32    newNumOfFeatures = 0;

  kkuint16* leftFeatureNums    = featureNums;
  kkint32   leftNumOfFeatures  = numOfFeatures;
  kkuint16* rightFeatureNums   = rightSide.featureNums;
  kkint32   rightNumOfFeatures = rightSide.numOfFeatures;

  kkint32  l = 0;
  kkint32  r = 0;

  while  ((l < leftNumOfFeatures)  &&  (r < rightNumOfFeatures))
  {
    if  (leftFeatureNums[l] < rightFeatureNums[r])
    {
      newFeatureNums[newNumOfFeatures] = leftFeatureNums[l];
      newNumOfFeatures++;
      l++;
    }

    else if  (leftFeatureNums[l] > rightFeatureNums[r])
    {
      newFeatureNums[newNumOfFeatures] = rightFeatureNums[r];
      newNumOfFeatures++;
      r++;
    }

    else
    {
      newFeatureNums[newNumOfFeatures] = rightFeatureNums[r];
      newNumOfFeatures++;
      l++;
      r++;
    }
  }

  while  (l < leftNumOfFeatures)
  {
    newFeatureNums[newNumOfFeatures] = leftFeatureNums[l];
    newNumOfFeatures++;
    l++;
  }

  while  (r < rightNumOfFeatures)
  {
    newFeatureNums[newNumOfFeatures] = rightFeatureNums[r];
    newNumOfFeatures++;
    r++;
  }

  delete  [] featureNums;
  featureNums = newFeatureNums;
  featureNumsAllocatedSize = newFeatureNumsAllocatedSize;
  numOfFeatures = newNumOfFeatures;

  return  *this;
}  /* operator+= */




/** @brief Adds the feature 'featureNum' to the selected list of features. */
FeatureNumList2&  FeatureNumList2::operator+= (kkuint16  featureNum)
{
  AddFeature (featureNum);
  return *this;
}  /* operator+= */





/** @brief Returns a new FeatureNumList2 instance that will consists of the left FeatureNumList2 instance with 'rightSide' feature added in. */
FeatureNumList2  FeatureNumList2::operator+ (kkuint16  rightSide) const
{
  if  (kkint32 (rightSide) >= fileDesc->NumOfFields ())
  {
    KKStr errMsg = "FeatureNumList2::operator+  ***ERROR***";
    errMsg <<" Feature[" << rightSide << "]  is too large.";
    cerr << endl << endl << errMsg << endl <<endl;
    throw KKException (errMsg);
  }
  
  FeatureNumList2  result (*this);
  result.AddFeature (rightSide);
  return  result;
}  /* operator+ */




/** @brief Returns a new FeatureNumList2 instance that will consists of the left FeatureNumList2 instance with 'rightSide' removed from it. */
FeatureNumList2  FeatureNumList2::operator-  (kkuint16  rightSide) const
{
  if  (rightSide >= fileDesc->NumOfFields ())
  {
    KKStr errMsg = "FeatureNumList2::operator-  ***ERROR***";
    errMsg <<" Feature[" << rightSide << "]  is too large.";
    cerr << endl << endl << errMsg << endl <<endl;
    throw KKException (errMsg);
  }

  FeatureNumList2  result (*this);
  result.UnSet (rightSide);

  return  result;
}  /* operator- */



/**
 * @brief Returns a new FeatureNumList2 instance that consists of the left side instance with the
 *  selected features in the right side removed.
 * @details Both FeatureNumList2 objects must be referencing the same FileDesc instance otherwise
 *  an exception will be thrown.
 */
FeatureNumList2  FeatureNumList2::operator- (const FeatureNumList2&  rightSide) const
{
  if  (fileDesc != rightSide.FileDesc ())
  {
    KKStr errMsg = "FeatureNumList2::operator-  ***ERROR***   Incompatible FileDesc's";
    errMsg << endl
           << "      The associated FileDesc instances are not the same.";
    cerr << endl << endl << errMsg << endl <<endl;
    throw KKException (errMsg);
  }

  FeatureNumList2  result (*this);
  kkint32  x;
  for  (x = 0;  x < rightSide.NumOfFeatures ();  x++)
    result.UnSet (rightSide[x]);

  return  result;
}  /* operator- */




/** @brief removes the feature specified on the right side from the FeatureNumList2 on the left side. */
FeatureNumList2&  FeatureNumList2::operator-= (kkuint16  rightSide)
{
  UnSet (rightSide);
  return  *this;
}  /* operator-= */



FeatureNumList2Ptr   FeatureNumList2::RandomlySelectFeatures (kkint32  numToKeep)  const
{
  if  (numToKeep > numOfFeatures)
  {
    cerr << endl
         << endl
         << "FeatureNumList2::RandomlySelectFeatures    *** ERROR ***" << endl
         << endl
         << "NumToKeep[" << numToKeep << "]  Is greater than  NumOfFeatures[" << numOfFeatures << "]" << endl
         << endl
         << endl;
    numToKeep = numOfFeatures;
  }

  FeatureNumList2Ptr  randomlySelectedFeatures = new FeatureNumList2 (fileDesc);

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




AttributeType  FeatureNumList2::FeatureAttributeType (kkint32 idx)  const
{
  if  ((idx < 0)  ||  (idx >= numOfFeatures))
  {
    cerr << endl << endl
         << "FeatureNumList2::AttributeType       *** ERROR ***"  << endl
         << endl
         << "                Invalid Index[" << idx << "]  Valid Range (0.." << (numOfFeatures - 1) << ")" << endl
         << endl;

    return  NULLAttribute;
  }

  if  (!fileDesc)
  {
    KKStr errMsg = "FeatureNumList2::AttributeType   ***ERROR***  'fileDesc == NULL'   There is a major programing flaw.";
    cerr << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  return  fileDesc->Type (featureNums[idx]);
}  /* FeatureAttributeType */



FeatureNumList2  FeatureNumList2::Complement ()  const
{
  kkuint16  x;

  FeatureNumList2  result;

  for  (x = 0;  x < kkuint16 (fileDesc->NumOfFields ());  x++)
  {
    if  (!InList (x))
      result.AddFeature (x);
  }

  return  result;
}  /* Complement */

