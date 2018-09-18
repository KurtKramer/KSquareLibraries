#include "FirstIncludes.h"
#include <vector>
#include <map>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "MemoryDebug.h"
using namespace  std;


#include "KKBaseTypes.h"
#include "KKException.h"
#include "OSservices.h"
using namespace  KKB;


#include "Orderings.h"
#include "FeatureFileIO.h"
#include "FileDesc.h"
#include "MLClass.h"
#include "FeatureVector.h"
using namespace  KKMLL;


Orderings::Orderings (FeatureVectorListPtr  _data,
                      kkuint32              _numOfOrderings,
                      kkuint32              _numOfFolds,
                      RunLog&               _log
                     ):
  
  data           (new FeatureVectorList (*_data, false)),
  fileDesc       (_data->FileDesc ()),
  mlClasses      (_data->ExtractListOfClasses ()),
  numOfFolds     (_numOfFolds),
  numOfOrderings (_numOfOrderings),
  valid          (true)

{
  featureFileName = data->FileName ();
  if  (!featureFileName.Empty ())
    indexFileName = osRemoveExtension (featureFileName) + ".idx";

  CreateOrderings (_log);
}




Orderings::Orderings (const FeatureVectorListPtr  _data,
                      const KKStr&                _indexFileName,
                      kkuint32                    _numOfOrderings,
                      kkuint32                    _numOfFolds,
                      RunLog&                     _log
                     ):
  
  data           (new FeatureVectorList (*_data, false)),
  fileDesc       (_data->FileDesc ()),
  mlClasses      (_data->ExtractListOfClasses ()),
  indexFileName  (_indexFileName),
  numOfFolds     (_numOfFolds),
  numOfOrderings (_numOfOrderings),
  valid          (true)

{
  if  (!osFileExists (indexFileName))
  {
    CreateOrderings (_log);
    Save ();
  }
  else
  {
    Load (_log);
  }
}


Orderings::Orderings (FeatureVectorListPtr  _data,
                      RunLog&               _log
                     ):
  
  data           (new FeatureVectorList (*_data, false)),
  fileDesc       (_data->FileDesc ()),
  mlClasses      (_data->ExtractListOfClasses ()),
  indexFileName  (),
  numOfFolds     (0),
  numOfOrderings (0),
  valid          (true)

{
  featureFileName = data->FileName ();
  if  (featureFileName.Empty ())
  {
    _log.Level (-1) << endl << endl
                   << "Orderings   *** ERROR ***   No File Name in FeatureVectorList object." << endl
                   << endl;
    valid = false;
    return;
  }

  bool  successful = true;
  indexFileName = osRemoveExtension (featureFileName) + ".idx";

  Load (indexFileName, successful, _log);
  if  (!successful)
  {
    _log.Level (-1) << endl
                    << "Orderings::Orderings   Error Loading existing ordering[" << indexFileName << "]" << endl
                    << endl;
    valid = false;
  }

  return;
}






Orderings::Orderings (const KKStr&      _featureFileName,
                      FeatureFileIOPtr  _driver,
                      RunLog&           _log,
                      bool&             _cancelFlag
                    ):
  
  data            (NULL),
  featureFileName (_featureFileName),
  fileDesc        (NULL),
  mlClasses       (NULL),
  numOfFolds      (0),
  numOfOrderings  (0),
  valid           (true)

{
  bool  changesdMade = false;
  bool  successful   = true;

  mlClasses = new MLClassList ();

  data = _driver->LoadFeatureFile (featureFileName, 
                                   *mlClasses,
                                   -1,  // Load in all data
                                   _cancelFlag,
                                   successful,
                                   changesdMade,
                                   _log
                                  );

  if  (_cancelFlag)
  {
    _log.Level (-1) << endl
                   << "Orderings   ***ERROR***   CancelFlag was set,   load was canceled." << endl
                   << endl;
    valid = false;
    return;
  }


  if  (!successful)
  {
    _log.Level (-1) << endl
                   << "Orderings   ***ERROR***   Loading Feature File[" << featureFileName << "]" << endl
                   << endl;
    valid = false;
    return;
  }

  fileDesc = data->FileDesc ();

  indexFileName = osRemoveExtension (featureFileName) + ".idx";
  Load (indexFileName, successful, _log);
  if  (!successful)
  {
    _log.Level (-1) << endl
                    << "Orderings   *** ERROR ***   Loading Index File[" << indexFileName << "]" << endl
                    << endl;
    valid = false;
    return;
  }
}




