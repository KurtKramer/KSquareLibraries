#include "FirstIncludes.h"
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "MemoryDebug.h"
using namespace  std;


#include "GlobalGoalKeeper.h"
#include "DateTime.h"
#include "KKBaseTypes.h"
#include "KKException.h"
#include "OSservices.h"
#include "KKQueue.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace  KKB;


#include "FileDesc.h"
#include "FeatureNumList.h"
#include "MLClass.h"
#include "FeatureVector.h"
using namespace  KKMLL;



void  FileDesc::FinalCleanUp ()
{
  if  (finalCleanUpRanAlready)
    return;

  FileDesc::CreateBlocker ();
  blocker->StartBlock ();
  if  (!finalCleanUpRanAlready)
  {
    while  (exisitingDescriptions.size() > 0)
    {
      auto fd = exisitingDescriptions.back ();
      exisitingDescriptions.pop_back ();
      delete (FileDescPtr)fd;
    }
  }
  blocker->EndBlock ();

  GoalKeeper::Destroy (blocker);  blocker = NULL;
}  /* FinalCleanUp */




FileDesc::FileDesc ():

  attributes          (true),
  cardinalityVector   (),
  classes             (),
  curAttribute        (NULL),
  sparseMinFeatureNum (0),
  version             (0)
{
}


   
FileDesc::~FileDesc ()
{
}



kkMemSize  FileDesc::MemoryConsumedEstimated ()  const
{
  kkMemSize  memoryConsumedEstimated = sizeof (FileDesc)         +
             attributes.MemoryConsumedEstimated ()               +
             sizeof (AttributeType) * attributeVector.size ()    +
             sizeof (kkint32)       * cardinalityVector.size ()  +
             classes.MemoryConsumedEstimated ()                  +
             classNameAttribute.MemoryConsumedEstimated ();

  return  memoryConsumedEstimated;
}



FileDescConstPtr   FileDesc::NewContinuousDataOnly (VectorKKStr&  _fieldNames)
{
  bool  alreadyExists = false;
  FileDescPtr  newFileDesc = new FileDesc ();
  for  (kkint32 fieldNum = 0;  fieldNum < (kkint32)_fieldNames.size ();  fieldNum++)
  {
    kkint32  seqNum = 0;
    do
    {
      KKStr  fieldName = _fieldNames[fieldNum];
      if  (seqNum > 0)
        fieldName << "_" << StrFormatInt (seqNum, "000");

      newFileDesc->AddAAttribute (fieldName, AttributeType::Numeric, alreadyExists);
      seqNum++;
    }
      while  (alreadyExists);
  }

  return  GetExistingFileDesc (newFileDesc);
}  /* NewContinuousDataOnly */



void  FileDesc::AddAAttribute (const Attribute&  attribute)
{
  attributes.PushOnBack (new Attribute (attribute));
  attributeVector.push_back (attribute.Type ());

  kkint32  card = 0;
  if  (attribute.Type () == AttributeType::Numeric)
     card = 999999999;

  cardinalityVector.push_back (card);
}  /* AddAAttribute */



void  FileDesc::AddAttributes (const KKMLL::AttributeList&  attributesToAdd)
{
  for  (auto idx = attributesToAdd.begin ();  idx != attributesToAdd.end ();  ++idx)
  {
    AddAAttribute (**idx);
  }
}



void  FileDesc::AddAAttribute (const KKStr&   _name,
                               AttributeType  _type,
                               bool&          alreadyExists
                              )
{
  alreadyExists = false;
  auto existingAttribute = attributes.LookUpByName (_name);
  if  (existingAttribute)
  {
    // This is a very bad error, it should not be able to happen
    alreadyExists = true;
    return;
  }

  curAttribute = new Attribute (_name, _type, attributes.QueueSize ());
  attributes.PushOnBack (curAttribute);
  attributeVector.push_back (curAttribute->Type ());

  kkint32  card = 0;
  if  (curAttribute->Type () == AttributeType::Numeric)
     card = INT_MAX;
  cardinalityVector.push_back (card);
}  /* AddAAttribute */



