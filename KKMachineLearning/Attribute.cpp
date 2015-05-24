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
#include "GlobalGoalKeeper.h"
#include "KKBaseTypes.h"
#include "KKException.h"
#include "KKQueue.h"
#include "KKStr.h"
#include "KKStrParser.h"
#include "OSservices.h"
#include "RunLog.h"
#include "XmlStream.h"
using namespace KKB;

#include "Attribute.h"
#include "KKMLLTypes.h"
#include "FeatureNumList.h"
#include "MLClass.h"
using namespace KKMLL;


Attribute::Attribute ():
    fieldNum           (-1),
    name               (),
    nameUpper          (),
    nominalValuesUpper (NULL),
    type               (AttributeType::NULLAttribute)

{
}


Attribute::Attribute (const KKStr&   _name,
                      AttributeType  _type,
                      kkint32        _fieldNum
                     ):
    fieldNum           (_fieldNum),
    name               (_name),
    nameUpper          (_name),
    nominalValuesUpper (NULL),
    type               (_type)

{
  nameUpper.Upper ();
  if  ((type == AttributeType::Nominal)  ||  
       (type == AttributeType::Symbolic)
      )
  {
    nominalValuesUpper = new KKStrListIndexed (true,   // true == nominalValuesUpper will own its contents.
                                               false   // false = Not Case Sensitive.
                                              );
  }
}


Attribute::Attribute (const Attribute&  a):
    fieldNum           (a.fieldNum),
    name               (a.name),
    nameUpper          (a.nameUpper),
    nominalValuesUpper (NULL),
    type               (a.type)

{
  if  ((type == AttributeType::Nominal)  ||  
       (type == AttributeType::Symbolic)
      )
  {
    nominalValuesUpper = new KKStrListIndexed (*(a.nominalValuesUpper));
  }
}




Attribute::~Attribute ()
{
  delete  nominalValuesUpper;
}


kkint32  Attribute::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = sizeof (Attribute)  + 
           name.MemoryConsumedEstimated ()               +
           nameUpper.MemoryConsumedEstimated ();

  if  (nominalValuesUpper)
    memoryConsumedEstimated += nominalValuesUpper->MemoryConsumedEstimated ();

  return  memoryConsumedEstimated;
}



void  Attribute::ValidateNominalType (const KKStr&  funcName)  const
{
  if  ((type != AttributeType::Nominal)  &&  
       (type != AttributeType::Symbolic)
      )
  {
    KKStr  msg (80);
    msg <<  "Attribute::ValidateNominalType   Attribute[" << funcName << "] must be a Nominal or Symbolic Type to perform this operation.";
    cerr << std::endl << msg << std::endl << std::endl;
    throw  KKException (msg);
  }

  return;
}  /* ValidateNuminalType */



void  Attribute::AddANominalValue (const KKStr&  nominalValue,
                                   bool&         alreadyExists
                                  )
{
  ValidateNominalType ("AddANominalValue");
  kkint32  code = GetNominalCode (nominalValue);
  if  (code >= 0)
  {
    alreadyExists = true;
    return;
  }

  alreadyExists = false;
  nominalValuesUpper->Add (new KKStr (nominalValue));
}  /* AddANominalValue */




const
KKStr&  Attribute::GetNominalValue (kkint32 code)  const
{
  ValidateNominalType ("GetNominalValue");

  static KKStr missingDataValue = "?";

  if  (code == -1)
  {
    // Missing Value Flag.
    return missingDataValue;
  }

  if  (!nominalValuesUpper)
    return  KKStr::EmptyStr ();

  KKStrConstPtr  result = nominalValuesUpper->LookUp (code);
  if  (result == NULL)
  {
    cerr << endl << endl
      << "Attribute::GetNominalValue   ***ERROR***    Code[" << code << "]  Does not exist." << endl
      << "                    Attribute::Name[" << name << "]"  << endl
      << endl;
    return  KKStr::EmptyStr ();
  }

  return  *result;
}  /* GetNominalValue */



kkint32 Attribute::Cardinality ()  const
{
  if  ((type == AttributeType::Nominal)  ||  
       (type == AttributeType::Symbolic)
      )
    return  nominalValuesUpper->size ();
  else
    return 999999999;
}  /* Cardinality */



kkint32  Attribute::GetNominalCode  (const KKStr&  nominalValue)  const
{
  ValidateNominalType ("GetNominalCode");
  kkint32  code = nominalValuesUpper->LookUp (nominalValue);
  return code;
}  /* GetNominalCode */



