#ifndef  _BINARYCLASSPARMS_
#define  _BINARYCLASSPARMS_

/**
 @class  KKMLL::BinaryClassParms
 @brief  Machine Learning parameters to be used by a pair of classes.
 @author Kurt Kramer
 @details
 @code
 **********************************************************************
 ** Written by: Kurt Kramer                                           *
 **        For: Research Work, Plankton recognition System            *
 **-------------------------------------------------------------------*
 **  History                                                          *
 **                                                                   *
 **    date     Programmer   Description                              *
 **                                                                   *
 **  May-2004  Kurt Kramer  Original Development.                     *
 **-------------------------------------------------------------------*
 ** Was developed to support Feature Selection for each binary class  *
 ** combination.  This way each two class combination can have SVM    *
 ** and feature parameters that are specific to them.                 *
 **********************************************************************
 @endcode
 @see  KKMLL::SVMparam, KKMLL::ModelParamOldSVM, KKMLL::SVM233
 */
#include "KKQueue.h"

#include "svm.h"
#include "FeatureNumList.h"
#include "FileDesc.h"

namespace KKMLL
{
  #if  !defined (_MLCLASS_)
  class  MLClass;
  typedef  MLClass*  MLClassPtr;
  #endif


  /**
   *@brief Similar to SVMparam  except it is specialized for two classes.
   * @see  SVMparam
   * @see  SVM233
   */
  class  BinaryClassParms
  {
  public:
    typedef  BinaryClassParms*  BinaryClassParmsPtr;

    typedef  SVM233::svm_parameter  svm_parameter;

    /** 
     *@brief  Constructor for 'BinaryClassParms' where caller supplies the two classes and parameters for that specific class pair. 
     *@param[in] _class1
     *@param[in] _class2
     *@param[in] _param
     *@param[in] _selectedFeatures  Will create a duplicate instance; caller will still have same ownership status.
     *@param[in] _weight
     */
    
    BinaryClassParms (MLClassPtr              _class1,
                      MLClassPtr              _class2,
                      const svm_parameter&    _param,
                      FeatureNumListConstPtr  _selectedFeatures,
                      float                   _weight
                     );
    
    BinaryClassParms (const BinaryClassParms&  binaryClassParms);

    ~BinaryClassParms ();

    static
    BinaryClassParmsPtr  CreateFromTabDelStr (const KKStr& _str,
                                              RunLog&      _log
                                             );

    //  Member Access Methods
    double                   AParam             () const {return  param.A;}
    MLClassPtr               Class1             () const {return  class1;}
    MLClassPtr               Class2             () const {return  class2;}
    double                   C                  () const {return  param.C;}
    const svm_parameter&     Param              () const {return  param;}
    FeatureNumListConstPtr   SelectedFeatures   () const {return  selectedFeatures;}
    float                    Weight             () const {return  weight;}

    kkuint16                 NumOfFeatures      (FileDescConstPtr fileDesc) const;
    FeatureNumListConstPtr   SelectedFeaturesFD (FileDescConstPtr fileDesc) const;


    // member Update methods
    void  AParam           (float                       _A)                 {param.A           = _A;}
    void  C                (double                      _c)                 {param.C           = _c;}
    void  Gamma            (double                      _gamma)             {param.gamma       = _gamma;}
    void  Param            (const svm_parameter&        _param)             {param             = _param;}
    void  SelectedFeatures (const FeatureNumListConst&  _selectedFeatures);
    void  SelectedFeatures (FeatureNumListConstPtr      _selectedFeatures);  /**< Will make copy of instance;  caller will retain ownership status. */
    void  Weight           (float                       _weight)            {weight            = _weight;}

    KKStr     Class1Name ()  const;
    KKStr     Class2Name ()  const;
    kkMemSize MemoryConsumedEstimated ()  const;
    KKStr     ToTabDelString ()  const;


  private:
    MLClassPtr         class1;
    MLClassPtr         class2;

    svm_parameter      param;             /**< From SVMlib             */

    mutable
    FeatureNumListPtr  selectedFeatures;  /**< Feature Numbers to use. */
    float              weight;
  };  /* BinaryClassParms */




  typedef  BinaryClassParms::BinaryClassParmsPtr   BinaryClassParmsPtr;



  class  BinaryClassParmsList: public KKQueue<BinaryClassParms>
  {
  public:
    typedef  BinaryClassParmsList*  BinaryClassParmsListPtr;
    typedef  BinaryClassParmsList const *  BinaryClassParmsListConstPtr;

    BinaryClassParmsList ();

    BinaryClassParmsList (bool  _owner);
    

  private:
    BinaryClassParmsList (const BinaryClassParmsList&  binaryClassList);

  public:
    BinaryClassParmsList (const BinaryClassParmsList&  binaryClassList,
                          bool                         _owner
                         );


    static  
      BinaryClassParmsListPtr  CreateFromXML (std::istream&      i,
                                              FileDescConstPtr   fileDesc,
                                              RunLog&            log
                                             );

    static  
      BinaryClassParmsListPtr  CreateFromXML (std::FILE*        i,
                                              FileDescConstPtr  fileDesc,
                                              RunLog&           log
                                             );

    ~BinaryClassParmsList ();

    float  FeatureCountNet (FileDescConstPtr  fileDesc)  const;

    BinaryClassParmsPtr  LookUp (MLClassPtr  _class1,
                                 MLClassPtr  _class2
                                )  const;

    BinaryClassParmsList*  DuplicateListAndContents ()  const;

    kkMemSize  MemoryConsumedEstimated ()  const;

    virtual
    void  PushOnBack  (BinaryClassParmsPtr  binaryParms);

    virtual
    void  PushOnFront (BinaryClassParmsPtr  binaryParms);

    void  ReadXML (FILE*             i,
                   FileDescConstPtr  fileDesc,
                   RunLog&           log
                  );

    void  ReadXML (std::istream&     i,
                   FileDescConstPtr  fileDesc,
                   RunLog&           log
                  );

    void  WriteXML (std::ostream&  o)  const;



    virtual  void  ReadXML (XmlStream&      s,
                            XmlTagConstPtr  tag,
                            VolConstBool&   cancelFlag,
                            RunLog&         log
                           );


    virtual  void  WriteXML (const KKStr&   varName,
                             std::ostream&  o
                            )  const;




  private:
     struct  KeyField
     {
       KeyField (MLClassPtr  _class1,
                 MLClassPtr  _class2
                );

       bool  operator< (const KeyField& p2)  const;

       MLClassPtr  class1;
       MLClassPtr  class2;
     };

     typedef  std::map<KeyField,BinaryClassParmsPtr>   ClassIndexType;
     typedef  std::pair<KeyField, BinaryClassParmsPtr>  ClassIndexPair;
     ClassIndexType  classIndex;
  };


  typedef  BinaryClassParmsList::BinaryClassParmsListPtr  BinaryClassParmsListPtr;
  typedef  BinaryClassParmsList::BinaryClassParmsListConstPtr  BinaryClassParmsListConstPtr;


  typedef  XmlElementTemplate<BinaryClassParmsList>  XmlElementBinaryClassParmsList;
  typedef  XmlElementBinaryClassParmsList*  XmlElementBinaryClassParmsListPtr;


}  /* namespace KKMLL */

#endif


