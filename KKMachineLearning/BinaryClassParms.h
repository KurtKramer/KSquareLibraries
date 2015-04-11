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

namespace KKMLL
{
  #if  !defined (_MLCLASS_)
  class  MLClass;
  typedef  MLClass*  MLClassPtr;
  typedef  MLClass const * MLClassConstPtr;
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

    /** @brief  Create from existing Model on Disk. */
    BinaryClassParms (MLClassPtr             _class1,
                      MLClassPtr             _class2,
                      const svm_parameter&   _param,
                      const FeatureNumList&  _selectedFeatures,
                      float                  _weight
                     );
    
    BinaryClassParms (const BinaryClassParms&  binaryClassParms);

    ~BinaryClassParms ();

    static
    BinaryClassParmsPtr  CreateFromTabDelStr (const KKStr& _str,
                                              FileDescPtr  _fileDesc,
                                              RunLog&      _log
                                             );

    //  Member Access Methods
    double                 AParam           () const {return  param.A;}
    MLClassPtr             Class1           () const {return  class1;}
    MLClassPtr             Class2           () const {return  class2;}
    double                 C                () const {return  param.C;}
    const svm_parameter&   Param            () const {return  param;}
    const FeatureNumList&  SelectedFeatures () const {return  selectedFeatures;}
    float                  Weight           () const {return  weight;}

    // member Update methods
    void  AParam           (float                  _A)                 {param.A           = _A;}
    void  C                (double                 _c)                 {param.C           = _c;}
    void  Gamma            (double                 _gamma)             {param.gamma       = _gamma;}
    void  Param            (const svm_parameter&   _param)             {param             = _param;}
    void  SelectedFeatures (const FeatureNumList&  _selectedFeatures)  {selectedFeatures  = _selectedFeatures;}
    void  Weight           (float                  _weight)            {weight            = _weight;}

    KKStr   Class1Name ()  const;
    KKStr   Class2Name ()  const;
    kkint32 MemoryConsumedEstimated ()  const;
    KKStr   ToTabDelString ()  const;


  private:
    MLClassPtr      class1;
    MLClassPtr      class2;

    svm_parameter   param;             /**< From SVMlib            */
    FeatureNumList  selectedFeatures;  /**< Feature Number to use. */
    float           weight;
  };  /* BinaryClassParms */




  typedef  BinaryClassParms::BinaryClassParmsPtr   BinaryClassParmsPtr;



  class  BinaryClassParmsList: public KKQueue<BinaryClassParms>
  {
  public:
    typedef  BinaryClassParmsList*  BinaryClassParmsListPtr;

    BinaryClassParmsList (bool  _owner);
    

  private:
    BinaryClassParmsList (const BinaryClassParmsList&  binaryClassList);

  public:
    BinaryClassParmsList (const BinaryClassParmsList&  binaryClassList,
                          bool                         _owner
                         );


    static  
      BinaryClassParmsListPtr  CreateFromXML (istream&     i, 
                                              FileDescPtr  fileDesc,
                                              RunLog&      log
                                             );

    static  
      BinaryClassParmsListPtr  CreateFromXML (FILE*        i, 
                                              FileDescPtr  fileDesc,
                                              RunLog&      log
                                             );

    ~BinaryClassParmsList ();

    float  FeatureCountNet ()  const;

    BinaryClassParmsPtr  LookUp (MLClassPtr  _class1,
                                 MLClassPtr  _class2
                                )  const;

    BinaryClassParmsList*  DuplicateListAndContents ()  const;

    kkint32  MemoryConsumedEstimated ()  const;

    virtual
    void  PushOnBack  (BinaryClassParmsPtr  binaryParms);

    virtual
    void  PushOnFront (BinaryClassParmsPtr  binaryParms);

    void  ReadXML (FILE*        i,
                   FileDescPtr  fileDesc,
                   RunLog&      log
                  );

    void  ReadXML (istream&     i,
                   FileDescPtr  fileDesc,
                   RunLog&      log
                  );

    void  WriteXML (ostream&  o)  const;


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

     typedef  map<KeyField,BinaryClassParmsPtr>   ClassIndexType;
     typedef  pair<KeyField, BinaryClassParmsPtr>  ClassIndexPair;
     ClassIndexType  classIndex;
  };


  typedef  BinaryClassParmsList::BinaryClassParmsListPtr  BinaryClassParmsListPtr;

}  /* namespace KKMLL */

#endif


