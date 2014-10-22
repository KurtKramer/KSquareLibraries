#ifndef  _VARIABLES_
#define  _VARIABLES_

#include  "KKStr.h"

/** 
 *@namespace  KKLSC
 *@brief Contains Classes that are specific to Cameras  physical characteristics.
 *@details Classes that support the reading and writing of scanner files that support, 2bit, 3bit, 4bit,
 * and 8bit depth.  Support for embedding text and instrument data as well as Identifying header information
 * in all data formats.
 */
namespace  KKLSC  
{
  /** 
   *@class  Variables
   *@brief Variables that specify where we can locate Camera Application directories and files.
   *@details  All methods and applications are to use these variables to locate files and directories that they need.
   *           This way all the knowledge or where to locate things is kept in this one class.
   */
  class  Variables
  {
  public:
    Variables ();
    ~Variables ();

    static  void  SetHomeDir (const KKStr&  _homeDir);

    static  KKStr  HomeDir                ();  /**< Home Directory.  All other directories will be off this directory.  */
    static  KKStr  ConfigurationDir       ();  /**< Where application configuration files go;  NOT training models.     */
    static  KKStr  ScannerFilesDefaultDir ();


    /**
     *@brief  Expands the supplied string by substituting in Environment variable values.
     *@details  Similar to the 'osSubstituteInEvironmentVariables' method defined in 'osServices' except that if 'HomeDir' is
     *           not defined then the string 'c:\\Temp' will be used.  If a environment variable other than "HomeDir" is not
     *           defined then the original string will be returned.
     *           ex:  '$(HomeDir)\\TrainingLibraries\\Shrimp9G'  will expand to: 'SCS\\TrainingLibraries\\Shrimp9G'
     *
     *@param[in]  src  String that is to be search for environment variables and expanded.
     *@return  returns the expanded string.
     */
    static  KKStr  SubstituteInEvironmentVariables (const KKStr&  src);

  private:
    static  kkint32  LocateEnvStrStart (const KKStr&  str);
    static  KKStr    homeDir;

  };  /* Variables */
}


#endif