Orderings::~Orderings ()
{
  DeleteOrderings ();
  delete  data;
}



OrderingsPtr  Orderings::CreateOrderingsObjFromFileIfAvaliable (const FeatureVectorListPtr  _data,
                                                                kkuint32                    _numOfOrderings,
                                                                kkuint32                    _numOfFolds,
                                                                RunLog&                     _log
                                                               )
{
  KKStr  _featureFileName = _data->FileName ();

  if  (_featureFileName.Empty ())
  {
    _log.Level (-1) << endl
                   << "CreateOrderingsObjFromFileIfAvaliable  *** ERROR ***  FileName empty." << endl
                   << endl;
    return NULL;
  }

  KKStr _indexFileName = osRemoveExtension (_featureFileName) + ".idx";
  
  OrderingsPtr  orderings = new Orderings (_data, _indexFileName, _numOfOrderings, _numOfFolds, _log);
  if  (orderings->Valid ())
  {
    if  ((orderings->NumOfOrderings () != _numOfOrderings)  ||  
         (orderings->NumOfFolds     () != _numOfFolds)
        )
    {
      _log.Level (-1) << endl
                     << "CreateOrderingsObjFromFileIfAvaliable   *** ERROR ***      Dimension Mismatched." << endl
                     << endl
                     << "Dimensions Expected   NumOfOrderings[" << _numOfOrderings              << "]  NumOfFolds[" << _numOfFolds              << "]" << endl
                     << "Dimensions Found      NumOfOrderings[" << orderings->NumOfOrderings () << "]  NumOfFolds[" << orderings->NumOfFolds () << "]" << endl
                     << endl;
      delete  orderings;
      return NULL;
    }
  }
  else
  {
    delete orderings;
    orderings = new Orderings (_data, _numOfOrderings, _numOfFolds, _log);
    if  (!orderings->Valid ())
    {
      delete  orderings;
      return  NULL;
    }
    else
    {
      orderings->Save ();
    }
  }

  return  orderings;
}  /* CreateOrderingsObjFromFileIfAvaliable */




void  Orderings::CreateOrderings (RunLog&  log)
{
  DeleteOrderings ();

  FeatureVectorListPtr  workList = new FeatureVectorList (*data, false);
  while  (orderings.size () < numOfOrderings)
  {
    workList->RandomizeOrder ();
    FeatureVectorListPtr  ordering = workList->StratifyAmoungstClasses (mlClasses, -1, numOfFolds, log);
    orderings.push_back (ordering);
  }

  delete  workList;
}  /* CreateOrderings */




void  Orderings::DeleteOrderings ()
{
  while  (orderings.size () > 0)
  {
    FeatureVectorListPtr  ordering = orderings.back ();  
    orderings.pop_back ();
    delete  ordering;
  }
}  /* DeleteOrderings */


void  Orderings::Load (RunLog&  log)
{
  bool  successful = true;
  Load (indexFileName, successful, log);
  if  (!successful)
    valid = false;

}  /* Load */



