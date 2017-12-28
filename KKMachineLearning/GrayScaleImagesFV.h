#ifndef  _LASRCOSFEATUREVECTOR_
#define  _LASRCOSFEATUREVECTOR_

/**
 *@class KKMLL::GrayScaleImagesFV
 *@brief  Specialized version of KKMLL::FeatureVector that will be used 
 *to represent the features of a Shrimp.
 *@author  Kurt Kramer
 *@details
 * Used for the representation of a Single Plankton Image.  You create an instance of this object for 
 * each single image you need to keep track of.  There is a specialized version of KKMLL::FeatureFileIO 
 * calculated  KKMLL::FeatureFileIOKK that is used to write and read feature Data files. What makes this 
 * class of KKMLL::FeatureVector special are the additional fields that are Plankton specific such as 
 * centroidCol, centroidRow, latitude, longitude, numOfEdgePixels, centroid within SIPPEER file 
 * sfCentroidCol, sfCentroidRow and version.<p>
 *
 * The version number field is supposed to indicate which feature calculation routines were used.  This 
 * way if there are changes the way features are calculated it can be detected during runtime if the features
 * are up to date they need to be recomputed.
*/


#include "BMPImage.h"
#include "KKQueue.h"
#include "Raster.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace KKB;
            

//#include "FeatureNumList.h"
#include "FactoryFVProducer.h"
#include "FeatureVector.h"
#include "FileDesc.h"
#include "MLClass.h"
using namespace  KKMLL;


namespace KKMLL 
{
  #ifndef  _FEATURENUMLIST_
  class  FeatureNumList;
  typedef  FeatureNumList*  FeatureNumListPtr;
  #endif
 
  #ifndef  _MLCLASS_
  class  MLClass;
  typedef  MLClass*  MLClassPtr;
  class  MLClassList;
  typedef  MLClassList*  MLClassListPtr;
  #endif


  class  GrayScaleImagesFV:  public  FeatureVector 
  {
  public:
    typedef  GrayScaleImagesFV*  GrayScaleImagesFVPtr;
    typedef  GrayScaleImagesFV const *  GrayScaleImagesFVConstPtr;

    typedef  KKB::uchar  uchar;

    GrayScaleImagesFV (kkint32  _numOfFeatures);

    GrayScaleImagesFV (const GrayScaleImagesFV&  _fv);



    /**
     *@brief  Smart copy constructor that will detect the underlying type of the source instance.
     *@details
     *@code
     ** This constructor will detect what the underlying type of 'featureVector' is.
     ** If (underlying type is a 'GrayScaleImagesFV' object)  then
     **   | Information that is particular to a 'GrayScaleImagesFV' object will be extracted
     **   | from the 'FeatureVector' object.
     ** else
     **   | Info that is particular to a 'GrayScaleImagesFV' object will be set to default 
     **   | values.
     *@endcode
     */
    GrayScaleImagesFV (const FeatureVector&  featureVector);

    virtual  ~GrayScaleImagesFV ();

    virtual  GrayScaleImagesFVPtr  Duplicate ()  const;

    // Access Methods.
    void  CentroidCol      (float    _centroidCol)      {centroidCol      = _centroidCol;}
    void  CentroidRow      (float    _centroidRow)      {centroidRow      = _centroidRow;}
    void  NumOfEdgePixels  (kkint32  _numOfEdgePixels)  {numOfEdgePixels  = _numOfEdgePixels;}


    float   CentroidCol      () const  {return  centroidCol;}    // Centroid with respect to image
    float   CentroidRow      () const  {return  centroidRow;}    //  ""    ""    ""    ""    ""
    kkint32 NumOfEdgePixels  () const  {return  numOfEdgePixels;}


  private:
    static  RasterListPtr  calcImages;

    float     centroidCol;     /**<  centroid Column with just respect to image. */
    float     centroidRow;     /**<  centroid Row with just respect to image. */
    kkint32   numOfEdgePixels;

  };  /* GrayScaleImagesFV */

  typedef  GrayScaleImagesFV::GrayScaleImagesFVPtr  GrayScaleImagesFVPtr;

  typedef  GrayScaleImagesFV::GrayScaleImagesFVConstPtr  GrayScaleImagesFVConstPtr;

#define  _GrayScaleImagesFV_Defined_



  class  GrayScaleImagesFVList:  public FeatureVectorList
  {
  public: 
    typedef  GrayScaleImagesFVList*  GrayScaleImagesFVListPtr;

    GrayScaleImagesFVList (FileDescConstPtr  _fileDesc,
                           bool              _owner
                          );

  private:
    /**
     *@brief  Will create a duplicate List of examples, in the same order.  If the source 
     *        'examples' owns its entries, then new duplicate entries will be created, and will 
     *        own them otherwise will only get pointers to existing instances in 'examples'.
     */
    GrayScaleImagesFVList (const GrayScaleImagesFVList&  examples);


