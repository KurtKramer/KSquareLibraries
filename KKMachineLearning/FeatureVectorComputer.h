#if  !defined(_FEATUREVECTORCOMPUTER_)
#define  _FEATUREVECTORCOMPUTER_

/**
 *@class  KKMachineLearning::FeatureVectorComputer
 *@brief  A abstract class that is meant to compute a FeatureVector from a source image.
 *@details Applications that want to utilize this library wil need to either use one of the 
 *provided "FeatureVectorComputer" derived classes a=or supply their own.  This class will 
 *responsable for computing a FeatureVector from a supplied Image.
 *
 *Each FeatureVector computer will need to have a unique name taht will be supplied when 
 *constructed. This name will be used later when it is required to locate the appropriate 
 *FeatureVectorComputer to utilize.
 *@see FeatureVectorList
 *@see PostLarvaeFV
 *@see FeatureFileIO
 */


#include "RunLog.h"

namespace KKMachineLearning 
{

#if  !defined(_FeatureVector_Defined_)
  class  FeatureVector;
  typedef  FeatureVector*  FeatureVectorPtr;
#endif

#if  !defined(_Raster_Defined_)
  class  Raster;
  typedef  Raster*  RasterPtr;
  typedef  Raster const*  RasterConstPtr;
#endif


#if  !defined(_FileDesc_Defined_)
  class FileDesc;
  typedef  FileDesc*        FileDescPtr;
  typedef  FileDesc* const  FileDescConstPtr;
#endif


  class FeatureVectorComputer
  {
  public:
    typedef  FeatureVectorComputer*  FeatureVectorComputerPtr;

    FeatureVectorComputer (const KKStr&  _name,
                           FileDescPtr   _fileDesc
                          );

    virtual ~FeatureVectorComputer ();

    virtual  FeatureVectorPtr  ComputeFeatureVector (RasterPtr  image,
                                                     RunLog&    runLog
                                                    ) = 0;


    /**  @brief  Returns the 'type_info' of the Feature Vector that this instance of 'FeatureComputer' creates. */
    virtual  const type_info*  FeatureVectorTypeId () const = 0;


    /**  @brief  Returns back a "FileDesc" instance that describes the feastures that this instance of 'FeatureVectorComputer' creates.  */
    FileDescConstPtr  FileDesc () const {return  fileDesc;}


    /**  @brief  Returns a kkint16 description of the FeatureVector which can be used as part/all of a File or Direecttory name.  */
    const KKStr&  Name () const {return name;};


    //  Feature description related methods.
    kkuint32  FeatureCount ()  const;

    const KKStr&  FeatureName (kkuint32  fieldNum)  const;

    kkuint32  MaxNumOfFeatures ()     {return  FeatureCount ();}  /**<  Same as FeatureCount  */

  protected:
    void  SetFileDesc (FileDescPtr  _fileDesc);

    static  RegisterAFeatureVectorComputer (FeatureVectorComputerPtr  fvComputer);


  private:
    FileDescPtr  fileDesc;
    KKStr        name;

    static  map<KKStr,FeatureVectorComputerPtr>  existingInstances;
  };   /* FeatureVectorComputer */

#define  _FeatureVectorComputer_Defined_

}  /* KKMachineLearning */



#endif
