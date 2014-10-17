
#include "FirstIncludes.h"

#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include "MemoryDebug.h"
using namespace std;


#include "DateTime.h"
#include "KKBaseTypes.h"
#include "KKQueue.h"
#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace KKB;


#include "Attribute.h"
#include "FeatureNumList.h"
#include "MLClass.h"
using namespace KKMachineLearning;



Attribute::Attribute (const KKStr&   _name,
                      AttributeType  _type,
                      int32          _fieldNum
                     ):
    fieldNum           (_fieldNum),
    name               (_name),
    nameUpper          (_name),
    nominalValues      (NULL),
    nominalValuesUpper (NULL),
    type               (_type)

{
  nameUpper.Upper ();
  if  ((type == NominalAttribute)  ||  (type == SymbolicAttribute))
  {
    nominalValues      = new KKStrList (true);
    nominalValuesUpper = new KKStrList (true);
  }
}


Attribute::Attribute (const Attribute&  a):
    fieldNum           (a.fieldNum),
    name               (a.name),
    nameUpper          (a.nameUpper),
    nominalValues      (NULL),
    nominalValuesUpper (NULL),
    type               (a.type)

{
  if  ((type == NominalAttribute)  ||  (type == SymbolicAttribute))
  {
    nominalValues      = new KKStrList (true);
    nominalValuesUpper = new KKStrList (true);

    int32  x;
    for  (x = 0;  x < a.nominalValues->QueueSize ();  x++)
    {
      nominalValues->PushOnBack      (new KKStr (a.nominalValues->IdxToPtr      (x)));
      nominalValuesUpper->PushOnBack (new KKStr (a.nominalValuesUpper->IdxToPtr (x)));
    }
  }
}




Attribute::~Attribute ()
{
  delete  nominalValues;
  delete  nominalValuesUpper;
}


int32  Attribute::MemoryConsumedEstimated ()  const
{
  int32  memoryConsumedEstimated = sizeof (Attribute)  + 
    name.MemoryConsumedEstimated ()                  +
    nameUpper.MemoryConsumedEstimated ();

  if  (nominalValues)
    memoryConsumedEstimated += nominalValues->MemoryConsumedEstimated ();

  if  (nominalValuesUpper)
    memoryConsumedEstimated += nominalValuesUpper->MemoryConsumedEstimated ();

  return  memoryConsumedEstimated;
}



void  Attribute::ValidateNominalType (const KKStr&  funcName)  const
{
  if  ((type != NominalAttribute)  &&  (type != SymbolicAttribute))
  {
    cerr << endl
         << "***  ERROR  ***             Attribute::" << funcName << endl
         << endl
         << "     Must be a Nominal Type to perform this operation."
         << endl;
    osWaitForEnter ();
    exit (-1);
  }

  return;
}  /* ValidateNuminalType */



void  Attribute::AddANominalValue (const KKStr&  nominalValue,
                                   bool&          alreadyExists
                                  )
{
  ValidateNominalType ("AddANominalValue");

  int32  code = GetNominalCode (nominalValue);
  if  (code >= 0)
  {
    alreadyExists = true;
    return;
  }

  alreadyExists = false;

  KKStrPtr  nominalValueUpper = new KKStr (nominalValue);
  nominalValueUpper->Upper ();
  nominalValues->PushOnBack (new KKStr (nominalValue));
  nominalValuesUpper->PushOnBack (nominalValueUpper);
}  /* AddANominalValue */




const
KKStr&  Attribute::GetNominalValue (int32 code)  const
{
  ValidateNominalType ("GetNominalValue");

  static KKStr missingDataValue = "?";

  if  (code == -1)
  {
    // Missing Value Flag.
    return missingDataValue;
  }

  if  (!nominalValues)
    return  KKStr::EmptyStr ();

  if  ((code < 0)  ||  (code >= nominalValues->QueueSize ()))
  {
    // Out of bounds.  This should never happen,  if it does,  there is a major logic
    // error in the program that NEEDS to be fixed NOW NOW NOW
    cerr << endl
         << "***  ERROR  ***             Attribute::GetNominalValue" << endl
         << "      AttributeName[" << name                        << "]" << endl
         << "      Cardinality  [" << nominalValues->QueueSize () << "]" << endl
         << "      Code         [" << code                        << "]" << endl
         << endl;
    osWaitForEnter ();
    exit (-1);
  }
  
  return  *(nominalValues->IdxToPtr (code));
}  /* GetNominalValue */



int32  Attribute::Cardinality ()
{
  if  ((type == NominalAttribute)  ||  (type == SymbolicAttribute))
    return  nominalValues->QueueSize ();
  else
    return 999999999;
}  /* Cardinality */



int32  Attribute::GetNominalCode  (const KKStr&  nominalValue)  const
{
  ValidateNominalType ("GetNominalCode");

  KKStr  nominalValueUpper = nominalValue.ToUpper ();
  
  int32  code = 0;
  while  (code < nominalValuesUpper->QueueSize ())
  {
    if  (nominalValueUpper == *(nominalValuesUpper->IdxToPtr (code)))
      return code;
    code++;
  }

  return -1;
}  /* GetNominalCode */