  public:

    /**
     *@brief Creates a duplicate List of examples, in the same order, and depending on '_owner' will 
     *       create new instances or just point to the existing one.
     *@details 
     *@code 
     *  If '_owner' = true 
     *     Create new instances of contents and own them.  
     *  else if  'owner' = false, 
     *     Copy over pointers to existing instances.  
     *@endcode
     *@param[in]  _examples   The list of 'GrayScaleImagesFV' that is to be copied.
     *@param[in]  _owner      If 'true' new instances of 'GrayScaleImagesFV' instances will be created
     *                        and this new list will own them and if 'false' will just point to
     *                        the existing instances and not own the.
     */
    GrayScaleImagesFVList (const GrayScaleImagesFVList&  _examples,
                             bool                        _owner
                            );


    /**
     *@brief Creates a duplicate List of examples, in the same order, and depending on '_owner' will 
     *       create new instances or just point to the existing one.
     *@details 
     *@code 
     *  If '_owner' = true 
     *     Create new instances of contents and own them.  
     *  else if  'owner' = false, 
     *     Copy over pointers to existing instances.  
     *@endcode
     * If any of the existing instances do not have an underlying class of GrayScaleImagesFV;  
     * the function will throw an exception.
     *
     *@param[in]  _examples   The list of 'GrayScaleImagesFV' that is to be copied.
     *@param[in]  _owner      If 'true' new instances of 'GrayScaleImagesFV' instances will be created
     *                        and this new list will own them and if 'false' will just point to
     *                        the existing instances and not own the.
     */
    GrayScaleImagesFVList (const FeatureVectorList&  featureVectorList,
                           bool                      _owner
                          );


    /**
     *@brief  Constructor that will extract a list of feature vectors for all the image files in the 
     *        specified directory.
     *@details
     * Will scan the directory _dirName for any image files.  For each image found a new instance of GrayScaleImagesFV
     * will be created who's features will be derived from the image. These GrayScaleImagesFV' objects will be 
     * assigned the class specified by '_mlClass'. A new data file containing the extracted features will be 
     * saved in fileName.
     *
     *@param[in] _fvProducerFactory
     *@param[in] _mlClass   Class to assign to new 'GrayScaleImagesFV' objects.
     *@param[in] _dirName   Directory to scan for examples.
     *@param[in] _fileName  Name of file to contain the extracted feature data.  Will be of the Raw format.
     *@param[in] _log       Log file to write messages to.
     */
    GrayScaleImagesFVList (FactoryFVProducerPtr  _fvProducerFactory,
                           MLClassPtr            _mlClass,
                           KKStr                 _dirName,
                           KKStr                 _fileName,
                           RunLog&               _log
                          );



    /**
     *@brief  constructor that will create a list of examples from _examples that are assigned one of the 
     *        classes listed in _mlClasses.
     *@details
     *   Will Create a list of examples that are a subset of the ones in _examples.  The subset will
     *   consist of the examples who's mlClass is one of the  ones in mlClasses.  We will not own
     *   any the contents only point to the ones already in _examples.
     *@param[in] _mlClasses  List of classes that we are interested in.
     *@param[in] _examples        Source examples that we want to scan.
     */
    GrayScaleImagesFVList (MLClassList&            _mlClasses,
                           GrayScaleImagesFVList&  _examples
                         );


    /**
     @brief
     @details
        This constructor is meant to create a list of 'GrayScaleImagesFV' objects from the FeatureVector
        objects contained in featureVectorList.
     @code
     If  'featureVectorList'  owns its contents (that is 'featureVectorList.Owner () == true'  then
        |  We will create new Instances of 'GrayScaleImagesFV' objects that we will own.
        |  The underlying class of the 'FeatureVector' objects will be converted to a
        |  'GrayScaleImagesFV'  class.
     else
        |  all the 'FeatureVector' objects in 'featureVectorList' must have an underlying class of
        |  'GrayScaleImagesFV'.  If one or more do not then the program will halt with a message to
        |  the log.
    @endcode
    */
    GrayScaleImagesFVList (const FeatureVectorList&  featureVectorList);




    virtual  ~GrayScaleImagesFVList ();


    void                       AddQueue (GrayScaleImagesFVList&  imagesToAdd);

    GrayScaleImagesFVPtr       BackOfQueue ();

    GrayScaleImagesFVPtr       BinarySearchByName (const KKStr&  _imageFileName)  const;

    virtual
    GrayScaleImagesFVListPtr   Duplicate (bool _owner)  const;

    virtual
    GrayScaleImagesFVListPtr   DuplicateListAndContents ()  const;

    GrayScaleImagesFVListPtr   ExtractDuplicatesByRootImageFileName ();

