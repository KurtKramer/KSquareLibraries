/* Configuration.h -- Generic Configuration file manager.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#if  !defined(_CONFIGURATION_)
#define  _CONFIGURATION_

#include "KKQueue.h"
#include "KKStr.h"
#include "Option.h"
#include "RunLog.h"


namespace KKB 
{
  /**
   *@brief  General purpose Configuration File manager class.
   *@details  This class will read and write configuration files.  It understands the concept of Logical Sections and 
   * Variables.  You will be able to pragmatically define Sections and Settings in these sections.  Each Setting
   * will have a related value stored with it.
   *
   *@code
   *Example Configuration File
   *===========================================================
   * [Header]
   * ClassifierType = SVM
   * Parameters = -K RBF  -S SVC -G 1.76 -C 12
   * RootDir = ${TFTCHomeDir}\\Classifier\\TrainingLibraries\\
   *
   * [Class]
   * Name = Shrimp_01
   *
   * [Class]
   * Name = Shrimp_02
   *===========================================================
   *@endcode
   *
   * There are three sections defined in the example above; Header, Class, and Class.  Note
   * that Section names do not have to be unique.  You can access sections by name or index.
   * If the name is not unique the first instance will be returned.  Index values start at
   * the top of the file, that is the first section to appear in the file is section index
   * 0.  The Setting names are of the format "SettingName = Value".  Again you will be able 
   * to access Setting by either name or index.
   */

  class  Configuration
  {
  public:
    Configuration (const KKB::KKStr&  _fileName,
                   RunLog&            _log
                  );

    Configuration ();

    Configuration (const Configuration&  c);

    virtual  ~Configuration ();

    virtual
    void  Load (const KKB::KKStr&  _fileName,
                RunLog&            _log
               );


    bool  FormatGood ()  const  {return  formatGood;}

    void  FormatGood (bool _formatGood)  {formatGood = _formatGood;}

    const VectorKKStr&         FormatErrors         ()  const {return formatErrors;}
    const VectorOptionUInt32&  FormatErrorsLineNums ()  const {return formatErrorsLineNums;}

    VectorKKStr  FormatErrorsWithLineNumbers ()  const;

    void  FormatErrorsAdd (OptionUInt32  lineNum,
                           const KKStr&  error
                          );

    void  FormatErrorsClear ();  /**< @brief Call this to clear all format error messages. */

    void  LoadFile (RunLog& log);

    virtual kkMemSize  MemoryConsumedEstimated ()  const;

    kkuint32      NumOfSections ();
  
    OptionUInt32  NumOfSettings (const KKB::KKStr&  sectionName) const;

    OptionUInt32  NumOfSettings (kkuint32  sectionNum)  const;                /**< @brief Returns number of settings for the specified section, */
   
    void  PrintFormatErrors (std::ostream& o);

    bool  SectionDefined (const KKB::KKStr&  sectionName)  const;      /**< @brief Returns true if the section is defined. */


    // Access Methods.
    const KKB::KKStr&  FileName () const  {return  fileName;}

    KKStrConstPtr      SectionName (kkuint32 sectionNum)  const;        /**< @brief Returns the name of the section for specified index, if index not defined will return NULL. */

    OptionUInt32       SectionNum (const KKB::KKStr&  sectionName)  const;

    OptionUInt32       SectionLineNum (kkuint32 sectionNum)  const;

    KKStrConstPtr      SettingName (const KKB::KKStr&  sectionName, 
                                    kkuint32           settingNum
                                   )  const;
  
    KKStrConstPtr      SettingName (kkuint32  sectionNum,
                                    kkuint32  settingNum
                                   )  const;


    KKStrConstPtr      SettingValue (const KKB::KKStr&  sectionName,
                                     const KKB::KKStr&  settingName,
                                     OptionUInt32&      lineNum
                                    )  const;


    KKStr              SettingValueToStr (const KKB::KKStr&  sectionName,
                                          const KKB::KKStr&  settingName,
                                          OptionUInt32&      lineNum
                                         )  const;


    KKStrConstPtr      SettingValue (kkuint32           sectionNum,
                                     const KKB::KKStr&  settingName,
                                     OptionUInt32&      lineNum
                                    )  const;


    KKStr              SettingValueToStr (kkuint32           sectionNum,
                                          const KKB::KKStr&  settingName,
                                          OptionUInt32&      lineNum
                                         )  const;


    KKStrConstPtr      SettingValue (kkuint32      sectionNum,
                                     kkuint32      settingNum,
                                     OptionUInt32& lineNum
                                    )  const;


    void  GetSetting (const char*     sectiopnName,
                      kkuint32        settingNum,
                      KKStrConstPtr&  name,
                      KKStrConstPtr&  value,
                      OptionUInt32&   lineNum
                     );

  private:
    class  Setting;
    class  SettingList;
    class  ConfSection;
    class  ConfSectionList;
    typedef  Setting*          SettingPtr;
    typedef  Setting const *   SettingConstPtr;
    typedef  ConfSection*      ConfSectionPtr;
    typedef  ConfSectionList*  ConfSectionListPtr;

    KKB::KKStr            curSectionName;
    KKB::KKStr            fileName;
    bool                  formatGood;
    VectorKKStr           formatErrors;   /**< Configuration Format Errors will be recorder here. */
    VectorOptionUInt32    formatErrorsLineNums;
    ConfSectionListPtr    sections; 
  };  /* Configuration */

  typedef  Configuration*  ConfigurationPtr;

#define  _Configuration_Defined_

}
#endif
