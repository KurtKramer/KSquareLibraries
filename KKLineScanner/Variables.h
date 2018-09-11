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

  private:
    static  KKStr   homeDir;

  };  /* Variables */
}


#endif

