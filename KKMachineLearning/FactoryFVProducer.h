#if  !defined(_FACTORYFVPRODUCER_)
#define  _FACTORYFVPRODUCER_


#include "RunLog.h"


namespace KKMachineLearning 
{

#if  !defined(_FeatureVector_Defined_)
  class  FeatureVector;
  typedef  FeatureVector*    FeatureVectorPtr;
#endif


#if  !defined(_FeatureVectorList_Defined_)
  class  FeatureVectorList;
  typedef  FeatureVectorList*    FeatureVectorListPtr;
#endif


#if  !defined(_FeatureVectorProducer_Defined_)
  class  FeatureVectorProducer;
  typedef  FeatureVectorProducer*    FeatureVectorProducerPtr;
#endif


#if  !defined(_FeatureFileIO_Defined_)
  class  FeatureFileIO;
  typedef  FeatureFileIO* FeatureFileIOPtr;
#endif


#if  !defined(_FileDesc_Defined_)
  class  FileDesc;
  typedef  FileDesc*  FileDescPtr;
#endif


  /**
   *@brief  Responsible for creating a FeatureFectorProducer instance.
   *@details  The idea is for each class derived from FeatureVector there will be a correponding 'FeatureVectorProducer'
   * and this Factory class will be responsible for creating the instances of these FeatureVectorProducer.  Tere will 
   * be static members that will maintain a lkist of FactoryFVProducer factories with their associated name.  
   */
  class FactoryFVProducer
  {
  public:
    typedef  FactoryFVProducer*  FactoryFVProducerPtr;

    FactoryFVProducer (const KKStr&  _name,
                       const KKStr&  _description
                      );

  protected:
    /**
     *@brief  A Factory can neer be deleted until the application terminates;  the atexit method will perform the deletes.
     */
    virtual ~FactoryFVProducer ();

  public:
    const KKStr&   Description ()  const  {return description;}
    const KKStr&   Name        ()  const  {return name;}


    virtual  FeatureFileIOPtr  DefaultFeatureFileIO ()  const = 0;

    virtual  FileDescPtr       FileDesc             ()  const = 0;

    virtual  FeatureVectorProducerPtr  ManufactureInstance (RunLog&  runLog)  = 0;


    /**
     *@brief Manufactures a instance of a derived 'FeatureVectorList' class that is approprite for containing instances 
     *of FeatureVectors produced by the associated FeatureVectorProducer.
     */
    virtual  FeatureVectorListPtr  ManufacturFeatureVectorList (bool     owner,
                                                                RunLog&  runLog
                                                               );


    /**
     *@brief Using register Factory  with "_name" will return a new instanve of FeatureVectorProducer.
     */
    static  FeatureVectorProducerPtr  ManufactureInstance (const KKStr&  _name,
                                                           RunLog&       runLog
                                                          );


    static  FactoryFVProducerPtr  LookUpFactory (const KKStr&  _name);

    static  void  RegisterFactory (FactoryFVProducerPtr  factory,
                                   RunLog*               runLog
                                  );


  private:
    KKStr             description;            /**<  Description  of  FeatuireVectorProducer.                                                            */
    FeatureFileIOPtr  defaultFeatureFileIO;
    FileDescPtr       fileDesc;               /**<  File Description  that would be used by the feature vectors produced by the FeatireVector producer. */
    KKStr             name;                   /**<  Name of FeatuireVectorProducer will create.                                                         */

    static  void  FinaleCleanUp ();

    static  bool  atExitDefined;

    static  map<KKStr,FactoryFVProducerPtr>  factories;
    static  map<KKStr,FactoryFVProducerPtr>::iterator  factoriesIdx;
  };  /* FactoryFVProducer */

  typedef  FactoryFVProducer::FactoryFVProducerPtr  FactoryFVProducerPtr;

#define  _FactoryFVProducer_Defined_


}  /* KKMachineLearning */



#endif
