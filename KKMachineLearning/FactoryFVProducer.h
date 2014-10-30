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

    FactoryFVProducer (const FactoryFVProducer&  factory);

    FactoryFVProducer (const KKStr&      _name,
                       const KKStr&      _description,
                       FileDescPtr       _fileDesc,
                       FeatureFileIOPtr  _defaultFeatureFileIO
                      );

  private:
    /**
     *@brief  A Factory can neer be deleted until the application terminates;  the atexit method will perform the deletes.
     */
    virtual ~FactoryFVProducer ();

  public:
    const KKStr&      Description          ()  const  {return description;}
    FeatureFileIOPtr  DefaultFeatureFileIO ()  const  {return defaultFeatureFileIO;}
    FileDescPtr       FileDesc             ()  const  {return fileDesc;}
    const KKStr&      Name                 ()  const  {return name;}


    virtual  FeatureVectorProducerPtr  ManufacturInstance (RunLog&  runLog)  = 0;


    /**
     *@brief Manufactures a instance of a derived 'FeatureVectorList' class that is approprite for containing instances 
     *of FeatureVectors produced by the associated FeatureVectorProducer.
     *@details  The instanve 'FeatureVectorList' that is returned have 'owner' set to true; meaning that it will 
     *own the FeatureVector instances added to it.
     */
    virtual  FeatureVectorListPtr  ManufacturFeatureVectorList (RunLog&  runLog);



    /**
     *@brief Using register Factory  with "_name" will return a new instanve of FeatureVectorProducer.
     */
    static  FeatureVectorProducerPtr  ManufacturInstance (const KKStr&  _name,
                                                          RunLog&       runLog
                                                         );


    static  FactoryFVProducerPtr  LookUpFactory (const KKStr&  _name);

    static  void  RegisterFactory (FactoryFVProducerPtr  factory,
                                   RunLog&               runLog
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
