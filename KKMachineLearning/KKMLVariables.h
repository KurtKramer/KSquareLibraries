#if  !defined(_KKMLVARIABLES_)
#define  _KKMLVARIABLES_




/** 
 *@namespace  KKMachineLearning
 *@brief This library provides data structures that support Machine Learning functionality.
 *@details There are several classes defined in this library but the central ones that should be understood are
 * MLClass, FeatureVector, Model, FileDesc, and Attribute.  there is support for reading and writing several
 * feature Data file formats  C45 probably being the most well known.<p>
 */

namespace KKMachineLearning 
{
  /** 
   *@class  Variables
   *@brief Variables that specific to the Machine Learning Library.
   *@details  All methods in this library are to use these variables to locate files and directories that they need.
   */
  class KKMLVariables
  {
  public:
    KKMLVariables ();
    ~KKMLVariables ();

    static  KKStr  TempDir              ();  /**< Directory where we can put temporary files into.                    */
    static  KKStr  TrainingLibrariesDir ();  /**< Directory where training library directories will be under.         */
    static  KKStr  TrainingModelsDir    ();  /**< Directory where all Training Model Configuration files are stored.  */

    static  void   SetMachineLearninigHomeDir (const KKStr&  _machineLearningHomeDir);

  private:
    static  const KKStr&  MachineLearningHomeDir ();

    static  KKStr  machineLearningHomeDir;
  };  /* KKMLVariables */
}  /* KKMachineLearning*/
#endif

