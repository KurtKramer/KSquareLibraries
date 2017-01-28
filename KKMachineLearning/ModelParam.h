#ifndef  _MODELPARAM_
#define  _MODELPARAM_

#include "RunLog.h"
#include "KKStr.h"
#include "FeatureNumList.h"

namespace KKMLL
{
  #ifndef _FileDesc_Defined_
  class  FileDesc;
  typedef  FileDesc*  FileDescPtr;
  typedef  FileDesc const *  FileDescConstPtr;
  #endif


  // All the classes sub-classed from 'ModelParam'  will need this.
  #if  !defined(_MLCLASS_)
  class  MLClass;
  typedef  MLClass*  MLClassPtr;
  class  MLClassList;
  typedef  MLClassList*  MLClassListPtr;
  #endif


/**
  *@brief  Abstract Base class for Machine Learning parameters.
  *@author Kurt Kramer
  *@details
  *For each Machine Learning algorithm implemented you would create a specialization of this class
  *to manage the parameters required by the algorithm. Specifically for each new class that you
  *create that is derived from 'Model' you will create a class derived from 'ModelParam'.
  *This class encapsulates general parameters that are common to all Machine Learning Models.
  *@see  Model
  */
  class  ModelParam
  {
  public:
    typedef  ModelParam*  ModelParamPtr;

    enum  class  ModelParamTypes
                      {Null,
                       Dual,
                       KNN,
                       OldSVM,
                       SvmBase,
                       UsfCasCor
                      };

    static KKStr            ModelParamTypeToStr   (ModelParamTypes _modelParamType);
    static ModelParamTypes  ModelParamTypeFromStr (const KKStr&    _modelParamTypeStr);
  

    /**
     * Needs to be synchronized with SVMparam::SVM_EncodingMethod
     */
    enum class  EncodingMethodType 
    {
      Null,
      NoEncoding,
      Binary,
      Scaled
    };
  
    ModelParam  ();
  
  
    ModelParam  (const ModelParam&  _param);
  
    virtual
    ~ModelParam  ();



    virtual
    ModelParamPtr  Duplicate () const = 0;


    virtual
    kkMemSize  MemoryConsumedEstimated ()  const;


    virtual
    void    ParseCmdLine (KKStr    _cmdLineStr,
                          bool&    _validFormat,
                          RunLog&  _log
                         );


    virtual
    void   ParseCmdLinePost (RunLog&  log);



  
    /**
     *@brief Creates a a Command Line String that represents these parameters.
     *@details  All derived classes should implement this method. They should first call this method and
     *          then append there own parameters that are specific to their implementation.
     */
    virtual
    KKStr   ToCmdLineStr ()  const;
  

 
    // Member access methods

    virtual ModelParamTypes          ModelParamType             () const = 0;
    virtual KKStr                    ModelParamTypeStr          () const {return ModelParamTypeToStr (ModelParamType ());}

    virtual float                    AvgMumOfFeatures           () const;
    virtual EncodingMethodType       EncodingMethod             () const {return encodingMethod;}
    virtual KKStr                    EncodingMethodStr          () const {return EncodingMethodToStr (encodingMethod);}
    virtual kkint32                  ExamplesPerClass           () const {return examplesPerClass;}
    virtual const KKStr&             FileName                   () const {return fileName;}
    virtual bool                     NormalizeNominalFeatures   () const {return normalizeNominalFeatures;}
    virtual FeatureNumListConstPtr   SelectedFeatures           () const {return selectedFeatures;}
    virtual bool                     ValidParam                 () const {return validParam;}
  
    // Access members that were originally put here for 'ModelSVMBase'  and  'ModelOldSVM'
    virtual float   A_Param  () const;
    virtual double  C_Param  () const;  // Same as 'Cost'
    virtual double  Cost     () const;
    virtual double  Gamma    () const;
    virtual float   Prob     () const;


    // Member update methods
    virtual void  EncodingMethod     (EncodingMethodType     _encodingMethod)     {encodingMethod     = _encodingMethod;}
    virtual void  ExamplesPerClass   (kkint32                _examplesPerClass)   {examplesPerClass   = _examplesPerClass;}
    virtual void  FileName           (const KKStr&           _fileName)           {fileName           = _fileName;}
    virtual void  SelectedFeatures   (FeatureNumListConst&   _selectedFeatures);
    virtual void  ValidParam         (bool                   _validParam)         {validParam         = _validParam;}


