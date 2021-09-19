#include "FirstIncludes.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "MemoryDebug.h"
using namespace  std;


#include "GlobalGoalKeeper.h"
#include "KKBaseTypes.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace  KKB;

#include "BinaryClassParms.h"
#include "FileDesc.h"
#include "MLClass.h"
using namespace KKMLL;

#include "svm.h"
using namespace  SVM233;


BinaryClassParms::BinaryClassParms (MLClassPtr               _class1,
                                    MLClassPtr               _class2,
                                    const svm_parameter&     _param,
                                    FeatureNumListConstPtr   _selectedFeatures,
                                    float                    _weight
                                   ):
    class1           (_class1),
    class2           (_class2),
    param            (_param),
    selectedFeatures (NULL),
    weight           (_weight)
{
  if  (_selectedFeatures)
    selectedFeatures = new FeatureNumList (*_selectedFeatures);
}



BinaryClassParms::BinaryClassParms (const BinaryClassParms&  binaryClassParms):
    class1           (binaryClassParms.class1),
    class2           (binaryClassParms.class2),
    param            (binaryClassParms.param),
    selectedFeatures (NULL),
    weight           (binaryClassParms.weight)
{
  if  (binaryClassParms.selectedFeatures)
    selectedFeatures = new FeatureNumList (*binaryClassParms.selectedFeatures);
}


  
BinaryClassParms::~BinaryClassParms ()
{
  delete  selectedFeatures;
  selectedFeatures = NULL;
}



kkuint32  BinaryClassParms::NumOfFeatures (FileDescConstPtr fileDesc) const
{
  return  SelectedFeaturesFD (fileDesc)->NumOfFeatures ();
}


void  BinaryClassParms::SelectedFeatures (const FeatureNumList&  _selectedFeatures)
{
  delete  selectedFeatures;
  selectedFeatures  = new FeatureNumList (_selectedFeatures);
}



void  BinaryClassParms::SelectedFeatures (FeatureNumListConstPtr  _selectedFeatures)
{
   delete selectedFeatures;
   selectedFeatures = new FeatureNumList (*_selectedFeatures);
}



KKStr   BinaryClassParms::Class1Name ()  const
{
  if  (class1)
    return  class1->Name ();
  else
    return "";
}  /* Class1Name */



KKStr   BinaryClassParms::Class2Name ()  const
{
  if  (class2)
    return class2->Name ();
  else
    return "";
}  /* Class2Name */



FeatureNumListConstPtr  BinaryClassParms::SelectedFeaturesFD (FileDescConstPtr fileDesc) const
{
  if  (!selectedFeatures)
    selectedFeatures = new FeatureNumList (fileDesc);
  return  selectedFeatures;
}



KKStr  BinaryClassParms::ToTabDelString ()  const
{
  KKStr  result;
  result << "Class1"            << "\t" << Class1Name ()                << "\t"
         << "Class2"            << "\t" << Class2Name ()                << "\t"
         << "Svm_Parameter"     << "\t" << param.ToCmdLineStr ()        << "\t";

  if  (selectedFeatures)
      result << "SelectedFeatures"  << "\t" << selectedFeatures->ToString () << "\t";

  result << "Weight"            << "\t" << weight;

  return  result;
}  /* ToTabDelString */



BinaryClassParmsPtr  BinaryClassParms::CreateFromTabDelStr (const KKStr&  _str)
{
  MLClassPtr         class1    = NULL;
  MLClassPtr         class2    = NULL;
  svm_parameter*     svm_param = NULL;
  FeatureNumListPtr  selectedFeatures = NULL;
  float              weight    = 0.0f;

  KKStr  str (_str);
  while  (!str.Empty())
  {
    KKStr  field = str.ExtractQuotedStr ("\n\r\t", true);
    field.Upper ();
    KKStr  value = str.ExtractQuotedStr ("\n\r\t", true);

    if  (field == "CLASS1")
    {
      class1 = MLClass::CreateNewMLClass (value);
    }

    else if  (field == "CLASS2")
    {
      class2 = MLClass::CreateNewMLClass (value);
    }

    else if  (field == "SVM_PARAMETER")
    {
      svm_param = new svm_parameter (value);
    }

    else if  (field == "SELECTEDFEATURES")
    {
      bool  valid = false;
      selectedFeatures = new FeatureNumList (value, valid);
    }

    else if  (field == "WEIGHT")
      weight = float (atof (value.Str ()));
  }

  BinaryClassParmsPtr  binaryClassParms = (svm_param == NULL) ? NULL :new  BinaryClassParms (class1, class2, *svm_param, selectedFeatures, weight);

  delete  selectedFeatures;  selectedFeatures = NULL;
  delete  svm_param;         svm_param        = NULL;

  return  binaryClassParms;
}  /* CreateFromTabDelStr */



