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


#include "KKBaseTypes.h"
#include "DateTime.h"
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




namespace KKMLL 
{
  /**
   *@class FileDescList
   *@brief Container class file 'FileDesc' instances.  
   *@details The class definition is not in the header file because there is no reason for any other entity 
   * other than FileDesc to access a list of FileDesc instances.
   */
  class  FileDescList: public KKQueue<FileDesc>
  {
  public:
    FileDescList (bool _owner);

    ~FileDescList ();

  private:
  };

}  /* KKMLL */




void  FileDesc::FinalCleanUp ()
{
  if  (finalCleanUpRanAlready)
    return;

  FileDesc::CreateBlocker ();
  blocker->StartBlock ();
  if  (!finalCleanUpRanAlready)
  {
    if  (exisitingDescriptions)
    {
      delete  exisitingDescriptions;
      exisitingDescriptions = NULL;
    }
    finalCleanUpRanAlready = true;
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
  kkint32  ZED = 7887;
}


kkint32  FileDesc::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = sizeof (FileDesc)         +
         attributes.MemoryConsumedEstimated ()               +
         sizeof (AttributeType) * attributeVector.size ()    +
         sizeof (kkint32)         * cardinalityVector.size ()  +
         classes.MemoryConsumedEstimated ()                  +
         classNameAttribute.MemoryConsumedEstimated ();

  return  memoryConsumedEstimated;
}



FileDescPtr   FileDesc::NewContinuousDataOnly (RunLog&       _log,
                                               VectorKKStr&  _fieldNames
                                              )
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

      newFileDesc->AddAAttribute (fieldName, NumericAttribute, alreadyExists);
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
  if  (attribute.Type () == NumericAttribute)
     card = 999999999;

  cardinalityVector.push_back (card);
}  /* AddAAttribute */




void  FileDesc::AddAAttribute (const KKStr&   _name,
                               AttributeType  _type,
                               bool&          alreadyExists
                              )
{
  alreadyExists = false;
  AttributePtr existingAttribute = attributes.LookUpByName (_name);
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
  if  (curAttribute->Type () == NumericAttribute)
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
                                  bool&         alreadyExist,
                                  RunLog&       log
                                 )
                        
{
  ValidateFieldNum (fieldNum, "AddANominalValue");
  AttributeType t = Type (fieldNum);
  if  ((t == NominalAttribute)  ||  (t = SymbolicAttribute))
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
    log.Level (-1) << endl
                   << "FileDesc::AddANominalValue    *** ERROR ***    No Current Attribute Set." << endl
                   << endl;
    osWaitForEnter ();
    exit (-1);
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
  curAttribute = attributes.LookUpByName (attributeName);
  if  (!curAttribute)
  {
    log.Level (-1) << endl
                   << "FileDesc::AddANominalValue   *** ERROR ***,   Invalid Attribute[" << attributeName << "]." << endl
                   << endl;
    osWaitForEnter ();
    exit(-1);
  }

  AddANominalValue (nominalValue, alreadyExist, log);
}  /* AddANominalValue */




MLClassPtr       FileDesc::LookUpMLClassByName (const KKStr&  className)
{
  return  classes.LookUpByName (className);
}



MLClassPtr       FileDesc::LookUpUnKnownMLClass ()
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
    cerr  << endl
          << endl
          << "FileDesc::" << funcName << "   *** ERROR ***  Invalid FieldNum[" << fieldNum << "]  Only [" << attributes.QueueSize () << "] fields defined." << endl
          << endl
          << endl;
    osWaitForEnter ();
    exit (-1);
  }
}  /* ValidateFieldNum */




kkint32 FileDesc::LookUpNominalCode (kkint32       fieldNum, 
                                     const KKStr&  nominalValue
                                    )  const
{
  ValidateFieldNum (fieldNum, "LookUpNominalCode");
  const Attribute& a = attributes[fieldNum];
  if  ((a.Type () != NominalAttribute)  &&  (a.Type () != SymbolicAttribute))
  {
    return -1;
  }

  return  a.GetNominalCode (nominalValue);
}  /* LookUpNominalCode */