    virtual void  A_Param  (float   _prob);
    virtual void  C_Param  (double  _cost);   // Same as 'Cost'
    virtual void  Cost     (double  _cost);
    virtual void  Gamma    (double  _gamma);
    virtual void  Prob     (float   _prob);

    virtual kkint32  NumOfFeaturesAfterEncoding (FileDescConstPtr  fileDesc,
                                                 RunLog&           log
                                                ) const;


    static  KKStr   EncodingMethodToStr    (EncodingMethodType     encodingMethod);

    static  EncodingMethodType     EncodingMethodFromStr    (const KKStr&  encodingMethodStr);


    virtual  void  ReadXML (XmlStream&      s,
                            XmlTagConstPtr  tag,
                            VolConstBool&   cancelFlag,
                            RunLog&         log
                           ) = 0;


    virtual  void  WriteXML (const KKStr&   varName,
                             std::ostream&  o
                            )  const = 0;


    /**  @brief  Will process any tokens that belong to 'ModelParam' and return NULL ones that are not will be passed back. */
    XmlTokenPtr  ReadXMLModelParamToken (XmlTokenPtr  t);

    void  WriteXMLFields (std::ostream&  o)  const;


  private:
 
    virtual
    void  ParseCmdLineParameter (const KKStr&  parameter,
                                 const KKStr&  value,
                                 bool&         parameterUsed,
                                 RunLog&       log
                                ) = 0;


    EncodingMethodType       encodingMethod;

    kkint32                  examplesPerClass;

    KKStr                    fileName;

    bool                     normalizeNominalFeatures;

    mutable
    FeatureNumListPtr        selectedFeatures;    /**< Feature Number to use. */

    bool                     validParam;

    // The following parameters are placed hear to support SVM based algorithms.
    // to help transition from old(CRAPPY) design to newer less crappy design.
    double  cost;
    double  gamma;
    float   prob;


  };  /* ModelParam */

  typedef  ModelParam::ModelParamPtr   ModelParamPtr;





  /**
   * @brief Base class to be used by all ModelParam derived objects.
   * @details Each derived class will need to implement their own XmlElement derived helper class. They will do it by utilizing the
   * [[XmlElementModelParamTemplate]] template which inherits this base class "XmlElementModelParam".
   */
  class   XmlElementModelParam:  public  XmlElement
  {
  public:
    XmlElementModelParam (XmlTagPtr   tag,
                          XmlStream&  s,
                          RunLog&     log
                         ):
     XmlElement (tag, s, log),
     value (NULL)
    {
    }

                
    ~XmlElementModelParam ()
    {
      delete value;
      value = NULL;
    }


    virtual
    ModelParam const *  Value ()  const  {return value;}

    virtual 
    ModelParam*  TakeOwnership ()
    {
      ModelParam* v = value;
      value = NULL;
      return  v;
    }

  protected:
    ModelParam*  value;
  };  /* XmlElementModelParam */

  typedef  XmlElementModelParam*  XmlElementModelParamPtr;






  /**
   *@brief  ModelParam derived classes will implement their "XmlElement" helper class via this template.
   *@see XmlElementModelParamKnn, XmlElementModelParamUsfCasCor, XmlElementModelOldSVM
   */
  template<class  T>
  class  XmlElementModelParamTemplate:  public  XmlElementModelParam
  {
  public:
    XmlElementModelParamTemplate (XmlTagPtr      tag,
                                  XmlStream&     s,
                                  VolConstBool&  cancelFlag,
                                  RunLog&        log
                                 ):
    XmlElementModelParam (tag, s, log)
    {
      value = new T();
      value->ReadXML (s, tag, cancelFlag, log);
    }

                
    ~XmlElementModelParamTemplate ()
    {
    }

    T const *  Value ()  const  {return dynamic_cast<T*> (value);}
    
    T*  TakeOwnership ()
    {
      T* v = dynamic_cast<T*> (value);
      value = NULL;
      return  v;
    }

    static
    void  WriteXML (const T&       t,
                    const KKStr&   varName,
                    std::ostream&  o
                   )
    {
      dynamic_cast<T>(t).WriteXML (varName, o);
    }
  private:
  };  /* XmlElementModelParamTemplate */


}  /* namespace KKMLL */



#endif