bool  Attribute::operator== (const Attribute&  rightSide)  const
{
  if  ((nameUpper != rightSide.nameUpper)  ||  (Type ()   != rightSide.Type ()))
    return false;

  if  ((type == NominalAttribute)  ||  (type == SymbolicAttribute))
  {
    // Lets make sure that the nominal values are equal.  Not Case sensitive.
    if  ((*nominalValuesUpper) != (*rightSide.nominalValuesUpper))
      return  false;
  }

  return true;
}  /* operator== */



bool  Attribute::operator!= (const Attribute&  rightSide)  const
{
  return  !(*this == rightSide);
}  /* operator== */





Attribute&  Attribute::operator= (const Attribute&  right)
{
  fieldNum           = right.fieldNum;
  name               = right.name;
  nameUpper          = right.nameUpper;

  if  (right.nominalValues  &&  nominalValuesUpper)
  {
    nominalValues      = right.nominalValues->DuplicateListAndContents ();
    nominalValuesUpper = right.nominalValuesUpper->DuplicateListAndContents ();
  }
  else
  {
    nominalValues      = NULL;
    nominalValuesUpper = NULL;
  }

  type = right.type;
  return  *this;
}  /* operator= */




KKStr  Attribute::TypeStr () const
{
  return  AttributeTypeToStr (type);
}  /* TypeStr */




AttributeList::AttributeList (bool _owner):
  KKQueue<Attribute> (_owner)
{
}



AttributeList::~AttributeList ()
{
}


int32  AttributeList::MemoryConsumedEstimated ()  const
{
  int32  memoryConsumedEstimated = sizeof (AttributeList) + nameIndex.size ();
  
  {
    std::map<KKStr, AttributePtr>::const_iterator  idx;
    for  (idx = nameIndex.begin ();  idx != nameIndex.end ();  ++idx)
      memoryConsumedEstimated += (sizeof (AttributePtr) + idx->first.MemoryConsumedEstimated ());
  }

  {
    AttributeList::const_iterator  idx;
    for  (idx = begin ();  idx != end ();  ++idx)
    {
      memoryConsumedEstimated += (*idx)->MemoryConsumedEstimated ();
    }
  }

  return  memoryConsumedEstimated;
}  /* MemoryConsumedEstimated */



const
AttributePtr  AttributeList::LookUpByName (const KKStr&  name)  const
{
  KKStr  nameUpper (name);
  nameUpper.Upper ();

  map<KKStr, AttributePtr>::const_iterator p;
  p = nameIndex.find (nameUpper);
  if (p == nameIndex.end ())
    return NULL;
  return p->second;
/*

  AttributePtr  attribute = NULL;
  int32  idx = 0;

  KKStr  nameUpper = name.ToUpper ();

  while  (idx < QueueSize ())
  {
    attribute = IdxToPtr (idx);
    if  (attribute->NameUpper () == nameUpper)
      return attribute;
    idx++;
  }
  return NULL;
*/
}  /* LookUpByName */



void  AttributeList::PushOnBack   (AttributePtr  attribute)
{
  AddToNameIndex (attribute);
  KKQueue<Attribute>::PushOnBack (attribute);
}


void  AttributeList::PushOnFront  (AttributePtr  attribute)
{
  AddToNameIndex (attribute);
  KKQueue<Attribute>::PushOnFront (attribute);
}



void  AttributeList::AddToNameIndex (AttributePtr  attribute)
{
  nameIndex.insert (pair<KKStr, AttributePtr> (attribute->Name ().ToUpper (), attribute));
}  /* AddToNameIndex */



AttributeTypeVectorPtr  AttributeList::CreateAttributeTypeVector ()  const
{
  AttributeTypeVectorPtr  v = new AttributeTypeVector ();

  const_iterator idx;
  for  (idx = begin ();  idx != end ();  idx++)
  {
    v->push_back ((*idx)->Type ());
  }

  return  v;
}  /* CreateAttributeTypeVector */






KKStr  KKMachineLearning::AttributeTypeToStr (AttributeType  type)
{
  static const char* AttributeTypeStrings[] = 
    {
      "Ignore",
      "Numeric",
      "Nominal",
      "Ordinal",
      "NULL"
    };

  if  ((type < 0)  ||  (type >= NULLAttribute))
    return  "";

  return AttributeTypeStrings[type];
}  /* TypeStr */




bool  KKMachineLearning::operator== (const AttributeList&  left,
                                     const AttributeList&  right
                                    )
{ 
  if  (left.size () != right.size ())
    return false;

  AttributeList::const_iterator idxL;
  AttributeList::const_iterator idxR;

  idxL = left.begin ();
  idxR = right.begin ();

  while  (idxL != left.end ())
  {
    AttributePtr leftAttribute  = *idxL;
    AttributePtr rightAttribute = *idxR;

    if  ((*leftAttribute) != (*rightAttribute))
      return false;

    idxL++;
    idxR++;
  }

  return true;
}  /* operator== */



bool  KKMachineLearning::operator!= (const AttributeList&  left,
                                     const AttributeList&  right
                                    )
{ 
  return  !(left == right);
}  /* operator!= */





