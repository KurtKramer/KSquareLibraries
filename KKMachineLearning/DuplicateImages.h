#ifndef  _DUPLICATEIMAGES_
#define  _DUPLICATEIMAGES_

/** 
 *@class KKMLL::DuplicateImages
 *@author  Kurt Kramer
 *@brief   Detects duplicate images in a given FeaureVectorList objects.
 *@details  Will derive a list of duplicate FeatureVector objects in a given list.  It will 
 *          use both the Image File Name and feature data to detect duplicates.  A duplicate
 *          can be detected in two ways.  If two or more entries have the same ExampleFileName  
 *          or FeatureData.
 *         
 *          The simplest way to use this object is to create an instance with a FeatureVectorList 
 *          object that you are concerned with.  Then call the method DupExamples (), which will 
 *          return the list of duplicates found via a structure called DuplicateImageList.
 */


#include  "RunLog.h"
#include  "FeatureVector.h"


namespace KKMLL
{
  class  DuplicateImageList;
  typedef  DuplicateImageList*  DuplicateImageListPtr;

  class  DuplicateImage;
  typedef  DuplicateImage*  DuplicateImagePtr;


  #ifndef  _IMAGEFEATURESDATAINDEXED_
  class  ImageFeaturesDataIndexed;
  typedef  ImageFeaturesDataIndexed*  ImageFeaturesDataIndexedPtr;
  #endif


  #ifndef  _FEATUREVECTOR_
  class FeatureVector;
  typedef  FeatureVector* FeatureVectorPtr;
  class FeatureVectorList;
  typedef  FeatureVectorList*  FeatureVectorListPtr;
  #endif

  #ifndef  _IMAGEFEATURESNAMEINDEXED_
  class  ImageFeaturesNameIndexed;
  typedef  ImageFeaturesNameIndexed* ImageFeaturesNameIndexedPtr;
  #endif




  class DuplicateImages
  {
  public:
    /**
     *@brief  You would use this instance to search for duplicates in the list of 'examples'.
     *@details  You can still call 'AddExamples' and 'AddSingleExample'; 
     */
    DuplicateImages (FeatureVectorListPtr  _examples,
                     RunLog&               _log
                    );


    DuplicateImages (FileDescConstPtr  _fileDesc,
                     RunLog&           _log
                    );

    ~DuplicateImages ();


    /** @brief  Will add all the examples; be careful of ownership. */
    bool                   AddExamples (FeatureVectorListPtr  examples);

    /**
     *@brief  Add one more FeatureVector to the list.
     *@details  Will add one more example to list and if it turns out to be a duplicate will 
     *          return pointer to a "DuplicateImage" structure that will contain a list of 
     *          all images that it is duplicate to. If no duplicate found will then return
     *          a NULL pointer.      
     *@param[in]  example  FeatureVecvtor that you want to add to the list.
     */
    DuplicateImagePtr      AddSingleExample (FeatureVectorPtr  example);

    DuplicateImageListPtr  DupExamples        ()  const {return dupExamples;}

    kkint32                DuplicateCount     ()  const {return duplicateCount;}
    kkint32                DuplicateDataCount ()  const {return duplicateDataCount;}
    kkint32                DuplicateNameCount ()  const {return duplicateNameCount;}

    bool                   DuplicatesFound ()  const;

    bool                   ExampleInDetector (FeatureVectorPtr    fv);

    FeatureVectorListPtr   ListOfExamplesToDelete ();

    void                   PurgeDuplicates (FeatureVectorListPtr  examples,
                                            bool                  allowDupsInSameClass,
                                            std::ostream*         report
                                           );  /**<  if not equal NULL will list examples being purged. */

    void                   ReportDuplicates (std::ostream&  o);


  private:
    void  FindDuplicates (FeatureVectorListPtr  examples);  /**< Used to build duplicate list from current contents of examples. */

    kkint32                      duplicateCount;
    kkint32                      duplicateDataCount;
    kkint32                      duplicateNameCount;
    DuplicateImageListPtr        dupExamples;
    ImageFeaturesDataIndexedPtr  featureDataTree;
    FileDescConstPtr             fileDesc;
    RunLog&                      log;
    ImageFeaturesNameIndexedPtr  nameTree;
  };


  typedef  DuplicateImages*  DuplicateImagesPtr;




  class  DuplicateImage
  {
  public:
    DuplicateImage (FileDescConstPtr  _fileDesc,
                    FeatureVectorPtr  _image1,
                    FeatureVectorPtr  _image2
                   );

    ~DuplicateImage ();

    void  AddADuplicate (FeatureVectorPtr  example);

    bool  AllTheSameClass ();

    bool  AlreadyHaveExample (FeatureVectorPtr example);

    FeatureVectorListConstPtr  DuplicatedImages ()  {return &duplicatedImages;}

    FeatureVectorPtr  FirstExampleAdded  ()  {return firstImageAdded;}

    FeatureVectorPtr  ExampleWithSmallestScanLine (); 

  private:
    FileDescConstPtr   fileDesc;
    FeatureVectorList  duplicatedImages;
    FeatureVectorPtr   firstImageAdded;
  };





  typedef  DuplicateImage*  DuplicateImagePtr;




  class  DuplicateImageList: public KKQueue<DuplicateImage>
  {
  public:
    DuplicateImageList (bool _owner);
    ~DuplicateImageList ();

    DuplicateImagePtr  LocateByImage (FeatureVectorPtr  example);

  private:
  };

  typedef  DuplicateImageList*  DuplicateImageListPtr;

}  /* namespace KKMLL */

#endif