kkint32  FileDesc::Cardinality (kkint32  fieldNum,
                              RunLog&  log
                             )  const
{
  ValidateFieldNum (fieldNum, "Type");

  AttributePtr a = attributes.IdxToPtr (fieldNum);

  if  (!a)
  {
    log.Level (-1) << endl
                   << endl
                   << "FileDesc::Cardinality    *** ERROR ***" << endl
                   << "                Could not locate attribute[" << fieldNum << "]" << endl
                   << endl;
    osWaitForEnter ();
    exit (-1);
  }

  switch  (a->Type ())
  {
  case  IgnoreAttribute:   return  a->Cardinality ();
  case  NominalAttribute:  return  a->Cardinality ();
  case  NumericAttribute:  return  INT_MAX;
  case  SymbolicAttribute: return  a->Cardinality ();
  
  default: return  INT_MAX;
  }

  return  INT_MAX;
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



const KKStr&     FileDesc::GetNominalValue (kkint32  fieldNum, 
                                            kkint32  code
                                           ) const
{
  ValidateFieldNum (fieldNum, "GetNominalValue");
  return  attributes[fieldNum].GetNominalValue (code);
}  /* GetNominalValue */





const
AttributePtr*  FileDesc::CreateAAttributeTable ()  const
{
  AttributePtr*  table = new AttributePtr[attributes.QueueSize ()];

  for  (kkint32 x = 0;  x < attributes.QueueSize ();  x++)
    table[x] = attributes.IdxToPtr (x);
  
  return  table;
}  /* CreateAAttributeTable */




vector<AttributeType>  FileDesc::CreateAttributeTypeTable ()  const
{
  kkint32  x;
  vector<AttributeType>  attributeTypes (attributes.QueueSize (), NULLAttribute);
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
FileDescListPtr  FileDesc::exisitingDescriptions = NULL;


GoalKeeperPtr    FileDesc::blocker = NULL;


bool             FileDesc::finalCleanUpRanAlready = false;



// Will instantiate an instance of "GoalKeeper" if "blocker" does not already
// point one.
void  FileDesc::CreateBlocker ()
{
  if  (!blocker)
    GoalKeeper::Create ("FileDescBlocker", blocker);  // Will handle Race condition.
}



FileDescPtr  FileDesc::GetExistingFileDesc (FileDescPtr  fileDesc)
{
  FileDescPtr  result = NULL;

  CreateBlocker ();

  blocker->StartBlock ();

  if  (!exisitingDescriptions)
  {
    exisitingDescriptions = new FileDescList (true);
    exisitingDescriptions->PushOnBack (fileDesc);
    result = fileDesc;
    finalCleanUpRanAlready = false;
    atexit (FileDesc::FinalCleanUp);
  }

  else
  {
    FileDescList::iterator  idx;

    for  (idx = exisitingDescriptions->begin ();  idx != exisitingDescriptions->end ();  idx++)
    {
      FileDescPtr existingFileDesc = *idx;

      if  (existingFileDesc == fileDesc)
        return  fileDesc;

      if  ((*existingFileDesc) == (*fileDesc))
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
      exisitingDescriptions->PushOnBack (fileDesc);
      result = fileDesc;
    }
  }

  blocker->EndBlock ();

  return  result;
} /* GetExistingFileDesc */





FileDescList::FileDescList (bool  _owner):
  KKQueue<FileDesc> (_owner)
{
}



FileDescList::~FileDescList ()
{
}




void FileDesc::DisplayAttributeMappings ( )
{
  kkuint32 i;
  kkint32  j;
  AttributePtr a;

  for (i = 0; i < NumOfFields(); i++)
  {
    a = attributes.IdxToPtr (i);
    cout << i << ": ";

    if  (a->Type() == NominalAttribute)
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

    else if  (a->Type() == SymbolicAttribute)
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

    
    else if (a->Type() == IgnoreAttribute)
    {
      cout << "ignore";
    }

    else if (a->Type() == NumericAttribute)
    {
      cout << "numeric";
    }

    else if (a->Type() == OrdinalAttribute)
    {
      cout << "ordinal";
    }

    else if (a->Type() == NULLAttribute)
    {
      cout << "NULL";
    }
    cout << endl;
  }
}  /* DisplayAttributeMappings */



const
AttributePtr  FileDesc::LookUpByName (const KKStr&  attributeName)  const
{
  return  attributes.LookUpByName (attributeName);
}  /* LookUpByName */




kkint32  FileDesc::GetFieldNumFromAttributeName (const KKStr&  attributeName)  const
{
  AttributePtr  a = attributes.LookUpByName (attributeName);
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
    if  ((t != NumericAttribute)  &&  (t != IgnoreAttribute))
      return false;
  }

  
  return  true;
}  /* AllFieldsAreNumeric */





FileDescPtr  FileDesc::MergeSymbolicFields (const FileDesc&  left,
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
    if  (lType != SymbolicAttribute)
    {
      continue;
    }

    // We can merge in Nominal Values for this field.

    kkint32  z;
    for  (z = 0;  z < right.Cardinality (fieldNum, log);  z++)
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