bool  Attribute::operator== (const Attribute&  rightSide)  const
{
  if  ((nameUpper != rightSide.nameUpper)  ||  (Type ()   != rightSide.Type ()))
    return false;

  if  ((type == AttributeType::Nominal)  ||
       (type == AttributeType::Symbolic)
      )
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
  fieldNum  = right.fieldNum;
  name      = right.name;
  nameUpper = right.nameUpper;

  delete  nominalValuesUpper;
  if  (nominalValuesUpper)
    nominalValuesUpper = new KKStrListIndexed (*right.nominalValuesUpper);
  else
    nominalValuesUpper = NULL;

  type = right.type;
  return  *this;
}  /* operator= */




KKStr  Attribute::TypeStr () const
{
  return  AttributeTypeToStr (type);
}  /* TypeStr */




void  Attribute::WriteXML (const KKStr&  varName,
                           ostream&      o
                          )  const
{
  XmlTag::TagTypes  startTagType = XmlTag::TagTypes::tagEmpty;
  if  ((type == KKMLL::AttributeType::Nominal) ||
       (type == KKMLL::AttributeType::Symbolic)
      )
    startTagType  = XmlTag::TagTypes::tagStart;

  XmlTag  startTag ("Attribute", startTagType);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.AddAtribute ("Name",     name);
  startTag.AddAtribute ("Type",     TypeStr ());
  startTag.AddAtribute ("FieldNum", fieldNum);
  startTag.WriteXML (o);
  o << endl;

  if  (startTagType == XmlTag::TagTypes::tagStart)
  {
    VectorKKStr  nominalValues;
    for  (kkint32  nominalIdx = 0;  nominalIdx < Cardinality ();  ++nominalIdx)
      nominalValues.push_back (GetNominalValue (nominalIdx));
    nominalValues.WriteXML ("NominalValues", o);
    XmlTag  endTag ("Attribute", XmlTag::TagTypes::tagEnd);
    endTag.WriteXML (o);
    o << endl;
  }
}  /* WriteXML */




void  Attribute::ReadXML (XmlStream&      s,
                          XmlTagConstPtr  tag,
                          RunLog&         log
                         )
{
  delete  nominalValuesUpper;
  nominalValuesUpper = NULL;
  type = AttributeTypeFromStr (tag->AttributeValueKKStr ("Type"));
  fieldNum = tag->AttributeValueInt32 ("FieldNum");
  name = tag->AttributeValueKKStr ("Name");
  nameUpper = name.ToUpper ();

  XmlTokenPtr t = s.GetNextToken (log);
  while  (t)
  {
    if  ((t->SectionName ().EqualIgnoreCase ("NominalValues"))  &&  (typeid(*t) == typeid(XmlElementVectorKKStr)))
    {
      delete  nominalValuesUpper;
      nominalValuesUpper = new KKStrListIndexed (true,   // true == nominalValuesUpper will own its contents.
                                                 false   // false = Not Case Sensitive.
                                                );
      XmlElementVectorKKStrPtr v = dynamic_cast<XmlElementVectorKKStrPtr>(t);
      if  ((v != NULL)  &&  (v->Value () != NULL))
      {
        VectorKKStr::const_iterator  idx;
        bool  alreadyEixists = false;
        for (idx = v->Value ()->begin ();  idx != v->Value ()->end ();  ++idx)
          AddANominalValue (*idx, alreadyEixists);
      }
    }
  }
}





AttributeList::AttributeList ():
  KKQueue<Attribute> (true)
{
}





AttributeList::AttributeList (bool _owner):
  KKQueue<Attribute> (_owner)
{
}



AttributeList::~AttributeList ()
{
}


kkint32  AttributeList::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = sizeof (AttributeList) + nameIndex.size ();
  
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




const KKStr&  KKMLL::AttributeTypeToStr (AttributeType  type)
{
  static vector<KKStr> AttributeTypeStrings = 
    {
      "Null",
      "Ignore",
      "Numeric",
      "Nominal",
      "Ordinal"
    };

  if  ((type < (AttributeType)0)  ||  ((kkuint32)type >= AttributeTypeStrings.size ()))
    return  KKStr::EmptyStr ();

  return AttributeTypeStrings[(int)type];
}  /* TypeStr */




AttributeType  KKMLL::AttributeTypeFromStr (const KKStr&  s)
{
  if  (s.EqualIgnoreCase ("Ignore"))
    return  AttributeType::Ignore;

  else if  (s.EqualIgnoreCase ("Numeric"))
    return  AttributeType::Numeric;

  else if  (s.EqualIgnoreCase ("Nominal"))
    return AttributeType::Nominal;

  else if  (s.EqualIgnoreCase ("Ordinal"))
    return  AttributeType::Ordinal;

  else if  (s.EqualIgnoreCase ("Symbolic"))
    return  AttributeType::Symbolic;

  else
    return  AttributeType::NULLAttribute;
}