void  Orderings::Load (const KKStr&  _indexFileName,
                       bool&         successful,
                       RunLog&       log
                      )
{
  log.Level (10) << endl << endl << endl << endl;

  log.Level (10) << "Orderings::Load  indexFileName[" << _indexFileName << "]" << endl;

  successful = true;

  DeleteOrderings ();
  indexFileName = _indexFileName;
  if  (indexFileName.Empty ())
  {
    // We have a problem,  no way of creating a file name.
    log.Level (-1) << endl << endl
                   << "Orderings::Load    *** ERROR ***    No Index File Name Specified." << endl
                   << endl;
    successful = false;
    valid      = false;
    return;
  }

  ifstream i (indexFileName.Str ());
  if  (!i.is_open ())
  {
    successful = false;
    valid      = false;
    log.Level (-1) << endl << endl
                   << "Orderings::Load    *** ERROR ***   Opening[" << indexFileName << "]" << endl
                   << endl;
    return;
  }

  KKStr  line;

  {
    // Make sure 1st line is Orderings.
    do  {
      i >> line;
      cout << "Beginning Orderings Line[" << line << "]" << endl;
    }  while  ((!i.eof ())  && (line.Empty ()));

    line.Upper ();
    if  (line != "//ORDERINGS")
    {
      log.Level (-1) << endl << endl
                     << "Orderings::Load    *** ERROR ***     Invalid File Heading[" << indexFileName << "]" << endl
                     << "                   First Line[" << line << "]" << endl
                     << "                   Expected  [//Orderings]" << endl
                     << endl;
      successful = false;
      valid      = false;
      i.close ();
      return;
    }
  }
  
  bool  headerFields = true;
  KKStr  field;
  numOfOrderings = 0;
  numOfFolds = 0;
  while  (headerFields  &&  (!i.eof ()))
  {
    i >> field;
    field.Upper ();

    if  (field == "//FEATUREFILENAME")
      i >> featureFileName;

    else if  (field == "//NUMOFORDERINGS")
      i >> numOfOrderings;

    else if  (field == "//NUMOFFOLDS")
      i >> numOfFolds;

    else if  (field == "//ENDOFHEADER")
      headerFields = false;
  }

  if  (numOfOrderings < 1)
  {
    log.Level (-1) << endl << endl
                   << "Orderings::Load    *** ERROR ***  Invalid Header Field[" << indexFileName << "]" << endl
                   << "                                  NumOfOrderings[" << numOfOrderings << "]" << endl
                   << endl;
    successful = false;
    valid      = false;
    i.close ();
    return;
  }


  log.Level (10) << "Orderings::Load  featureFileName[" << featureFileName << "]" << endl;
  log.Level (10) << "Orderings::Load  numOfOrderings [" << numOfOrderings << "]" << endl;
  log.Level (10) << "Orderings::Load  numOfFolds     [" << numOfFolds << "]" << endl;

  kkuint32  orderingIDX = 0;

  for  (orderingIDX = 0;  orderingIDX < numOfOrderings;  orderingIDX++)
  {
    vector<bool>  indexLoaded (data->QueueSize (), false);

    kkint32  imagesInOrdering = 0;

    FeatureVectorListPtr  ordering = new FeatureVectorList (fileDesc, false);
    orderings.push_back (ordering);

    {
      // Get Ordering Header Record.
      i >> field;
      field.Upper ();

      if  ((i.eof ())  ||  (field != "//ORDERINGNUM"))
      {
        log.Level (-1) << endl << endl
                       << "Orderings::Load    *** ERROR ***  Index File [" << indexFileName << "] Incomplete." << endl
                       << endl;
        successful = false;
        valid      = false;
        i.close ();
        return;
      }

      kkint32  orderingNum;
      i >> orderingNum;

      if  (orderingNum != kkint32 (orderingIDX))
      {
        log.Level (-1) << endl << endl
                       << "Orderings::Load  *** ERROR ***  Orderings out of sequence." << endl
                       << "                 Expected[" << orderingIDX << "]" << endl
                       << "                 Found   [" << orderingNum << "]" << endl
                       << endl;
        successful = false;
        valid      = false;
        i.close ();
        return;
      }

      i >> field;  // Load "Count" label.
      field.Upper ();
      if  (field != "COUNT")
      {
        log.Level (-1) << endl << endl
                       << "Orderings::Load  *** ERROR ***  Orderings out of sequence." << endl
                       << "                 Missing Count Label for Ordering[" << orderingNum << "]" << endl
                       << endl;
        successful = false;
        valid      = false;
        i.close ();
        return;
      }

      i >> imagesInOrdering;
    }

    i >> field;
    field.Upper ();

    while  ((field != "//ENDOFORDERING") &&  (!i.eof ()))
    {
      kkuint32  imageIdx = field.ToUint32 ();

      if  (imageIdx >= data->QueueSize ())
      {
        log.Level (-1) << endl << endl
                       << "Orderings::Load  *** ERROR ***  Invalid Index Encountered." << endl
                       << "                 Index        [" << imageIdx           << "]" << endl
                       << "                 Size of Data [" << data->QueueSize () << "]" << endl
                       << endl;
        successful = false;
        valid      = false;
        i.close ();
        return;
      }

      if  (indexLoaded[imageIdx])
      {
        log.Level (-1) << endl << endl
                       << "Orderings::Load  *** ERROR ***  Duplicate Index in same ordering." << endl
                       << "                 Index       [" << imageIdx     << "]" << endl
                       << "                 OrderingNum [" << orderingIDX  << "]" << endl
                       << endl;
        successful = false;
        valid      = false;
        i.close ();
        return;
      }

      indexLoaded[imageIdx] = true;
      ordering->PushOnBack (data->IdxToPtr (imageIdx));

      i >> field;
      field.Upper ();
    }

    if  (ordering->QueueSize () < imagesInOrdering)
    {
      log.Level (-1) << endl << endl
                     << "Orderings::Load  *** ERROR ***  Missing Indexes for Ordering[" << orderingIDX << "]." << endl
                     << "                 Expected [" << imagesInOrdering       << "]" << endl
                     << "                 Found    [" << ordering->QueueSize () << "]" << endl
                     << endl;
      successful = false;
      valid      = false;
      i.close ();
      return;
    }

    if  (field == "//ENDOFORDERING")
    {
      kkint32  endOfOrderingNum;
      i >> endOfOrderingNum;
    }

    cout << "Ordering " << orderingIDX << " of " << numOfOrderings << " Loaded" << endl;
  }

  if  (orderings.size () != numOfOrderings)
  {
    log.Level (-1) << endl << endl
                   << "Orderings::Load  *** ERROR ***  Not orderings were loaded." << endl
                   << "                 Expected     [" << numOfOrderings          << "]" << endl
                   << "                 Number Found [" << (kkuint32)orderings.size () << "]" << endl
                   << endl;
    successful = false;
    valid      = false;
    i.close ();
    return;
  }

  i.close ();
}  /* LoadIndexsFromFile */



