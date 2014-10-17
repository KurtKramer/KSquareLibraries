#ifndef  _IMAGEFEATURESDATAINDEXED_
#define _IMAGEFEATURESDATAINDEXED_


#include  "RBTree.h"
#include  "KKStr.h"



namespace KKMachineLearning
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

    ImageFeaturesDataIndexed (FeatureVectorList&  images);


    void              RBInsert (FeatureVectorPtr  image);

    FeatureVectorPtr  GetEqual (FeatureVectorPtr  image);

  private:
  };


  typedef  ImageFeaturesDataIndexed*  ImageFeaturesDataIndexedPtr;






  class ImageFeaturesNodeKey
  {
  public:
    ImageFeaturesNodeKey (FeatureVectorPtr  _image);

    bool  operator== (const ImageFeaturesNodeKey& rightNode)  const;
    bool  operator<  (const ImageFeaturesNodeKey& rightNode)  const;
    bool  operator>  (const ImageFeaturesNodeKey& rightNode)  const;

    int32  CompareTwoImages (const FeatureVectorPtr i1,
                           const FeatureVectorPtr i2
                          )  const;

    FeatureVectorPtr  image;
  };  /* ImageFeaturesNodeKey */




  class  ImageDataTreeEntry
  {
  public:
    ImageDataTreeEntry (FeatureVectorPtr  _image);

    const ImageFeaturesNodeKey&  NodeKey () const  {return  nodeKey;}

    FeatureVectorPtr  Image ()  {return  image;}


  private:
    FeatureVectorPtr      image;
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


}  /* namespace KKMachineLearning */


#endif
