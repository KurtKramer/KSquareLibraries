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
  typedef  MLClass const *  MLClassConstPtr;
  typedef  MLClass const *  MLClassConstPtr;
  class  MLClassList;
  typedef  MLClassList *  MLClassListPtr;
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
     *@param[in] _name The name of the class that this'TrainingClass' instance will be pointing to.
     *@param[in] _weight You can give extra weight to a class with this parameter; its impact/use varies with particular algorithm(Model) used.
     *@param[in] _countFactor 
     *@param[in] _subClassifier  If not NULL points to the configuration that is to be used if this class 
     *                           is predicted. This is how multi-level classifier is implemented.
     *@param[in,out] _mlClasses List of classes.
     */
    TrainingClass (const VectorKKStr&         _directories,
                   KKStr                      _name,
                   float                      _weight,
                   float                      _countFactor,
                   TrainingConfiguration2Ptr  _subClassifier,
                   MLClassList&               _mlClasses
                  );

    TrainingClass (const TrainingClass&  tc);

    virtual ~TrainingClass ();

    float                      CountFactor       () const  {return  countFactor;}
    const KKStr&               Directory         (kkuint32 idx) const;
    kkuint32                   DirectoryCount    () const;
    const VectorKKStr&         Directories       () const  {return  directories;}
    const KKStr&               FeatureFileName   () const  {return  featureFileName;}
    MLClassPtr                 MLClass           () const  {return  mlClass;}
    const KKStr&               Name              () const;
    TrainingConfiguration2Ptr  SubClassifier     () const  {return  subClassifier;}
    const KKStr&               SubClassifierName () const  {return  subClassifierName;}

    float                      Weight            () const  {return  weight;}

    KKStr                      ExpandedDirectory (const KKStr&  rootDir,
                                                  kkuint32      idx
                                                 )  const;

    void  AddDirectory    (const KKStr&  _directory);
    void  AddDirectories  (const VectorKKStr&  _directories);

    void  CountFactor       (float                      _countFactor)        {countFactor       = _countFactor;}
    void  FeatureFileName   (const KKStr&               _featureFileName)    {featureFileName   = _featureFileName;}
    void  MLClass           (MLClassPtr                 _mlClass)            {mlClass           = _mlClass;}
    void  SubClassifier     (TrainingConfiguration2Ptr  _subClassifier)      {subClassifier     = _subClassifier;}
    void  SubClassifierName (const KKStr&               _subClassifierName)  {subClassifierName = _subClassifierName;}
    void  Weight            (float                      _weight)             {weight            = _weight;}

    void  ClassNameLineNum     (kkint32  _classNameLineNum)     { classNameLineNum     = _classNameLineNum; }
    void  CountFactorLineNum   (kkint32  _countFactorLineNum)   { countFactorLineNum   = _countFactorLineNum; }
    void  DirLineNum           (kkint32  _dirLineNum)           { dirLineNum           = _dirLineNum; }
    void  SubClassifierLineNum (kkint32  _subClassifierLineNum) { subClassifierLineNum = _subClassifierLineNum; }
    void  WeightLineNum        (kkint32  _weightLineNum)        { weightLineNum        = _weightLineNum; }

    void  Directory       (kkuint32      idx, 
                           const KKStr&  directory
                          );

    virtual
    void  ReadXML (XmlStream&      s,
                   XmlTagConstPtr  tag,
                   VolConstBool&   cancelFlag,
                   RunLog&         log
                  );


    virtual  
    void  WriteXML (const KKStr&   varName,
                    std::ostream&  o
                   )  const;

    virtual
    void  WriteXML (const KKStr&  varName,
                    const KKStr&  rootDir,
                    std::ostream& o
                   )  const;



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

    kkint32  classNameLineNum;
    kkint32  dirLineNum;
    kkint32  weightLineNum;
    kkint32  countFactorLineNum;
    kkint32  subClassifierLineNum;
  };  /* TrainingClass */


  typedef  TrainingClass*  TrainingClassPtr;

  typedef  TrainingClass const *  TrainingClassConstPtr;


  class  TrainingClassList:  public KKQueue<TrainingClass>
  {
  public:
    TrainingClassList ();

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

    virtual void        ReadXML (XmlStream&     s,
                                 XmlTagPtr      tag,
                                 VolConstBool&  cancelFlag,
                                 RunLog&        log
                                );

    virtual void        WriteXML (const KKStr&   varName,
                                  std::ostream&  o
                                 )  const;
  private: 
    KKStr   rootDir;
    
  };  /* TrainingClassList*/

  typedef  TrainingClassList*  TrainingClassListPtr;



  typedef  XmlElementTemplate<TrainingClass>  XmlElementTrainingClass;
  typedef  XmlElementTrainingClass*  XmlElementTrainingClassPtr;


  typedef  XmlElementTemplate<TrainingClassList>  XmlElementTrainingClassList;
  typedef  XmlElementTrainingClassList*  XmlElementTrainingClassListPtr;

}  /* KKMLL */


#endif
