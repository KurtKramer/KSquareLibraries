#ifndef  _TRAININGCLASS_
#define  _TRAININGCLASS_
/**
 @class  KKMachineLearning::TrainingClass
 @code
 *********************************************************************
 *                         TrainingClass                             *
 *                                                                   *
 * You create one instance of this object for each set of training   *
 * data tou want to specify.  There will be a 'many to one' re-      *
 * lationship between 'TrainingClass'  and  'MLClass'.  This is   *
 * based on the assumption that there can be more than one set of    *
 * training data that you can use for the same class.                *
 *                                                                   *
 *-------------------------------------------------------------------*
 *                                                                   *
 *  directory  - Directory where Training Images are stored.         *
 *                                                                   *
 *  featureFileName - Name of data file that is to contain feature   *
 *               data for this TrainingClass.                        *
 *                                                                   *
 *  mlClass - Pointer to MLClass that this TrainingClass is    *
 *               for.                                                *
 *********************************************************************
 @endcode
 */



#include  "KKStr.h"
#include  "KKQueue.h"

namespace KKMachineLearning {


#ifndef  _MLCLASS_
class  MLClass;
typedef  MLClass*  MLClassPtr;
class  MLClassList;
typedef  MLClassList*  MLClassListPtr;
#endif




class  TrainingClass
{

public:

  /**
   *************************************************************************
   *  Constructor,  Creates a new instance of TrainingClass and populates  *
   *  fields with respective data from parameters.                         *
   *                                                                       *
   *  mlClasses - list of ImageClasses.  Constructor will update this   *
   *                 set 'mlClass' to the MLClass in this list that  *
   *                 has the same name  as '_name'.  If one does not exist *
   *                 it will create a new MLClass object and add it to  *
   *                 'mlClasses'.                                       *
   *************************************************************************
   */
  TrainingClass (KKStr            _directory,
                 KKStr            _name,
                 float            _weight,
                 float            _countFactor,
                 MLClassList&  mlClasses
                );

  TrainingClass (const TrainingClass&  tc);


  float               CountFactor     () const  {return  countFactor;}
  const KKStr&        Directory       () const  {return  directory;}
  const KKStr&        FeatureFileName () const  {return  featureFileName;}
  const MLClassPtr MLClass      () const  {return  mlClass;}
  const KKStr&        Name            () const;
  float               Weight          () const  {return  weight;}

  KKStr               ExpandedDirectory (const KKStr&  rootDir);

  void  CountFactor     (float         _countFactor)      {countFactor     = _countFactor;}
  void  Directory       (const KKStr&  _directory)        {directory       = _directory;}
  void  FeatureFileName (const KKStr&  _featureFileName)  {featureFileName = _featureFileName;}
  void  MLClass      (MLClassPtr _mlClass)       {mlClass      = _mlClass;}
  void  Weight          (float         _weight)           {weight          = _weight;}


private:
  KKStr          SubstituteInEvironmentVariables (const KKStr&  src);

  KKStr          directory;
  KKStr          featureFileName;
  MLClassPtr  mlClass;
  float          weight;      // Will be used in 'TrainingProcess::ExtractFeatures' to weight images.  
                              // the SVM Cost parameter from examples in this class will be weighted by this value.

  float          countFactor;  /**<  Used when counting particles,  specifies the impact on the count that this [articular trainingClass has. */
};


typedef  TrainingClass*  TrainingClassPtr;


class  TrainingClassList:  public KKQueue<TrainingClass>
{
public:
  TrainingClassList (const KKStr&  _rootDirExpanded,
                     bool  owner    = true,
                     int32 initSize = 5
                    );

private:
  TrainingClassList (const TrainingClassList&  tcl);

public:
  TrainingClassList (const TrainingClassList&  tcl,
                     bool                      _owner
                    );


  const KKStr&      RootDirExpanded ()  {return  rootDirExpanded;}
  void              RootDir (const KKStr&  _rootDirExpanded)  {rootDirExpanded = _rootDirExpanded;}

  void              AddTrainingClass (TrainingClassPtr  trainingClass);


  TrainingClassList*  DuplicateListAndContents ()  const;

  TrainingClassPtr    LocateByImageClass (const MLClassPtr  _mlClass)  const;

  TrainingClassPtr    LocateByImageClassName (const KKStr&  className);

  TrainingClassPtr    LocateByDirectory (const KKStr&  directory);

private: 
  KKStr   rootDirExpanded;
};


typedef  TrainingClassList*  TrainingClassListPtr;

}  /* namespace KKMachineLearning */


#endif