bool  AttributeList::operator== (const AttributeList&  right)  const
{ 
  if  (size () != right.size ())
    return false;

  AttributeList::const_iterator idxL;
  AttributeList::const_iterator idxR;

  idxL = begin ();
  idxR = right.begin ();

  while  (idxL != end ())
  {
    AttributePtr leftAttribute  = *idxL;
    AttributePtr rightAttribute = *idxR;

    if  ((*leftAttribute) != (*rightAttribute))
      return false;

    ++idxL;
    ++idxR;
  }

  return true;
}  /* operator== */



bool  AttributeList::operator!= (const AttributeList&  right)  const
{ 
  return  !(*this == right);
}  /* operator!= */



void  AttributeList::WriteXML (const KKStr&  varName,
                               ostream&      o
                              )  const
{
  XmlTag  startTag ("AttributeList",  XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);
  o << endl;

  AttributeList::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  ++idx)
  {
    AttributePtr  attribute = *idx;
    attribute->WriteXML ("", o);
  }
  XmlTag  endTag ("AttributeList", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}  /* WriteXML */






void  AttributeList::ReadXML (XmlStream&      s,
                              XmlTagConstPtr  tag,
                              RunLog&         log
                             )
{
  DeleteContents ();
  Owner (true);
  XmlTokenPtr  t = s.GetNextToken (log);
  while  (t)
  {
    if  (typeid (*t) == typeid(XmlElementAttribute))
    {
      XmlElementAttributePtr  attrToken = dynamic_cast<XmlElementAttributePtr> (t);
      if  (attrToken->Value ())
        PushOnBack (attrToken->TakeOwnership ());
    }
    delete  t;
    t = s.GetNextToken (log);
  }
}


AttributeTypeVector:: AttributeTypeVector ():
   vector<AttributeType> ()
{
}


AttributeTypeVector::AttributeTypeVector (kkuint32       initialSize,  
                                          AttributeType  initialValue
                                         ):
   vector<AttributeType> (initialSize, initialValue)
{
}


AttributeType  KKMLL::operator++(AttributeType  zed)
{
  return  (AttributeType)((int)zed + 1);
}  /* operator++ */


void  AttributeTypeVector::WriteXML (const KKStr&  varName,
                                     ostream&      o
                                    )  const
{
  XmlTag  tagStart ("AttributeTypeVector", XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    tagStart.AddAtribute ("VarName", varName);
  tagStart.AddAtribute ("Size", (kkint32)size ());
  tagStart.WriteXML (o);
  o << endl;

  o << "CodeTable";
  for  (AttributeType  zed = AttributeType::NULLAttribute;  zed <= AttributeType::Symbolic;  ++zed)
    o << "\t" << AttributeTypeToStr (zed);
  o << endl;

  o << "CodedAttributeValues";
  for  (auto  idx: *this)
    o << "\t" << (int)idx;
  o << endl;

  XmlTag  tagEnd ("AttributeTypeVector", XmlTag::TagTypes::tagEnd);
  tagEnd.WriteXML (o);
  o << endl;
}


void  AttributeTypeVector::ReadXML (XmlStream&      s,
                                    XmlTagConstPtr  tag,
                                    RunLog&         log
                                   )
{
  clear ();

  kkuint32  expectedLen = (kkuint32)tag->AttributeValueInt32 ("Size");
  AttributeTypeVector decodeTable;

  XmlTokenPtr t = s.GetNextToken (log);
  while  (t)
  {
    if  (typeid (*t) == typeid (XmlContent))
    {
      KKStrParser p (*(dynamic_cast<XmlContentPtr> (t)->Content ()));
      p.TrimWhiteSpace (" ");
      KKStr  lineName =p.GetNextToken ();
      if  (lineName.EqualIgnoreCase ("CodeTable"))
      {
        KKStr  fieldName = p.GetNextToken ();
        while  (p.MoreTokens ())
        {
          if  (!fieldName.Empty ())
            decodeTable.push_back (AttributeTypeFromStr (fieldName));
          fieldName = p.GetNextToken ();
        }
      }

      else if  (lineName.EqualIgnoreCase ("CodedAttributeValues"))
      {
        kkint32 x = p.GetNextTokenInt ();
        if  ((x < 0)  ||  (x >= (kkint32)decodeTable.size ()))
        {
          log.Level (-1) << endl << "AttributeTypeVector::ReadXML   ***ERROR***    Invalid Code Value[" << x << "]" << endl  << endl;
          push_back (AttributeType::NULLAttribute);
        }
        else
        {
          push_back ((AttributeType)x);
        }
        
      }
    }
    t = s.GetNextToken (log);
  }
}




XmlFactoryMacro(Attribute)
XmlFactoryMacro(AttributeList)
XmlFactoryMacro(AttributeTypeVector)