void  FileDesc::AddClasses (const MLClassList&  classesToAdd)
{
  MLClassList::const_iterator  idx;
  for  (idx = classesToAdd.begin ();  idx != classesToAdd.end ();  idx++)
  {
    MLClassPtr  ic = *idx;
    if  (classes.PtrToIdx (ic) < 0)
      classes.AddMLClass (ic);
  }
}  /* AddClasses */



const Attribute&  FileDesc::GetAAttribute (kkint32 fieldNum) const
{
  ValidateFieldNum (fieldNum, "GetAAttribute");
  const Attribute&  a = attributes [fieldNum];
  return  a;
}  /* GetAAttribute */



void  FileDesc::AddANominalValue (kkint32       fieldNum,
                                  const KKStr&  nominalValue,
                                  bool&         alreadyExist
                                 )
                        
{
  ValidateFieldNum (fieldNum, "AddANominalValue");
  AttributeType t = Type (fieldNum);
  if  ((t == AttributeType::Nominal)  ||
       (t == AttributeType::Symbolic)
      )
  {
    attributes[fieldNum].AddANominalValue (nominalValue, alreadyExist);
    if  (!alreadyExist)
      cardinalityVector[fieldNum]++;
  }
}  /* AddANominalValue */



void  FileDesc::AddANominalValue (const KKStr&   nominalValue,
                                  bool&          alreadyExist,
                                  RunLog&        log
                                 )
{
  if  (!curAttribute)
  {
    // This should never happen, means that there has not been a nominal feature added yet.
    KKStr  errMsg = "FileDesc::AddANominalValue    ***ERROR***    No Current Attribute Set.";
    log.Level (-1) << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }
  
  alreadyExist = false;
  curAttribute->AddANominalValue (nominalValue, alreadyExist);
  if  (!alreadyExist)
    cardinalityVector[curAttribute->FieldNum ()]++;
}  /* AddANominalValue */