size_t  BinaryClassParms::MemoryConsumedEstimated ()  const
{
  size_t  memoryConsumedEstimated = sizeof (*this)  + param.MemoryConsumedEstimated ();

  if  (selectedFeatures)
    memoryConsumedEstimated += selectedFeatures->MemoryConsumedEstimated ();

  return  memoryConsumedEstimated;
}



BinaryClassParmsList::BinaryClassParmsList ():
        KKQueue<BinaryClassParms> (true)
{
}



BinaryClassParmsList::BinaryClassParmsList (bool  _owner):
        KKQueue<BinaryClassParms> (_owner)
{
}



BinaryClassParmsList::BinaryClassParmsList (const BinaryClassParmsList&  binaryClassList):
        KKQueue<BinaryClassParms> (binaryClassList)
{
}



BinaryClassParmsList::BinaryClassParmsList (const  BinaryClassParmsList&  binaryClassList,
                                            bool                          _owner
                                           ):
        KKQueue<BinaryClassParms> (binaryClassList, _owner)
{
}


  
BinaryClassParmsList::~BinaryClassParmsList ()
{
}



size_t  BinaryClassParmsList::MemoryConsumedEstimated ()  const
{
  size_t  memoryConsumedEstimated = sizeof (BinaryClassParmsList);
  BinaryClassParmsList::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  ++idx)
    memoryConsumedEstimated  += (*idx)->MemoryConsumedEstimated ();
  return  memoryConsumedEstimated;
}



/**
 @brief  Returns the Average number of selected features.
 */
float  BinaryClassParmsList::FeatureCountNet (FileDescConstPtr fileDesc)  const
{
  if  (size () < 1)
    return 0.0f;

  const_iterator  idx;

  kkuint32  featureCountTotal = 0;

  for  (idx = begin ();  idx != end ();  idx++)
  {
    const BinaryClassParmsPtr bcp = *idx;
    featureCountTotal += bcp->NumOfFeatures (fileDesc);
  }

  return  scFLOAT (featureCountTotal / scFLOAT (size ()));
}



BinaryClassParmsPtr  BinaryClassParmsList::LookUp (MLClassPtr  _class1,
                                                   MLClassPtr  _class2
                                                  )  const
{
  KeyField kf (_class1, _class2);
  ClassIndexType::const_iterator  idx;
  idx = classIndex.find (kf);
  if  (idx == classIndex.end ())
  {
    kf.class1 = _class2;
    kf.class2 = _class1;
    idx = classIndex.find (kf);
  }

  if  (idx == classIndex.end ())
    return NULL;
  else
    return idx->second;
}  /* LookUp */



BinaryClassParmsListPtr  BinaryClassParmsList::DuplicateListAndContents ()  const
{
  BinaryClassParmsListPtr  duplicatedQueue = new BinaryClassParmsList (true);

  for  (const_iterator idx = begin ();  idx != end ();  idx++)
  {
    BinaryClassParmsPtr e = *idx;
    duplicatedQueue->PushOnBack (new BinaryClassParms (*e));
  }
  
  return  duplicatedQueue;
}  /* DuplicateListAndContents */



void  BinaryClassParmsList::WriteXML (ostream&  o)  const
{
  o << "<BinaryClassParmsList>" << endl;
  
  const_iterator  idx;

  for  (idx = begin ();  idx != end ();  idx++)
  {
    o << "<BinaryClassParms>"                   << "\t"
      << (*idx)->ToTabDelString ().QuotedStr () << "\t"
      << "</BinaryClassParms>" 
      << endl;
  }
  
  o << "</BinaryClassParmsList>" << endl;
}  /* WriteXML */



