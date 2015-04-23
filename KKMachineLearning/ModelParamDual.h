#if  !defined(_MODELPARAMDUAL_)
#define  _MODELPARAMDUAL_


#include  "ModelParam.h"


namespace KKMLL 
{
  /**
   *@class  KKMLL::ModelParamDual
   ************************************************************************************************
   *                          Parameters used for the Dual Classifier.                            *
   ************************************************************************************************
   *@see  Model
   */
  class  ModelParamDual: public  ModelParam
  {
  public:
    typedef  ModelParamDual*  ModelParamDualPtr;

    typedef  enum  {pfmNULL, pfmOr, pfmAnd}  ProbFusionMethodType;
  
    ModelParamDual  (RunLog& _log);
  
    ModelParamDual  (const KKStr&  _configFileName1,
                     const KKStr&  _configFileName2,
                     bool          _fullHierarchyMustMatch,
                     RunLog&       _log
                    );
  
    ModelParamDual  (const ModelParamDual&  _param);
  
    virtual
    ~ModelParamDual  ();

    static
      ProbFusionMethodType  ProbFusionMethodFromStr (const KKStr& s);

    static
      KKStr  ProbFusionMethodToStr (ProbFusionMethodType  pfm);


    const KKStr&  ConfigFileName1        () const {return configFileName1;}
    const KKStr&  ConfigFileName2        () const {return configFileName2;}
    bool          FullHierarchyMustMatch () const {return fullHierarchyMustMatch;}
    MLClassPtr    OtherClass             () const {return otherClass;}

    virtual ModelParamTypes  ModelParamType () const {return ModelParamTypes::mptDual;}

    void  OtherClass (MLClassPtr  _otherClass)  {otherClass = _otherClass;}

    virtual
    ModelParamDualPtr  Duplicate () const;

    virtual
    void    ReadSpecificImplementationXML (istream&     i,
                                           FileDescPtr  fileDesc
                                          );
  

    /**
     *@brief Creates a Command Line String that represents these parameters.
     *@details  All derived classes should implement this method.  They should first call this method and
     *          then append there own parameters that are specific to their implementation.
     */
    virtual
    KKStr   ToCmdLineStr ()  const;
  

    virtual
    void    WriteSpecificImplementationXML (std::ostream&  o)  const;
  
  
  private:
    virtual
    void   ParseCmdLinePost ();  // Will get called after the entire parameter string has been processed.

    virtual
    void  ParseCmdLineParameter (const KKStr&  parameter,
                                 const KKStr&  value,
                                 bool&         parameterUsed
                                );
 
    KKStr                 configFileName1;
    KKStr                 configFileName2;
    bool                  fullHierarchyMustMatch;
    MLClassPtr            otherClass;
    ProbFusionMethodType  probFusionMethod;



  };  /* ModelParamDual */
  
  typedef  ModelParamDual::ModelParamDualPtr   ModelParamDualPtr;



}  /* namespace KKMLL */



#endif

