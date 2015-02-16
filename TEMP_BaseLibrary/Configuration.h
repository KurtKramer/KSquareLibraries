/* Configuration.h -- Generic Configuration file manager.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#ifndef  _CONFIGURATION_
#define  _CONFIGURATION_

#include "Str.h"
#include "RunLog.h"


namespace KKU 
{
  /**
   *@brief  General purpose Configuration File manager class.
   *@details  This class will read and write configuration files. It understands the concept of Logical Sections and
   * Variables. You will be able to programmatically define Sections and Settings in these sections. Each Setting
   * will have a related value stored with it.
   *
   *@code
   *Example Configuration File
   *===========================================================
   * [Header]
   * ClassifierType = SVM
   * Parameters = -K RBF  -S SVC -G 1.76 -C 12
   * RootDir = ${PicesHmeDir}\TraningLibraries\SimpleLibrary\
   *
   * [Class]
   * Name = Copepod
   *
   * [Class]
   * Name = Larvacean
   *===========================================================
   *@endcode
   *
   * There are three sections defined in the example above; Header, Class, and Class. Note
   * that Section names do not have to be unique. You can access sections by name or index.
   * If the name is not unique the first instance will be returned. Index values start at
   * the top of the file, that is the first section to appear in the file is section index
   * 0. The Setting names are of the format "SettingName = Value". Again you will be able
   * to access Setting by either name or index.
   */

  class  Configuration
  {
  public:
    Configuration (const KKU::KKStr&  _fileName,
                   RunLog&            _log
                  );

    Configuration (RunLog&  _log);

    Configuration (const Configuration&  c);

    ~Configuration ();

    bool  FormatGood ()                  {return  formatGood;}

    void  FormatGood (bool _formatGood)  {formatGood = _formatGood;}

    const VectorKKStr&   FormatErrors ()  const {return formatErrors;}

    VectorKKStr          FormatErrorsWithLineNumbers ()  const;

    void  FormatErrorsAdd (int32         lineNum, 
                           const KKStr&  error
                          );

    void  FormatErrorsClear ();  /**< @brief Call this to clear all format error messages. */

    void  LoadFile ();

    int32 MemoryConsumedEstimated ()  const;

    int32 NumOfSections ();
  
    int32 NumOfSettings (const KKU::KKStr&  sectionName);

    int32 NumOfSettings (int32  sectionNum)  const;                 /**< @brief Returns number of settings for the specified section, */
   
    void  PrintFormatErrors (ostream& o);

    bool  SectionDefined (const KKU::KKStr&  sectionName)  const;   /**< @brief Returns true if the section is defined. */


    // Access Methods.
    const KKU::KKStr&  FileName () const  {return  fileName;}

    KKStrConstPtr      SectionName (int32 sectionNum)  const;       /**< @brief Returns the name of the section for specified index, if index not defined will return NULL. */

    int32              SectionNum (const KKU::KKStr&  sectionName)  const;

    int32              SectionLineNum (int32 sectionNum)  const;

    KKStrConstPtr      SettingName (const KKU::KKStr&  sectionName, 
                                    int32              settingNum
                                   )  const;
  
    KKStrConstPtr      SettingName (int32  sectionNum,
                                    int32  settingNum
                                   )  const;


    KKStrConstPtr      SettingValue (const KKU::KKStr&  sectionName,
                                     const KKU::KKStr&  settingName,
                                     int32&             lineNum
                                    )  const;


    KKStrConstPtr      SettingValue (int32              sectionNum,
                                     const KKU::KKStr&  settingName,
                                     int32&             lineNum
                                    )  const;


    KKStrConstPtr      SettingValue (int32   sectionNum,
                                     int32   settingNum,
                                     int32&  lineNum
                                    )  const;


    void  GetSetting (const char*     sectiopnName,
                      int32           settingNum,
                      KKStrConstPtr&  name,
                      KKStrConstPtr&  value,
                      int32&          lineNum
                     );

  private:
    class  Setting;
    class  SettingList;
    class  ConfSection;
    class  ConfSectionList;
    typedef  Setting*          SettingPtr;
    typedef  ConfSection*      ConfSectionPtr;
    typedef  ConfSectionList*  ConfSectionListPtr;

    KKU::KKStr          curSectionName;
    KKU::KKStr          fileName;
    bool                formatGood;
    VectorKKStr         formatErrors;   /**< Configuration Format Errors will be recorder here. */
    VectorInt           formatErrorsLineNums;
    RunLog&             log;
    ConfSectionListPtr  sections; 
  };


  typedef  Configuration*  ConfigurationPtr;
}
#endif
