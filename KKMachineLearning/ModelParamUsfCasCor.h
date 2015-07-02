#if  !defined(_MODELPARAMUSFCASCOR_)
#define  _MODELPARAMUSFCASCOR_

#include "RunLog.h"
#include "KKStr.h"

#include "ModelParam.h"


namespace KKMLL 
{
  /**
   *@brief  This class encapsulates are the information necessary to build a UsfCasCor class.
   */
  class  ModelParamUsfCasCor:  public  ModelParam
  {
  public:
    typedef  ModelParamUsfCasCor*  ModelParamUsfCasCorPtr;

    ModelParamUsfCasCor  ();
     
    ModelParamUsfCasCor  (int      _in_limit,
                          int      _out_limit,
                          int      _number_of_rounds,
                          int      _number_of_trials,
                          kkint64  _random_seed,
                          bool     _useCache
                         );
  
    virtual ~ModelParamUsfCasCor  ();
  
    virtual ModelParamUsfCasCorPtr  Duplicate () const;

    virtual ModelParamTypes  ModelParamType () const {return ModelParamTypes::UsfCasCor;}


    // Member access methods
    int      In_limit         () const {return in_limit;}
    int      Out_limit        () const {return out_limit;}
    int      Number_of_rounds () const {return number_of_rounds;}
    int      Number_of_trials () const {return number_of_trials;}
    kkint64  Random_seed      () const {return random_seed;}
    bool     UseCache         () const {return useCache;}


    // Member update method  s
    void  In_limit         (int      _in_limit)         {in_limit          = _in_limit;}
    void  Out_limit        (int      _out_limit)        {out_limit         = _out_limit;}
    void  Number_of_rounds (int      _number_of_rounds) {number_of_rounds  = _number_of_rounds;}
    void  Number_of_trials (int      _number_of_trials) {number_of_trials  = _number_of_trials;}
    void  Random_seed      (kkint64  _random_seed)      {random_seed       = _random_seed;}
    void  UseCache         (bool     _useCache)         {useCache          = _useCache;}


    /**
     *@brief Creates a Command Line String that represents these parameters.
     *@details  All derived classes should implement this method. They should first call this method and 
     *          then append there own parameters that are specific to their implementation.
     */
    virtual KKStr  ToCmdLineStr ()  const;


    virtual  void  ReadXML (XmlStream&      s,
                            XmlTagConstPtr  tag,
                            RunLog&         log
                           );


    virtual  void  WriteXML (const KKStr&  varName,
                             ostream&      o
                            )  const;

  private:
    virtual void  ParseCmdLinePost ();  /**< Will get called after the entire parameter string has been processed. */

    virtual void  ParseCmdLineParameter (const KKStr&  parameter,
                                         const KKStr&  value,
                                         bool&         parameterUsed,
                                         RunLog&       log
                                        );

    int      in_limit;
    int      out_limit;
    int      number_of_rounds;
    int      number_of_trials;
    kkint64  random_seed;
    bool     useCache;
  };  /* ModelParamUsfCasCor */

  typedef  ModelParamUsfCasCor*   ModelParamUsfCasCorPtr;

  typedef  XmlElementModelParamTemplate<ModelParamUsfCasCor>  XmlElementModelParamUsfCasCor;
  typedef  XmlElementModelParamUsfCasCor*  XmlElementModelParamUsfCasCorPtr;
}  /* namespace KKMLL */



#endif


