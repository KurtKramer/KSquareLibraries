#if  !defined(_FEATUREVECTORPRODUCER_)
#define  _FEATUREVECTORPRODUCER_

/**
 *@class  KKMachineLearning::FeatureVectorProducer
 *@brief  A abstract class that is meant to compute a FeatureVector from a source image.
 *@details Applications that want to utilize this library will either use one of the 
 *provided "FeatureVectorProducer" derived classes or supply their own.  This class will 
 *be responsible for computing a FeatureVector from a supplied Image.
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
using namespace  KKB;



namespace KKMachineLearning 
{

#if  !defined(_FeatureVector_Defined_)
  class  FeatureVector;
  typedef  FeatureVector*  FeatureVectorPtr;
#endif


#if  !defined(_FeatureVectorList_Defined_)
  class  FeatureVectorList;
  typedef  FeatureVectorList*  FeatureVectorListPtr;
#endif


#if  !defined(_FileDesc_Defined_)
  class FileDesc;
  typedef  FileDesc*        FileDescPtr;
  typedef  FileDesc* const  FileDescConstPtr;
#endif


#if  !defined(_MLClass_Defined_)
  class  MLClass;
  typedef  MLClass*  MLClassPtr;
#endif


#if  !defined(_FactoryFVProducer_Defined_)
  class  FactoryFVProducer;
  typedef  FactoryFVProducer* FactoryFVProducerPtr;
#endif


  class  FeatureVectorProducer
  {
  public:
    typedef  FeatureVectorProducer*  FeatureVectorProducerPtr;

    FeatureVectorProducer (const KKStr&          _name,
                           FactoryFVProducerPtr  _factory  /**<  Pointer to factory that instantiated this instance. */
                         );

    virtual ~FeatureVectorProducer ();

    virtual  FeatureVectorPtr  ComputeFeatureVector (const Raster&           image,
                                                     const MLClassPtr  knownClass,
                                                     RasterListPtr     intermediateImages,
                                                     RunLog&           runLog
                                                    ) = 0;


    /**  @brief  Returns the 'type_info' of the FeatureVector that this instance of 'FeatureVectorProducer' creates. */
    virtual  const type_info*  FeatureVectorTypeId () const = 0;

    /** 
     *@brief Returns the 'type_info' of the FeatureVectorList derived class that can contain instances in 'FeatureVector' 
     * instances created by this producer.
     */
    virtual  const type_info*  FeatureVectorListTypeId () const = 0;


    virtual  kkint16  Version ()  const = 0;  /**< The version number of the FeatureVector that is going to be computed. */


    /** @brief  Returns pointer to factory that instantiated this instance */
    FactoryFVProducerPtr  Factory ()  const  {return factory;}

    /**  
     *@brief  Returns back a "FileDesc" instance that describes the features that this instance of 'FeatureVectorProducer' creates.
     * The class derived form this class is responsible for creating the FileDesc instance.  When this methoid is called if "fileDesc" 
     * equals NULL  then will call "DefineFileDesc", which is a purve virtual method that will be implemented by the derived class, to 
     * instaiate a new instance.
     */

    FileDescConstPtr  FileDesc ()  const;


    /**  @brief  Returns a kkint16 description of the FeatureVector which can be used as part/all of a File or Directory name.  */
    const KKStr&  Name () const {return name;};


    //  Feature description related methods.
    kkuint32  FeatureCount ()  const;

    const KKStr&  FeatureName (kkuint32  fieldNum)  const;

    kkuint32  MaxNumOfFeatures ()     {return  FeatureCount ();}  /**<  Same as FeatureCount  */

    /**
     *@brief Manufactures a instance of a derived 'FeatureVectorList' class that is appropriate for containing instances
     *of FeatureVectors by this FeatureVectorProducer.
     */
    virtual  FeatureVectorListPtr  ManufacturFeatureVectorList (bool     owner,
                                                                RunLog&  runLog
                                                               ) = 0;


  protected:
    virtual  FileDescPtr  DefineFileDesc () const = 0;

  private:
    FactoryFVProducerPtr  factory;
    mutable FileDescPtr   fileDesc;
    KKStr                 name;

    static  bool  atExitDefined;
  };   /* FeatureVectorProducer */

  typedef  FeatureVectorProducer::FeatureVectorProducerPtr  FeatureVectorProducerPtr;

#define  _FeatureVectorProducer_Defined_

}  /* KKMachineLearning */



#endif