void  Orderings::Save (const KKStr&  _indexFileName,
                       RunLog&       _log
                      )
{
  if  (indexFileName.Empty ())
  {
    // We have a problem,  no way of creating a file name.
    _log.Level (-1) << endl << endl
                   << "Orderings::Save    *** ERROR ***    No Index File Name Specified." << endl
                   << endl;
    osWaitForEnter ();
    exit (-1);
    return;
  }

  indexFileName = _indexFileName;
  Save ();
}  /* Save */



void  Orderings::Save ()
{
  // Build an index by relative location in master list data, so that we can 
  // quickly determine index for other orderings.
  map<FeatureVectorPtr,kkint32> index;

  for  (kkuint32 idx = 0;  idx < data->QueueSize ();  idx++)
  {
    FeatureVectorPtr  example = data->IdxToPtr (idx);
    index.insert (pair<FeatureVectorPtr,kkint32>(example, idx));
  }

  KKStr  tempName = featureFileName;
  if  (tempName.Empty ())
    tempName = indexFileName;

  map<FeatureVectorPtr,kkint32>::const_iterator  indexIDX;

  ofstream  o (indexFileName.Str ());

  o << "//Orderings"                                        << endl;
  o << "//FeatureFileName" << "\t" << tempName              << endl;
  o << "//NumOfOrderings"  << "\t" << numOfOrderings        << endl;
  o << "//NumOfFolds"      << "\t" << numOfFolds            << endl;
  o << "//DateCreated"     << "\t" << osGetLocalDateTime () << endl;
  o << "//EndOfHeader"                                      << endl;

  for  (kkuint32 orderingIDX = 0;  orderingIDX < orderings.size ();  orderingIDX++)
  {
    FeatureVectorListPtr  ordering = orderings[orderingIDX];
    o << "//OrderingNum" << "\t" << orderingIDX << "\t" << "Count" << "\t" << ordering->QueueSize () << endl;

    for  (auto example: *ordering)
    {
      indexIDX = index.find (example);
      if  (indexIDX == index.end ())
      {
        // We have a very serious problem,  for some reason we 
        // could not locate the FeatureVector object in the master list.

        KKStr errMsg;
        errMsg << "Orderings::Save  ***ERROR***   FileName[" << indexFileName << "]   Could not locate Image in data list";
        cerr << endl << errMsg << endl << endl;
        throw KKException (errMsg);
      }

      o << indexIDX->second << endl;
    }
    o << "//EndOfOrdering" << "\t" << orderingIDX << endl;

    cout << "Saved " << orderingIDX << " of " << orderings.size () << endl;
  }

  o.close ();
}  /* Save */



FeatureVectorListPtr  Orderings::Ordering (kkuint32  orderingIdx)  const
{
  if  ((orderingIdx >= orderings.size ()))
  {
    KKStr errMsg;
    errMsg << "Orderings::Ordering  ***ERROR***  Index Out Of Range;  Number Of Orderings [" << (kkuint32)orderings.size ()  << "]   OrderingIdx [" << (kkuint32)orderingIdx        << "]";
    cerr << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  return  orderings[orderingIdx];
}  /* Ordering */