    GrayScaleImagesFVListPtr   ExtractExamplesForAGivenClass (MLClassPtr  _mlClass,
                                                            kkint32     _maxToExtract = -1,
                                                            float       _minSize      = -1.0f
                                                           )  const;


    GrayScaleImagesFVPtr       IdxToPtr (kkint32 idx) const;

    GrayScaleImagesFVPtr       LookUpByImageFileName (const KKStr&  _imageFileName)  const;

    GrayScaleImagesFVPtr       LookUpByRootName (const KKStr&  _rootName);

    virtual
    GrayScaleImagesFVListPtr   ManufactureEmptyList (bool _owner)  const;

    
    /**
     *@brief  Using list of ImageFileNames in a file('orderedFileName') create a new GrayScaleImagesFVList instance 
     * with examples in order based off contents of file. If error occurs will return NULL.
     */
    GrayScaleImagesFVListPtr   OrderUsingNamesFromAFile (const KKStr&  orderedFileName,
                                                         RunLog&       log
                                                        );

    GrayScaleImagesFVPtr       PopFromBack ();

    void                       RecalcFeatureValuesFromImagesInDirTree (FactoryFVProducerPtr  fvProducerFactory,  
                                                                       const KKStr&          rootDir,
                                                                       bool&                 successful,
                                                                       RunLog&               log
                                                                      );

    GrayScaleImagesFVListPtr   StratifyAmoungstClasses (kkint32  numOfFolds,
                                                        RunLog&  log
                                                       );


    GrayScaleImagesFVListPtr   StratifyAmoungstClasses (MLClassListPtr  mlClasses,
                                                        kkint32         maxImagesPerClass,
                                                        kkint32         numOfFolds,
                                                        RunLog&          log
                                                       );


    class  const_iterator
    {
    private:
      FeatureVectorList::const_iterator  idx;
    
    public:
      const_iterator ():
          idx ()
      {
      }


      const_iterator (const const_iterator&  ivConst_Iterator):
          idx (ivConst_Iterator.idx)
      {
      }


      const_iterator (const FeatureVectorList::const_iterator&  fvConst_Iterator):
          idx (fvConst_Iterator)
      {
      }


      const_iterator (const FeatureVectorList::iterator&  fvIterator):
          idx (fvIterator)
      {
      }


      GrayScaleImagesFVPtr  operator*()
      {
        return  (const GrayScaleImagesFVPtr)*idx;
      }


      const_iterator&   operator= (const const_iterator&  right)
      {
        idx = right.idx;
        return  *this;
      }



      const_iterator&   operator= (const FeatureVectorList::iterator&  right)  
      {
        idx = right;
        return *this;
      }


      const_iterator&   operator= (const FeatureVectorList::const_iterator&  right)  
      {
        idx = right;
        return *this;
      }

      
      bool  operator!= (const const_iterator&  right)  const
      {
        return  idx != right.idx;
      }


      bool  operator!= (const FeatureVectorList::iterator&  right)  const
      {
        return  idx != (FeatureVectorList::const_iterator)right;
      }


      bool  operator!= (const FeatureVectorList::const_iterator&  right)  const
      {
        return  idx != right;
      }


      bool  operator== (const const_iterator&  right)  const
      {
        return  idx == right.idx;
      }


      bool  operator== (const FeatureVectorList::iterator&  right)  const
      {
        return  (idx == right);
      }


      const_iterator&   operator++ ()
      {
        ++idx;
        return  *this;
      }
    };  /* const_iterator */


    
    class  iterator
    {
    private:
      FeatureVectorList::iterator  idx;
    
    public:
      iterator ():
          idx ()
      {
      }

      iterator (const iterator&  iter):
          idx (iter.idx)
      {
      }


      iterator (const FeatureVectorList::iterator&  iter):
          idx (iter)
      {
      }


      GrayScaleImagesFVPtr  operator*()
      {
        return  (GrayScaleImagesFVPtr)*idx;
      }

      iterator&   operator= (const iterator&  right)
      {
        idx = right.idx;
        return  *this;
      }

      bool  operator!= (const iterator&  right)  const
      {
        return  idx != right.idx;
      }

      bool  operator== (const iterator&  right)  const
      {
        return  idx == right.idx;
      }

      iterator   operator++ (int)
      {
        iterator  result (*this);
        ++idx;
        return  result;
      }
    };

  private:
    void   FeatureExtraction (FactoryFVProducerPtr  _fvProducerFactory,
                              KKStr                 _dirName, 
                              KKStr                 _fileName, 
                              MLClassPtr            _mlClass,
                              RunLog&               _log
                             );



  };  /* GrayScaleImagesFVList */


  typedef  GrayScaleImagesFVList::GrayScaleImagesFVListPtr  GrayScaleImagesFVListPtr;

#define  _GrayScaleImagesFVList_Defined_


}  /* namespace LarcosCounterUnManaged */

#endif

