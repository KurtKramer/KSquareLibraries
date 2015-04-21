#if  !defined(_TRAININGCLASS_)
#define  _TRAININGCLASS_
/**
 *@class  KKMLL::TrainingClass
 *@brief  Specify where training examples and other related data for a MLClass that is  needed to train a classifier.
 *@details 
 * You create one instance of this object for each MLClass instance that you want to include in a classifier. Each instance 
 * will tie a class to the directories where you can find training examples. Other info that is included is 'CountFator', 
 * training Weight,  dun-classifier that is to be used to further define the prediction.
 */


#include "KKStr.h"
#include "KKQueue.h"
#include "XmlStream.h"

namespace KKMLL 
{
  #if   !defined(_MLCLASS_)
  class  MLClass;
  typedef  MLClass*  MLClassPtr;
  typedef  MLClass  const * MLClassConstPtr;
  class  MLClassList;
  typedef  MLClassList*  MLClassListPtr;
  #endif

  #if  !defined(_TrainingConfiguration2_Defined_)
  class  TrainingConfiguration2;
  typedef  TrainingConfiguration2*  TrainingConfiguration2Ptr;
  #endif

  #if  !defined(_TrainingConfiguration2List_Defined_)
  class  TrainingConfiguration2List;
  typedef  TrainingConfiguration2List*  TrainingConfiguration2ListPtr;
  #endif



  class  TrainingClass
  {
  public:
    TrainingClass ();

    /**
     *@brief  Constructor,  Creates a new instance of TrainingClass and populates
     *fields with respective data from parameters.
     *@param[in] _directories A list of directories where training examples for class '_name' can be found.
     *@param[in] _weight 
     *@param[in] _countFactor
     *@param[in] _subClassifier  If not NULL points to the configuration that is to be used if this class is predicted.
     *@param[in,out] mlClasses List of classes.
     */
    TrainingClass (const VectorKKStr&         _directories,
                   KKStr                      _name,
                   float                      _weight,
                   float                      _countFactor,
                   TrainingConfiguration2Ptr  _subClassifier,
                   MLClassList&               _mlClasses
                  );

    TrainingClass (const TrainingClass&  tc);


    float                      CountFactor       () const  {return  countFactor;}
    const KKStr&               Directory         (kkuint32 idx) const;
    kkuint32                   DirectoryCount    () const;
    const VectorKKStr&         Directories       () const  {return  directories;}
    const KKStr&               FeatureFileName   () const  {return  featureFileName;}
    const MLClassPtr           MLClass           () const  {return  mlClass;}
    const KKStr&               Name              () const;
    TrainingConfiguration2Ptr  SubClassifier     () const  {return  subClassifier;}
    const KKStr&               SubClassifierName () const  {return  subClassifierName;}

    float                      Weight            () const  {return  weight;}

    KKStr                      ExpandedDirectory (const KKStr&  rootDir,
                                                  kkuint32      idx
                                                 );

    void  AddDirectory    (const KKStr&  _directory);

    void  CountFactor       (float         _countFactor)       {countFactor       = _countFactor;}
    void  FeatureFileName   (const KKStr&  _featureFileName)   {featureFileName   = _featureFileName;}
    void  MLClass           (MLClassPtr    _mlClass)           {mlClass           = _mlClass;}
    void  SubClassifierName (const KKStr& _subClassifierName)  {subClassifierName = _subClassifierName;}
    void  Weight            (float         _weight)            {weight            = _weight;}

    void  Directory       (kkuint32      idx, 
                           const KKStr&  directory
                          );


  private:
    float            countFactor;    /**<  Used when counting particles,  specifies the impact on the count that this [articular trainingClass has. */
    VectorKKStr      directories;
    KKStr            featureFileName;
    MLClassPtr       mlClass;

    TrainingConfiguration2Ptr  
                     subClassifier;  /**< The classifier hat is to be used to further define the class;  for example
                                      *  if 'mlClass' is predicted 'subClassifier' is to be called to further define the
                                      *  prediction.  The instance of 'TrainingClass' will not own this classifier; it will
                                      *  be owned by 'subClassifiers' in 'TrainingConfiguration2'.
                                      */

    KKStr            subClassifierName;

    float            weight;         /**< Will be used in 'TrainingProcess::ExtractFeatures' to weight images.  
                                      * the SVM Cost parameter from examples in this class will be weighted by this value.
                                      */

  };  /* TrainingClass */



  typedef  TrainingClass*  TrainingClassPtr;


  class  TrainingClassList:  public KKQueue<TrainingClass>
  {
  public:
    TrainingClassList (const KKStr&  rootDir,
                       bool          owner
                      );

  private:
    TrainingClassList (const TrainingClassList&  tcl);

  public:
    TrainingClassList (const TrainingClassList&  tcl,
                       bool                      _owner
                      );


    const KKStr&      RootDir ()  const  {return  rootDir;}
    void              RootDir (const KKStr&  _rootDir)  {rootDir = _rootDir;}

    void              AddTrainingClass (TrainingClassPtr  trainingClass);


    TrainingClassList*  DuplicateListAndContents ()  const;

    TrainingClassPtr    LocateByMLClass (MLClassPtr       _mlClass)  const;

    TrainingClassPtr    LocateByMLClassName (const KKStr&  className);

    TrainingClassPtr    LocateByDirectory (const KKStr&  directory);



  private: 
    KKStr   rootDir;
    
  };  /* TrainingClassList*/

  typedef  TrainingClassList*  TrainingClassListPtr;



  class  XmlElementTrainingClass:  public  XmlElement
  {
  public:
    XmlElementTrainingClass (XmlTagPtr   tag,
                             XmlStream&  s,
                             RunLog&     log
                            );
                
    virtual  ~XmlElementTrainingClass ();

    TrainingClassPtr  Value ()  const;

    TrainingClassPtr  TakeOwnership ();

    static
    void  WriteXML (const TrainingClass&  tc,
                    const KKStr&          varName,
                    ostream&              o
                   );
  private:
    TrainingClassPtr  value;
  };
  typedef  XmlElementTrainingClass*  XmlElementTrainingClassPtr;




  class  XmlElementTrainingClassList:  public  XmlElement
  {
  public:
    XmlElementTrainingClassList (XmlTagPtr   tag,
                                 XmlStream&  s,
                                 RunLog&     log
                                );
                
    virtual  ~XmlElementTrainingClassList ();

    TrainingClassListPtr  Value ()  const;

    TrainingClassListPtr  TakeOwnership ();

    static
    void  WriteXML (const TrainingClassList&  tc,
                    const KKStr&              varName,
                    ostream&                  o
                   );    
  private:
    TrainingClassListPtr  value;
  };
  typedef  XmlElementTrainingClassList*  XmlElementTrainingClassListPtr;




}  /* KKMLL */


#endif