void  BinaryClassParmsList::PushOnBack  (BinaryClassParmsPtr  binaryParms)
{
  BinaryClassParmsPtr  existingEntry = LookUp (binaryParms->Class1 (), binaryParms->Class2 ());
  if  (existingEntry)
  {
    // We have a duplicate entry
    KKStr  errMsg (128U);
    errMsg << "BinaryClassParmsList::PushOnBack   ***ERROR***  Duplicate Entry   " << binaryParms->Class1Name () << "\t" << binaryParms->Class2Name () << endl;
    cerr << errMsg << endl;
    throw  KKException (errMsg);
  }

  KKQueue<BinaryClassParms>::PushOnBack (binaryParms);
  KeyField kf (binaryParms->Class1 (), binaryParms->Class2 ());
  classIndex.insert(ClassIndexPair (kf, binaryParms));
}  /* PushOnBack */



void  BinaryClassParmsList::PushOnFront (BinaryClassParmsPtr  binaryParms)
{
  BinaryClassParmsPtr  existingEntry = LookUp (binaryParms->Class1 (), binaryParms->Class2 ());
  if  (existingEntry)
  {
    // We have a duplicate entry
    KKStr  errMsg (128U);
    errMsg << "BinaryClassParmsList::PushOnFront   ***ERROR***  Duplicate Entry   " << binaryParms->Class1Name () << "\t" << binaryParms->Class2Name () << endl;
    cerr << errMsg << endl;
    throw  KKException (errMsg);
  }

  KKQueue<BinaryClassParms>::PushOnFront (binaryParms);
  KeyField kf (binaryParms->Class1 (), binaryParms->Class2 ());
  classIndex.insert(ClassIndexPair (kf, binaryParms));
}  /* PushOnFront */



BinaryClassParmsList::KeyField::KeyField (MLClassPtr  _class1,  
                                          MLClassPtr  _class2
                                         ):
    class1 (_class1), 
    class2 (_class2)
{
}



bool  BinaryClassParmsList::KeyField::operator< (const KeyField& p2) const
{
  kkint32 x = class1->Name ().Compare(p2.class1->Name ());
  if  (x < 0)
    return true;

  else if  (x > 0)
    return false;

  else 
    return class2->Name () < p2.class2->Name ();
}



void  BinaryClassParmsList::WriteXML (const KKStr&  varName,
                                      ostream&      o
                                     )  const
{
  XmlTag  startTag ("BinaryClassParmsList",  XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);
  o << endl;

  for  (auto  idx : *this)
  {
    XmlContent::WriteXml (idx->ToTabDelString (), o);
    o << endl;
  }

  XmlTag  endTag ("BinaryClassParmsList", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}  /* WriteXML */



void  BinaryClassParmsList::ReadXML (XmlStream&      s,
                                     XmlTagConstPtr  tag,
                                     VolConstBool&   cancelFlag,
                                     RunLog&         log
                                    )
{
  log.Level (50) << "BinaryClassParmsList::ReadXML  tag->Name: " << tag->Name () << endl;
  DeleteContents ();
  classIndex.clear ();

  bool  errorsFound = false;
  XmlTokenPtr  t = s.GetNextToken (cancelFlag, log);
  while  (t  &&  (!cancelFlag)  &&  (!errorsFound))
  {
    if  (t->TokenType () == XmlToken::TokenTypes::tokContent)
    {
      XmlContentPtr content = dynamic_cast<XmlContentPtr> (t);
      if  (content  &&  (content->Content ()))
      {
        BinaryClassParmsPtr  bcp = BinaryClassParms::CreateFromTabDelStr (*(content->Content ()));
        if  (bcp)
          PushOnBack (bcp);
      }
    }
    delete  t;
    t = s.GetNextToken (cancelFlag, log);
  }
  delete  t;
  t = NULL;
}  /* ReadXML */


XmlFactoryMacro(BinaryClassParmsList)
