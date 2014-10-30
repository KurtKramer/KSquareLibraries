#if  !defined(_FEATUREVECTORPRODUCER_)
#define  _FEATUREVECTORPRODUCER_

/**
 *@class  KKMachineLearning::FeatureVectorProducer
 *@brief  A abstract class that is meant to compute a FeatureVector from a source image.
 *@details Applications that want to utilize this library will either use one of the 
 *provided "FeatureVectorProducer" derived classes or supply their own.  This class will 
 *be responsable for computing a FeatureVector from a supplied Image.
 *
 *Each FeatureVectorProducer derived class will need to have a unique name that will be supplied when 
 *constructed. This name will be used later when it is required to locate the appropriate 
 *FeatureVectorProducer to utilize.
 *@see FeatureVectorList
 *@see PostLarvaeFV
 *@see FeatureFileIO
 */


#include "RunLog.h"
#include "Raster.h"

namespace KKMachineLearning 
{

#if  !defined(_FeatureVector_Defined_)
  class  FeatureVector;
  typedef  FeatureVector*  FeatureVectorPtr;
#endif


#if  !defined(_FileDesc_Defined_)
  class FileDesc;
  typedef  FileDesc*        FileDescPtr;
  typedef  FileDesc* const  FileDescConstPtr;
#endif



  class  FeatureVectorProducer
  {
  public:
    typedef  FeatureVectorProducer*  FeatureVectorProducerPtr;

    FeatureVectorProducer (const KKStr&  _name,
                          FileDescPtr   _fileDesc
                         );

    virtual ~FeatureVectorProducer ();

    virtual  FeatureVectorPtr  ComputeFeatureVector (RasterPtr  image,
                                                     RunLog&    runLog
                                                    ) = 0;


    virtual  FeatureVectorProducerPtr  DuplicateInstance ()  const = 0;

    /**  @brief  Returns the 'type_info' of the Feature Vector that this instance of 'FeatureComputer' creates. */
    virtual  const type_info*  FeatureVectorTypeId () const = 0;


    /**  @brief  Returns back a "FileDesc" instance that describes the feastures that this instance of 'FeatureVectorProducer' creates.  */
    FileDescConstPtr  FileDesc () const {return  fileDesc;}


    /**  @brief  Returns a kkint16 description of the FeatureVector which can be used as part/all of a File or Direecttory name.  */
    const KKStr&  Name () const {return name;};


    //  Feature description related methods.
    kkuint32  FeatureCount ()  const;

    const KKStr&  FeatureName (kkuint32  fieldNum)  const;

    kkuint32  MaxNumOfFeatures ()     {return  FeatureCount ();}  /**<  Same as FeatureCount  */

  protected:
    void  SetFileDesc (FileDescPtr  _fileDesc);

  private:
    FileDescPtr  fileDesc;
    KKStr        name;

    static  bool  atExitDefined;
  };   /* FeatureVectorProducer */

  typedef  FeatureVectorProducer::FeatureVectorProducerPtr  FeatureVectorProducerPtr;

#define  _FeatureVectorProducer_Defined_

}  /* KKMachineLearning */



#endif