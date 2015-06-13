#ifndef  _ORDERINGS_
#define  _ORDERINGS_


/**
 *@class  KKMLL::Orderings
 *@author  Kurt Kramer
 *@brief  Used to maintain multiple orderings of a single list of FeatureVector objects.
 *@details
 * Will maintain multiple orderings of a single FeatureVectorList.  These orderings will be saved in 
 * a text file for recall later. This will allow the user to be able to repeat experiments using the 
 * same ordering of data again. The Idea is that the 1st time this orderings is created the order 
 * will be randomly driven with Stratification by Class.  This ordering will then be saved in data 
 * Files for future recall.  More than one order can be maintained for a single list. An example of 
 * a good use of this is RandomSplits. 
 */



#include  "FileDesc.h"
#include  "RunLog.h"
#include  "KKStr.h"


namespace  KKMLL
{


#ifndef  _FeatureVector_Defined_
class  FeatureVector;
typedef  FeatureVector*  FeatureVectorPtr;
#endif


#ifndef  _FeatureVectorList_Defined_
class  FeatureVectorList;
typedef  FeatureVectorList*  FeatureVectorListPtr;
#endif


#ifndef  _MLCLASS_
class  MLClass;
typedef  MLClass*  MLClassPtr;
class  MLClassList;
typedef  MLClassList*  MLClassListPtr;
#endif


#ifndef  _FILEDESC_
class  FileDesc;
typedef  FileDesc*  FileDescPtr;
class  FileDescList;
typedef  FileDescList*  FileDescListPtr;
#endif



class  Orderings
{
public:

  typedef  Orderings*  OrderingsPtr;

  /**
   *@brief Constructs Orderings object from ImageFeatursList object.
   *@details Use this when an existing list does not exist.  Will create 'numOfOrderings' 
   *         separate lists of 'data' that are randomly ordered and stratified by 
   *         'numOfFolds'.
   *
   *@param[in] _data  ImagFeaturesList object, 
   *@param[in] _numOfOrderings  Number of separate orderings of data need.
   *@param[in] _numOfFolds  Used to help stratify data in each fold.
   */
  Orderings (const FeatureVectorListPtr _data,
             kkuint32                   _numOfOrderings,
             kkuint32                   _numOfFolds,
             RunLog&                    _log
            );


  /**
   *@brief Constructs Orderings of a FeatureVectorList from a previous construction that was saved
   *       in a data file.
   *@details Will load object from the Feature File '_featureFileName'  and retrieve the 
   *         different orderings from a separate index file who's name will be 
   *         osExtention (FeatureFileName) + ".idx". The load routine will validate that all 
   *         FeatureVector instances are accounted for in each ordering. If the index file does not 
   *         exist it then the  'successful' flag will bet to false.  
   *
   *@param[in] _featureFileName  File to load FeatureVector' objects from.  This will be used as 
   *                             master list for 'Orderings'.
   *@param[in] _driver           Feature File driver to utilize.
   *@param[in] _log              Log file to write messages to.
   *@param[in] v                 If flag turns to 'TRUE' then will terminate the load process and return to caller.
   */
  Orderings (const KKStr&      _featureFileName,
             FeatureFileIOPtr  _driver,
             RunLog&           _log,
             bool&             cancelFlag
            );


  /**
   *@brief Constructs a Orderings object from a FeatureLectorList object.  
   *@details  Will use 'data' as master list of FeatureVector objects.The orderings will be loaded 
   *          from 'indexFileName'.  It is expected that the size of the files will match.  The 
   *          load routine will validate that all FeatureVector objects are accounted for in each 
   *          ordering.
   *
   *@param[in] _data  Master List of FeatureVector instances.
   *@param[in] _indexFileName  File where orderings of 'data' are to be loaded from.
   *@param[in] _numOfOrderings
   *@param[in] _numOfFolds
   */
  Orderings (const FeatureVectorListPtr  _data,
             const KKStr&                _indexFileName,
             kkuint32                    _numOfOrderings,
             kkuint32                    _numOfFolds,
             RunLog&                     _log
            );


  /**
   *@brief   Constructs a Orderings objects for a specified FeatureVectorList using a previously 
   *         built Orderings data index file.
   *@details Will use FileName from "data" parameter to derive both 'featureFileName' and 
   *        'indexFileName' using the 'FileName' method from FeatureVectorList.  It is expected 
   *        that a separate index file by the name osDeletExtention (FeatureFileName) + ".idx" 
   *        will exist.  The orderings will be loaded from that file.
   *@param[in]  _data  FeatureVectorList that we want different orderings of.
   */
  Orderings (FeatureVectorListPtr  _data,
             RunLog&               _log
            );

  ~Orderings ();



  /**
   *@brief   Constructs a Orderings object for a specified FeatureVectorList.
   *@details Will use FileName from "_data" parameter to derive both 'featureFileName' and 
   *        'indexFileName' using the 'FileName' method from FeatureVectorList.  If a separate 
   *        Index file does not exist it will randomly create orderings and save the orderings 
   *        in a new Index file.
   *@param[in]  _data  FeatureVectorList that we want different orderings of.
   *@param[in]  _numOfOrderings  Expected number of orderings.
   *@param[in]  _numOfFolds  Number of folds each ordering should be stratified by.
   */
  static
  OrderingsPtr  CreateOrderingsObjFromFileIfAvaliable (const FeatureVectorListPtr  _data,
                                                       kkuint32                    _numOfOrderings,
                                                       kkuint32                    _numOfFolds,
                                                       RunLog&                     _log
                                                      );


  /***************************************************************************/
  /*                            Access Methods                               */
  /***************************************************************************/
  FeatureVectorListPtr     Data            ()  const  {return data;}
  const KKStr&             FeatureFileName ()  const  {return featureFileName;}
  const FileDescPtr        FileDesc        ()  const  {return fileDesc;}
  const MLClassListPtr     MLClasses       ()  const  {return mlClasses;}
  const KKStr&             IndexFileName   ()  const  {return indexFileName;}
  kkuint32                 NumOfFolds      ()  const  {return numOfFolds;}
  kkuint32                 NumOfOrderings  ()  const  {return numOfOrderings;}
  kkuint32                 Size            ()  const  {return (kkuint32)orderings.size ();}
  bool                     Valid           ()  const  {return valid;}




  const
  FeatureVectorListPtr  Ordering (kkuint32  orderingIdx)  const;




  void  Save ();
 
  void  Save (const KKStr&  _indexFileName,
              RunLog&       _log
             );


private:
  void  CreateOrderings (RunLog&  log);
  void  DeleteOrderings ();
  void  Load (RunLog&  log);

  void  Load (const KKStr&  _indexFileName,
              bool&         successful,
              RunLog&       log
             );


  FeatureVectorListPtr           data;
  KKStr                          featureFileName;
  FileDescPtr                    fileDesc;
  MLClassListPtr                 mlClasses;
  KKStr                          indexFileName;
  kkuint32                       numOfFolds;
  kkuint32                       numOfOrderings;
  vector<FeatureVectorListPtr>   orderings;
  bool                           valid;
};


typedef  Orderings::OrderingsPtr  OrderingsPtr;

}  /*  KKMLL  */

#endif
