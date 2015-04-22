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
  if  ((type == NominalAttribute)  ||  (type == SymbolicAttribute))
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
  if  ((type == NominalAttribute)  ||  (type == SymbolicAttribute))
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
  if  ((type != NominalAttribute)  &&  (type != SymbolicAttribute))
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
  if  ((type == NominalAttribute)  ||  (type == SymbolicAttribute))
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




XmlElementAttribute::XmlElementAttribute (XmlTagPtr   tag,
                                          XmlStream&  s,
                                          RunLog&     log
                                         ):
  XmlElement (tag, s, log),
  value (NULL)
{
  AttributeType  attributeType = NULLAttribute;
  kkint32        fieldNum      = 0;
  KKStr          name          = "";

  kkint32  c = tag->AttributeCount ();
  for  (kkint32 x = 0;  x < c;  ++x)
  {
    KKStrConstPtr n = tag->AttributeName (x);
    KKStrConstPtr v = tag->AttributeValue (x);

    if  (n->EqualIgnoreCase ("Type"))
      attributeType = AttributeTypeFromStr (v);

    else if  (n->EqualIgnoreCase ("FieldNum"))
      fieldNum = v->ToInt32 ();

    else if  (n->EqualIgnoreCase ("Name"))
      name= v;
  }

  value = new Attribute (name, attributeType, fieldNum);

  XmlTokenPtr t = s.GetNextToken (log);
  while  (t)
  {
    if  (t->TokenType () == XmlToken::tokContent)
    {
      XmlContentPtr  content = dynamic_cast<XmlContentPtr> (t);
      KKStrParser p (*content->Content ());
      KKStr  nextName = p.GetNextToken (",\t\n\\r");
      bool alreadyEixists = false;
      value->AddANominalValue (nextName, alreadyEixists);
    }
  };
}
typedef  XmlElementAttribute*  XmlElementAttributePtr;




XmlElementAttribute::~XmlElementAttribute ()
{
  delete  value;
  value = NULL;
}


AttributePtr  XmlElementAttribute::Value ()  const
{
  return  value;
}


AttributePtr  XmlElementAttribute::TakeOwnership ()
{
  AttributePtr  v = value;
  value = NULL;
  return  v;
}



void  XmlElementAttribute::WriteXML (const Attribute&  a,
                                     const KKStr&      varName,
                                     ostream&          o
                                    )
{
  XmlTag::TagTypes  startTagType = XmlTag::tagEmpty;
  if  ((a.Type () == KKMLL::NominalAttribute) ||  (a.Type () == KKMLL::SymbolicAttribute))
    startTagType  = XmlTag::tagStart;

  XmlTag  startTag ("Attribute", startTagType);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.AddAtribute ("Name",a.Name ());
  startTag.AddAtribute ("Type", a.TypeStr ());
  startTag.AddAtribute ("FieldNum", a.FieldNum ());
  startTag.WriteXML (o);

  if  (startTagType == XmlTag::tagStart)
  {
    // Need to write the nominal or symbolic fields
    for  (kkint32  nominalIdx = 0;  nominalIdx < a.Cardinality ();  ++nominalIdx)
    {
      if  (nominalIdx > 0)
        o << "\t";
      o << a.GetNominalValue (nominalIdx);
    }

    XmlTag  endTag ("Attribute", XmlTag::tagEnd);
    endTag.WriteXML (o);
  }
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


XmlFactoryMacro(Attribute)



KKStr  KKMLL::AttributeTypeToStr (AttributeType  type)
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




AttributeType  KKMLL::AttributeTypeFromStr (const KKStr&  s)
{
  if  (s.EqualIgnoreCase ("Ignore"))
    return  IgnoreAttribute;

  else if  (s.EqualIgnoreCase ("Numeric"))
    return  NumericAttribute;

  else if  (s.EqualIgnoreCase ("Nominal"))
    return  NominalAttribute;

  else if  (s.EqualIgnoreCase ("Ordinal"))
    return  OrdinalAttribute;

  else if  (s.EqualIgnoreCase ("Symbolic"))
    return  SymbolicAttribute;

  else
    return  NULLAttribute;
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




XmlElementAttributeList::XmlElementAttributeList (XmlTagPtr   tag,
                                                  XmlStream&  s,
                                                  RunLog&     log
                                                 ):
  XmlElement (tag, s, log),
  value (NULL)
{
  value = new AttributeList (true);
  XmlTokenPtr  t = s.GetNextToken (log);
  while  (t)
  {
    if  (typeid (*t) == typeid(XmlElementAttribute))
    {
      XmlElementAttributePtr  attrToken = dynamic_cast<XmlElementAttributePtr> (t);
      if  (attrToken->Value ())
        value->PushOnBack (attrToken->TakeOwnership ());
    }
    delete  t;
    t = s.GetNextToken (log);
  }
}
 

XmlElementAttributeList::~XmlElementAttributeList ()
{
  delete  value;
  value = NULL;
}


AttributeListPtr  XmlElementAttributeList::Value ()  const
{
  return  value;
}


AttributeListPtr  XmlElementAttributeList::TakeOwnership ()
{
  AttributeListPtr  v = value;
  value = NULL;
  return  NULL;
}



void  XmlElementAttributeList::WriteXML (const AttributeList&  attributeList,
                                         const KKStr&          varName,
                                         ostream&              o
                                        )
{
  XmlTag  startTag ("AttributeList", XmlTag::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  AttributeList::const_iterator  idx;
  for  (idx = attributeList.begin ();  idx != attributeList.end ();  ++idx)
  {
    AttributePtr  attribute = *idx;
    XmlElementAttribute::WriteXML (*attribute, KKStr::EmptyStr (), o);
  }
  XmlTag  endTag ("AttributeList", XmlTag::tagEnd);
  endTag.WriteXML (o);
}


XmlFactoryMacro(AttributeList)