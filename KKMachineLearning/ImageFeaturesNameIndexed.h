#ifndef  _IMAGEFEATURESNAMEINDEXED_
#define  _IMAGEFEATURESNAMEINDEXED_


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



  class  ExtractExampleFileName;


  class  ImageFeaturesNameIndexed: public  RBTree<FeatureVector, ExtractExampleFileName, KKStr>
  {
  public:
    ImageFeaturesNameIndexed ();

    ImageFeaturesNameIndexed (const FeatureVectorList&  images);

  private:
  };


  typedef  ImageFeaturesNameIndexed*  ImageFeaturesNameIndexedPtr;





  class  ExtractExampleFileName
  {
  public:
     KKStr  ExtractKey (FeatureVectorPtr  example);
  };

}  /* namespace KKMLL */


#endif

