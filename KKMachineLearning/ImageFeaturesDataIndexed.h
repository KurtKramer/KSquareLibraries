#ifndef  _IMAGEFEATURESDATAINDEXED_
#define _IMAGEFEATURESDATAINDEXED_


#include  "RBTree.h"
#include  "KKStr.h"



namespace KKMLL
{


  #ifndef  _FeatureVector_Defined_
  class  FeatureVector;
  typedef  FeatureVector*  FeatureVectorPtr;
  #endif


  #ifndef  _FeatureVectorList_Defined_
  class  FeatureVectorList;
  typedef  FeatureVectorList*  FeatureVectorListPtr;
  #endif



  class  ImageDataTreeEntry;
  class  ExtractFeatureData;
  class  ImageFeaturesNodeKey;

  class  ImageFeaturesDataIndexed: public  RBTree<ImageDataTreeEntry, ExtractFeatureData, ImageFeaturesNodeKey>
  {
  public:
    ImageFeaturesDataIndexed ();

    ImageFeaturesDataIndexed (const FeatureVectorList&  examples);


    void              RBInsert (FeatureVectorPtr  example);

    FeatureVectorPtr  GetEqual (FeatureVectorPtr  example);

  private:
  };


  typedef  ImageFeaturesDataIndexed*  ImageFeaturesDataIndexedPtr;






  class ImageFeaturesNodeKey
  {
  public:
    ImageFeaturesNodeKey (FeatureVectorPtr  _example);

    bool  operator== (const ImageFeaturesNodeKey& rightNode)  const;
    bool  operator<  (const ImageFeaturesNodeKey& rightNode)  const;
    bool  operator>  (const ImageFeaturesNodeKey& rightNode)  const;

    kkint32  CompareTwoExamples (const FeatureVectorPtr i1,
                                 const FeatureVectorPtr i2
                              )  const;

    FeatureVectorPtr  example;
  };  /* ImageFeaturesNodeKey */




  class  ImageDataTreeEntry
  {
  public:
    ImageDataTreeEntry (FeatureVectorPtr  _example);

    const ImageFeaturesNodeKey&  NodeKey () const  {return  nodeKey;}

    FeatureVectorPtr  Example ()  {return  example;}


  private:
    FeatureVectorPtr      example;
    ImageFeaturesNodeKey  nodeKey;
  };




  class  ExtractFeatureData
  {
  public:
     const ImageFeaturesNodeKey&  ExtractKey (ImageDataTreeEntry*  entry)
     {
        return  entry->NodeKey ();
     }
  };


}  /* namespace KKMLL */


#endif