void  FileDesc::AddANominalValue (const KKStr&   attributeName,
                                  const KKStr&   nominalValue,
                                  bool&          alreadyExist,
                                  RunLog&        log
                                 )
{
  auto existingAttribute = attributes.LookUpByName (attributeName);
  if  (!existingAttribute)
  {
    KKStr  errMsg (128);
    errMsg << "FileDesc::AddANominalValue   ***ERROR***,   Invalid Attribute[" << attributeName << "].";
    log.Level (-1) << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  AddANominalValue (nominalValue, alreadyExist, log);
}  /* AddANominalValue */



MLClassPtr  FileDesc::LookUpMLClassByName (const KKStr&  className)  const
{
  return  classes.LookUpByName (className);
}



MLClassPtr  FileDesc::LookUpUnKnownMLClass ()  const
{
  return  classes.GetUnKnownClass ();
}



MLClassPtr  FileDesc::GetMLClassPtr (const KKStr& className)
{
  return  classes.GetMLClassPtr (className);
}



void  FileDesc::ValidateFieldNum (kkint32      fieldNum,
                                  const char*  funcName
                                 )  const
{
  if  ((fieldNum < 0)  ||  (fieldNum >= attributes.QueueSize ()))
  {
    KKStr  errMsg (128);
    errMsg << "FileDesc::" << funcName << "   ***ERROR***  Invalid FieldNum[" << fieldNum << "]  Only [" << attributes.QueueSize () << "] fields defined.";
    cerr  << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }
}  /* ValidateFieldNum */



kkint32 FileDesc::LookUpNominalCode (kkint32       fieldNum, 
                                     const KKStr&  nominalValue
                                    )  const
{
  ValidateFieldNum (fieldNum, "LookUpNominalCode");
  const Attribute& a = attributes[fieldNum];
  if  ((a.Type () != AttributeType::Nominal)  &&
       (a.Type () != AttributeType::Symbolic)
      )
  {
    return -1;
  }

  return  a.GetNominalCode (nominalValue);
}  /* LookUpNominalCode */



kkint32  FileDesc::Cardinality (kkint32  fieldNum)  const
{
  ValidateFieldNum (fieldNum, "Type");

  AttributePtr a = attributes.IdxToPtr (fieldNum);

  if  (!a)
  {
    KKStr  errMsg;
    errMsg << "FileDesc::Cardinality    ***ERROR***    Could not locate attribute[" << fieldNum << "]";
    cerr << errMsg;
    throw  KKException (errMsg);
  }

  switch  (a->Type ())
  {
  case  AttributeType::Ignore:   return  a->Cardinality ();
  case  AttributeType::Nominal:  return  a->Cardinality ();
  case  AttributeType::Numeric:  return  INT_MAX;
  case  AttributeType::Symbolic: return  a->Cardinality ();
  
  default: return  INT_MAX;
  }
}  /* Cardinality */



AttributeType  FileDesc::Type (kkint32 fieldNum)  const
{
  ValidateFieldNum (fieldNum, "Type");
  return  attributes[fieldNum].Type ();
}  /* Type */



KKStr   FileDesc::TypeStr (kkint32 fieldNum)  const
{
  ValidateFieldNum (fieldNum, "TypeStr");
  return  AttributeTypeToStr (attributes[fieldNum].Type ());
}  /* TypeStr */



const KKStr&     FileDesc::FieldName (kkint32  fieldNum)  const
{
  ValidateFieldNum (fieldNum, "FieldName");
  return  attributes[fieldNum].Name ();
} /* FieldName */



const KKStr&   FileDesc::GetNominalValue (kkint32  fieldNum, 
                                            kkint32  code
                                           ) const
{
  ValidateFieldNum (fieldNum, "GetNominalValue");
  return  attributes[fieldNum].GetNominalValue (code);
}  /* GetNominalValue */



AttributePtr*  FileDesc::CreateAAttributeTable ()
{
  AttributePtr*  table = new AttributePtr[attributes.QueueSize ()];
  for  (kkint32 x = 0;  x < attributes.QueueSize ();  x++)
    table[x] = attributes.IdxToPtr (x);
  
  return  table;
}  /* CreateAAttributeTable */



AttributeConstPtr*  FileDesc::CreateAAttributeConstTable ()  const
{
  AttributeConstPtr*  table = new AttributeConstPtr[attributes.QueueSize ()];
  for (kkint32 x = 0; x < attributes.QueueSize (); x++)
    table[x] = attributes.IdxToPtr (x);

  return  table;
}



AttributeTypeVector  FileDesc::CreateAttributeTypeTable ()  const
{
  kkint32  x;
  AttributeTypeVector  attributeTypes ((kkuint32)attributes.size (), AttributeType::NULLAttribute);
  for  (x = 0;  x < attributes.QueueSize ();  x++)
    attributeTypes[x] = attributes[x].Type ();
  return attributeTypes;
}  /* CreateAttributeTypeTable () */



VectorInt32   FileDesc::CreateCardinalityTable ()  const
{
  kkint32  x;
  VectorInt32  cardinalityTable (attributes.QueueSize (), 0);
  for  (x = 0;  x < attributes.QueueSize ();  x++)
    cardinalityTable[x] = attributes[x].Cardinality ();
  return cardinalityTable;
}  /* CreateCardinalityTable */



bool  FileDesc::operator== (const FileDesc&  rightSide)  const
{
  if  ((NumOfFields () != rightSide.NumOfFields ())  ||
       (Version ()     != rightSide.Version ())      ||
       (attributes     != rightSide.attributes)
      )
    return false;

  return  true;  
} /* operator== */



bool  FileDesc::operator!= (const FileDesc&  rightSide)  const
{
  if  ((NumOfFields () != rightSide.NumOfFields ())  ||
       (Version ()     != rightSide.Version ())      ||
       (attributes     != rightSide.attributes)
      )
    return true;

  return  false;  
} /* operator== */



bool  FileDesc::SameExceptForSymbolicData (const FileDesc&  otherFd,
                                           RunLog&          log
                                          )  const
{
  bool  same = true;

  if  (NumOfFields () != otherFd.NumOfFields ())
  {
    log.Level (-1) << endl
                   << "FileDesc::SameExceptForSymbolicData    Field count mis-match" << endl
                   << "          File[" << fileName << "] count[" << otherFd.NumOfFields () << "]  File[" << otherFd.fileName << "] Count[" << otherFd.NumOfFields () << "]" << endl
                   << endl;

    return  false;
  }

  kkint32  numOfFields = NumOfFields ();
  kkint32  fieldNum = 0;

  const KKStr&  rightFileName = otherFd.FileName ();

  for  (fieldNum = 0;  fieldNum < numOfFields;  fieldNum++)
  {
    const KKStr&  lName = FieldName (fieldNum);
    const KKStr&  rName = otherFd.FieldName (fieldNum);

    if  (lName != rName)
    {
      log.Level (-1) << endl
                     << "FileDesc::SameExceptForSymbolicData   Field Name mis-match" << endl
                     << "          File[" << fileName << "] Name[" << lName << "]  File[" << rightFileName << "] Name[" << rName << "]" << endl
                     << endl;

      same = false;
    }

    else
    {
      AttributeType  lType = Type (fieldNum);
      AttributeType  rType = otherFd.Type (fieldNum);

      if  (lType != rType)
      {
        log.Level (-1) << endl
                       << "FileDesc::SameExceptForSymbolicData   Field Type  mis-match" << endl
                       << "          File[" << fileName << "] Name[" << lName << "]  File[" << rightFileName << "] Name[" << rName << "]" << endl
                       << endl;

        same = false;
      }
    }
  }

  return  same;
}  /* SameExceptForSymbolicData */



// Will keep a list of all FileDesc
// instances instantiated.
vector<FileDescConstPtr>  FileDesc::exisitingDescriptions;


GoalKeeperPtr    FileDesc::blocker = NULL;


bool             FileDesc::finalCleanUpRanAlready = false;



// Will instantiate an instance of "GoalKeeper" if "blocker" does not already
// point one.
void  FileDesc::CreateBlocker ()
{
  if  (!blocker)
    GoalKeeper::Create ("FileDescBlocker", blocker);  // Will handle Race condition.
}



FileDescConstPtr  FileDesc::GetExistingFileDesc (FileDescConstPtr  fileDesc)
{
  if (fileDesc == NULL)
  {
    KKStr errMsg = "FileDesc::GetExistingFileDesc   ***ERROR***   (fileDesc == NULL).";
    cerr << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }
  FileDescConstPtr  result = NULL;

  CreateBlocker ();

  blocker->StartBlock ();

  for  (auto existingFileDesc: exisitingDescriptions)  
  {
    if  (existingFileDesc == fileDesc)
    {
      result = existingFileDesc;
      break;
    }
    else if  (existingFileDesc == NULL)
    {
      continue;
    }
    else if  ((*existingFileDesc) == (*fileDesc))
    {
      // Looks like we already have a compatible "FileDesc" instance.
      // In this case this is the one the user will want.
      delete  fileDesc;
      result = existingFileDesc;
      break;
    }
  }

  if  (!result)
  {
    exisitingDescriptions.push_back (fileDesc);
    result = fileDesc;
  }

  blocker->EndBlock ();

  return  result;
} /* GetExistingFileDesc */



void FileDesc::DisplayAttributeMappings ( )
{
  kkuint32 i;
  kkint32  j;
  AttributePtr a;

  for (i = 0; i < NumOfFields(); i++)
  {
    a = attributes.IdxToPtr (i);
    cout << i << ": ";

    if  (a->Type() == AttributeType::Nominal)
    {
      for  (j = 0;  j<a->Cardinality ();  j++)
      {
        if (j != a->GetNominalCode( a->GetNominalValue(j) ))
        {
          cout << " Code does not match ";
        }
        cout << j << ":" << a->GetNominalValue(j) << "  ";
      }
    }

    else if  (a->Type() == AttributeType::Symbolic)
    {
      cout << "Symbolic (";
      for  (j = 0;  j<a->Cardinality ();  j++)
      {
        if (j != a->GetNominalCode ( a->GetNominalValue(j) ))
        {
          cout << " Code does not match ";
        }
        cout << j << ":" << a->GetNominalValue(j) << "  ";
      }

      cout << ")";
    }

    
    else if (a->Type() == AttributeType::Ignore)
    {
      cout << "ignore";
    }

    else if (a->Type() == AttributeType::Numeric)
    {
      cout << "numeric";
    }

    else if (a->Type() == AttributeType::Ordinal)
    {
      cout << "ordinal";
    }

    else if (a->Type() == AttributeType::NULLAttribute)
    {
      cout << "NULL";
    }
    cout << endl;
  }
}  /* DisplayAttributeMappings */



AttributeConstPtr  FileDesc::LookUpByName (const KKStr&  attributeName)  const
{
  return  attributes.LookUpByName (attributeName);
}  /* LookUpByName */



kkint32  FileDesc::GetFieldNumFromAttributeName (const KKStr&  attributeName)  const
{
  auto  a = attributes.LookUpByName (attributeName);
  if  (!a)
    return -1;
  else
    return a->FieldNum ();
}  /* GetFieldNumFromAttributeName */



/**
 * @brief Allows the user to quickly determine if there are no nominal fields.
 * @details  Example use is in CrossValidation application, by using this method it can quickly determine
 * if it is worth while using encoding.
 */
bool  FileDesc::AllFieldsAreNumeric ()  const
{
  for  (kkint32 fieldNum = 0;  fieldNum < (kkint32)NumOfFields ();  fieldNum++)
  {
    AttributeType  t = Type (fieldNum);
    if  ((t != AttributeType::Numeric)  &&  (t != AttributeType::Ignore))
      return false;
  }

  
  return  true;
}  /* AllFieldsAreNumeric */



FileDescConstPtr  FileDesc::MergeSymbolicFields (const FileDesc&  left,
                                                 const FileDesc&  right,
                                                 RunLog&          log
                                                )
{
  if  (!left.SameExceptForSymbolicData (right, log))
  {
    log.Level (-1) << endl
                   << "FileDesc::MergeSymbolicFields   There are more differences between file descriptions besides symbolic data." << endl
                   << "          File[" << left.fileName << "] File[" << right.fileName << "]" << endl
                   << endl;

    return  NULL;
  }

  FileDescPtr  f = new FileDesc ();

  kkint32  numOfFields = left.NumOfFields ();

  kkint32  fieldNum = 0;

  for  (fieldNum = 0;  fieldNum < numOfFields;  fieldNum++)
  {
    const KKStr&  lName = left.FieldName (fieldNum);
    const KKStr&  rName = right.FieldName (fieldNum);

    if  (lName != rName)
    {
      log.Level (-1) << endl
                     << "FileDesc::MergeSymbolicFields   Field Name mis-match" << endl
                     << "          File[" << left.fileName << "] Name[" << lName << "]  File[" << right.fileName << "] Name[" << rName << "]" << endl
                     << endl;

      return NULL;
    }

    AttributeType  lType = left.Type (fieldNum);
    AttributeType  rType = right.Type (fieldNum);

    if  (lType != rType)
    {
      log.Level (-1) << endl
                     << "FileDesc::MergeSymbolicFields   Field Type  mis-match" << endl
                     << "          File[" << left.fileName << "] Name[" << lName << "]  File[" << right.fileName << "] Name[" << rName << "]" << endl
                     << endl;

      return  NULL;
    }

    f->AddAAttribute (left.GetAAttribute (fieldNum));
    if  (lType != AttributeType::Symbolic)
    {
      continue;
    }

    // We can merge in Nominal Values for this field.

    kkint32  z;
    for  (z = 0;  z < right.Cardinality (fieldNum);  z++)
    {
      const KKStr&  rightNomName = right.GetNominalValue (fieldNum, z);
      kkint32  lCode = f->LookUpNominalCode (fieldNum, rightNomName);
      if  (lCode < 0)
      {
        bool  alreadyExists = false;
        f->AddANominalValue (fieldNum, rightNomName, alreadyExists, log);
        lCode = f->LookUpNominalCode (fieldNum, rightNomName);
      }
    }
  }

  return  GetExistingFileDesc (f);
}  /* MergeSymbolicFields */



void  FileDesc::ReadXML (XmlStream&      s,
                         XmlTagConstPtr  tag,
                         VolConstBool&   cancelFlag,
                         RunLog&         log
                        )
{
  log.Level (50) << "FileDesc::ReadXML    tag->Name: " << tag->Name () << endl;
  XmlTokenPtr  t = s.GetNextToken (cancelFlag, log);
  while  (t  &&  (!cancelFlag))
  {
    if  (t->TokenType () == XmlToken::TokenTypes::tokElement)
    {
      XmlElementPtr  e = dynamic_cast<XmlElementPtr> (t);
      const KKStr&  className = e->SectionName ();
      const KKStr&  varName = e->VarName ();
      if  (varName.EqualIgnoreCase ("FileName"))
      {
        XmlElementKKStrPtr  eKKStr = dynamic_cast<XmlElementKKStrPtr>(e);
        if  (eKKStr)
          FileName (*(eKKStr->Value ()));
      }

      else if  (varName.EqualIgnoreCase ("Attributes"))
      {
        XmlElementAttributeListPtr  eAttributes = dynamic_cast<XmlElementAttributeListPtr>(e);
        if  (eAttributes  &&  (eAttributes->Value ()))
          AddAttributes (*(eAttributes->Value ()));
      }

      else if  (varName.EqualIgnoreCase ("Classes"))
      {
        XmlElementMLClassNameListPtr  eCLasses = dynamic_cast<XmlElementMLClassNameListPtr>(e);
        if  (eCLasses  &&  (eCLasses->Value ()))
          AddClasses (*(eCLasses->Value ()));
      }

      else if  (varName.EqualIgnoreCase ("ClassNameAttribute"))
      {
        XmlElementKKStrPtr  eKKStr = dynamic_cast<XmlElementKKStrPtr>(e);
        if  (eKKStr)
          ClassNameAttribute (*(eKKStr->Value ()));
      }

      else if  (varName.EqualIgnoreCase ("Version"))
      {
        XmlElementInt32Ptr  eKKInt32 = dynamic_cast<XmlElementInt32Ptr>(e);
        if  (eKKInt32)
          Version ((kkint16)eKKInt32->Value ());
      }

      else if  (varName.EqualIgnoreCase ("SparseMinFeatureNum"))
      {
        XmlElementInt32Ptr  eKKInt32 = dynamic_cast<XmlElementInt32Ptr>(e);
        if  (eKKInt32)
          SparseMinFeatureNum (eKKInt32->Value ());
      }
      else
      {
        log.Level (-1) << endl
          << "XmlElementFileDesc   ***ERROR***   Unexpected Element <" << className << ", VarName=" << varName.QuotedStr () << ">" << endl
          << endl;
      }
    }

    delete t;
    t = s.GetNextToken (cancelFlag, log);
  }

  delete  t;
  t = NULL;
}  /* ReadXML */



void  FileDesc::WriteXML (const KKStr&  varName,
                          ostream&      o
                         )  const


{
  XmlTag  startTag ("FileDesc", XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);
  o << endl;

  if  (!fileName.Empty ())
    XmlElementKKStr::WriteXML (fileName, "FileName", o);

  XmlElementAttributeList::WriteXML (attributes, "Attributes", o);

  XmlElementMLClassNameList::WriteXML (classes, "Classes", o);

  if  (!ClassNameAttribute ().Empty ())
    XmlElementKKStr::WriteXML (ClassNameAttribute (), "ClassNameAttribute", o);

  XmlElementInt32::WriteXML (version, "Version", o);

  if  (sparseMinFeatureNum  != 0)
    XmlElementInt32::WriteXML (sparseMinFeatureNum, "SparseMinFeatureNum", o);

  XmlTag  endTag ("FileDesc", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}



XmlElementFileDesc::XmlElementFileDesc (XmlTagPtr      tag,
                                        XmlStream&     s,
                                        VolConstBool&  cancelFlag,
                                        RunLog&        log
                                       ):
  XmlElement (tag, s, log),
  value (NULL)
{
  auto temp = new FileDesc ();
  temp->ReadXML (s, tag, cancelFlag, log);
  value = FileDesc::GetExistingFileDesc (temp);
}
                

 
XmlElementFileDesc::~XmlElementFileDesc ()
{
  // You can not delete an instance of FileDesc.
  value = NULL;
}



FileDescConstPtr  XmlElementFileDesc::Value ()  const
{
  return  value;
}



FileDescConstPtr  XmlElementFileDesc::TakeOwnership ()
{
  FileDescConstPtr  v = value;
  value = NULL;
  return  v;
}



void  XmlElementFileDesc::WriteXML (const FileDescConst&  fileDesc,
                                    const KKStr&          varName,
                                    ostream&              o
                                  )
{
  fileDesc.WriteXML (varName, o);
}  /* WriteXML */



XmlFactoryMacro(FileDesc)
