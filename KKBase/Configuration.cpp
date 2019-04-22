/* Configuration.cpp -- Generic Configuration file manager.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"
#include <cstdio>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "MemoryDebug.h"
using namespace std;

#ifdef  WIN32
#include <windows.h>
#endif

#include "Configuration.h"
#include "KKQueue.h"
#include "OSservices.h"
#include "Option.h"
#include "RunLog.h"
using namespace KKB;



namespace  KKB
{
  class  Configuration::Setting
  {
  public:
    Setting (const KKStr&  _name,
             const KKStr&  _value,
             OptionUInt32  _lineNum
            ):
      lineNum (_lineNum),
      name    (_name),
      value   (_value)
    {}

    Setting (const Configuration::Setting&  s):
        lineNum (s.lineNum),
        name    (s.name),
        value   (s.value)
    {}

    OptionUInt32    LineNum  ()  const {return  lineNum;}
    KKStrConstPtr   Name     ()  const {return  &name;}
    KKStrConstPtr   Value    ()  const {return  &value;}

    kkMemSize MemoryConsumedEstimated ()  const  {return  (kkMemSize)sizeof (Setting) + 
                                                          name.MemoryConsumedEstimated () + 
                                                          value.MemoryConsumedEstimated ();}

  private:
    OptionUInt32  lineNum;
    KKStr         name;
    KKStr         value;
  };  /* Setting */



  class  Configuration::SettingList:  public KKQueue<Configuration::Setting>
  {
  public:
    SettingList (): KKQueue<Setting> (true)  {}
  
    SettingList (const  SettingList&  sl):
        KKQueue<Setting> (true)
    {
      kkuint32 x;
      for  (x = 0;  x < sl.size ();  x++)
      {
        SettingPtr  setting = sl.IdxToPtr (x);
        PushOnBack (new Configuration::Setting (*setting));
      }
    }

    kkMemSize MemoryConsumedEstimated ()  const
    {
      kkMemSize  memoryConsumedEstimated = sizeof (SettingList);
      SettingList::const_iterator  idx;
      for  (idx = begin ();  idx != end ();  ++idx)
        memoryConsumedEstimated += (*idx)->MemoryConsumedEstimated ();
      return  memoryConsumedEstimated;
    }


    SettingConstPtr  LookUp (const KKStr&  name)  const
    {
      kkuint32  qSize = QueueSize ();
      for  (kkuint32 idx = 0;  idx < qSize;  idx++)
      {
        SettingConstPtr setting = IdxToPtr (idx);
        if  (name.EqualIgnoreCase (setting->Name ()))
          return setting;
      }
      return  NULL;
    }  /* LookUp */



    OptionUInt32  LookUpLineNum (const KKStr&  name)  const
    {
      const_iterator  idx;
      for  (idx = begin ();  idx != end ();  idx++)
      {
        SettingPtr s = *idx;
        if  (s->Name ()->EqualIgnoreCase (name))
          return s->LineNum ();
      }
      return {};
    }  /* LookUpLineNum */

  

    void  AddSetting (SettingPtr  setting)
    {
      PushOnBack (setting);
    }



    void  AddSetting (const KKStr&  _name,
                      const KKStr&  _value,
                      OptionUInt32  _lineNum
                     )
    {
      PushOnBack (new Setting (_name, _value, _lineNum));
    }
  };  /* SettingList */



  class  Configuration::ConfSection
  {
  public:
    ConfSection (const KKStr& _name,
                 OptionUInt32 _lineNum
                ):
          lineNum  (_lineNum),
          name     (_name),
          settings ()
    {}
      
    ConfSection (const  Configuration::ConfSection&  cs):
           lineNum  (cs.lineNum),
           name     (cs.name),
           settings (cs.settings)
    {}


    OptionUInt32  LineNum ()  const {return lineNum;}

    KKStrConstPtr  Name ()  {return  &name;}

    kkMemSize  MemoryConsumedEstimated ()  const
    {
      return sizeof (lineNum) + name.MemoryConsumedEstimated () + settings.MemoryConsumedEstimated ();
    }



    kkuint32 NumOfSettings ()  const {return  settings.QueueSize ();}



    KKStrConstPtr   SettingName (kkuint32 settingNum)  const
    {
      if  (settingNum >= settings.size ())
        return NULL;
      return  settings[settingNum].Name ();
    }



    KKStrConstPtr   SettingValue (kkuint32      settingNum,
                                  OptionUInt32& settingLineNum
                                 )  const
    {
      if  (settingNum >= settings.size ())
        return NULL;
      settingLineNum = settings[settingNum].LineNum ();
      return  settings[settingNum].Value ();
    }



    void  GetSettings (kkuint32        settingNum,
                       KKStrConstPtr&  settingName,
                       KKStrConstPtr&  value,
                       OptionUInt32&   settingLineNum
                      )
    {
      SettingPtr  setting = settings.IdxToPtr (settingNum);
      if  (setting)
      {
        settingName    = setting->Name ();
        value          = setting->Value ();
        settingLineNum = setting->LineNum ();
      }
      else
      {
        settingName = NULL;
        value = NULL;
        settingLineNum = {};
      }
    } 



    void  AddSetting (const KKStr&  _name,
                      const KKStr&  _value,
                      OptionUInt32  _lineNum
                     )
    {
      settings.AddSetting (_name, _value, _lineNum);
    }



    KKStrConstPtr  LookUpValue (const KKStr&  _name, OptionUInt32& _lineNum)  const
    {
      SettingConstPtr  setting = settings.LookUp (_name);
      if  (setting)
      {
        _lineNum = setting->LineNum ();
        return  setting->Value ();
      }
      else
      {
        _lineNum = {};
        return NULL;
      }
    }

  private:
    OptionUInt32  lineNum;   // Text Line Number where section starts.
    KKStr         name;
    SettingList   settings;
  };  /* ConfSection */



  class  Configuration::ConfSectionList:  public KKQueue<ConfSection>
  {
  public:
    ConfSectionList (): KKQueue<ConfSection> (true)  {}


    kkMemSize MemoryConsumedEstimated ()  const
    {
      kkMemSize  memoryConsumedEstimated = sizeof (ConfSectionList);
      ConfSectionList::const_iterator  idx;
      for  (idx = begin ();  idx != end ();  ++idx)
        memoryConsumedEstimated += (*idx)->MemoryConsumedEstimated ();
      return  memoryConsumedEstimated;
    }


    ConfSectionPtr  LookUp (const KKStr& _name)
    {
      ConfSectionPtr   tempSection = NULL;
      ConfSectionPtr   section = NULL;
      ConfSectionList::const_iterator  idx;
      for  (idx = begin ();  idx != end ();  ++idx)
      {
        tempSection = *idx;
        if  (_name.EqualIgnoreCase (tempSection->Name ()))
        {
          section = tempSection;
          break;
        }
      }

      return  section;
    }  /* LookUp */
    

    void  AddConfSection (ConfSectionPtr  section)
    {
      PushOnBack (section);
    }

    
    void  AddConfSection (const KKStr&  _name,
                          OptionUInt32  _lineNum
                         )
    {
      PushOnBack (new ConfSection (_name, _lineNum));
    }
  };  /* ConfSectionList */
}  /* KKB */



Configuration::Configuration (const KKStr&  _fileName,
                              RunLog&       _log
                             ):
  curSectionName       (),
  fileName             (_fileName),
  formatGood           (true),
  formatErrors         (),
  formatErrorsLineNums (),
  sections             (NULL)
{
  sections = new ConfSectionList ();
  LoadFile (_log);
}


Configuration::Configuration ():
  curSectionName       (),
  fileName             (),
  formatGood           (true),
  formatErrors         (),
  formatErrorsLineNums (),
  sections             (NULL)
{
  sections = new ConfSectionList ();
}



Configuration::Configuration (const Configuration&  c):
  
  curSectionName       (c.curSectionName),
  fileName             (c.fileName),
  formatGood           (c.formatGood),
  formatErrors         (c.formatErrors),
  formatErrorsLineNums (c.formatErrorsLineNums),
  sections             (NULL)
{
  sections = new ConfSectionList ();

  for  (kkuint32 x = 0;  x < sections->QueueSize ();  x++)
  {
    const ConfSectionPtr  cs = sections->IdxToPtr (x);
    sections->AddConfSection (new ConfSection (*cs));
  }
}



Configuration::~Configuration ()
{
  delete  sections;
}



kkMemSize Configuration::MemoryConsumedEstimated ()  const
{
  kkMemSize  memoryConsumedEstimated = sizeof (Configuration)
    + curSectionName.MemoryConsumedEstimated ()
    + fileName.MemoryConsumedEstimated ()
    + (kkMemSize)formatErrors.size () * 100
    + (kkMemSize)formatErrorsLineNums.size () * sizeof (OptionUInt32);

  if  (sections)
    memoryConsumedEstimated += sections->MemoryConsumedEstimated ();

  return  memoryConsumedEstimated;
}  /* MemoryConsumedEstimated */



void  StripOutAnyComments (KKStr&  line)
{
  auto idx = line.LocateStr("//");
  if  (idx)
    line = line.SubStrSeg(0, idx);
} /* StripOutAnyComments */
 


void  Configuration::PrintFormatErrors (ostream& o)
{
  o << endl
    << "Num" << "\t" << "LineNum" << "\t" << "Description" << endl;

  for  (kkuint32 idx = 0;  idx < formatErrors.size ();  ++idx)
  {
    o << idx << "\t" << formatErrorsLineNums[idx]  << "\t"  << formatErrors[idx] << endl;
  }
}  /* PrintFormatErrors */



void  Configuration::Load (const KKB::KKStr&  _fileName,
                           RunLog&            _log
                          )
{
  fileName = _fileName;
  LoadFile (_log);
}



void  Configuration::LoadFile (RunLog&  log)
{
  log.Level (10) << "Configuration::LoadFile: " << fileName << endl;

  kkuint32  lastLineNum = 0;

  if  (fileName == "")
  {
    log.Level (-1) << endl
                   << "Configuration::LoadFile   ***ERROR***   File-Name is blank"  << endl
                   << endl;
    FormatGood (false);
    return;
  }

  FILE*  inFile = osFOPEN (fileName.Str (), "r");

  if  (!inFile)
  {
    log.Level (-1) << endl
                   << "Configuration::LoadFile   ***ERROR***    Opening File: " << fileName << endl
                   << endl;

    FormatGood (false);
    return;
  }

  char  buff[10240];
  kkint32 lineCount = 0;

  curSectionName = "";
  ConfSectionPtr  curSection = NULL;

  while  (std::fgets (buff, sizeof (buff), inFile))
  {
    lastLineNum++;
    KKStr  line (buff);
    line.TrimRight ();
    line.TrimLeft ();

    StripOutAnyComments (line);

    log.Level (70) << line << endl;
    
    StripOutAnyComments (line);

    if  (line.Empty ())            
    {
      // If we have a blank line, we do nothing.
    }

    else if  (line.FirstChar () == '[')
    {
      // Looks like definition of new section. 

      if  (line.LastChar () == ']')
      {
        curSectionName = line.SubStrSeg (1, line.Len () - 2);
        curSectionName.TrimLeft ();
        curSectionName.TrimRight ();
        curSectionName.Upper ();

        curSection = new ConfSection (curSectionName, lastLineNum);
        sections->AddConfSection (curSection);
        log.Level (30) << "LoadFile   SectionName[" << curSectionName << "]." << endl;
      }
      else
      {
        log.Level (-1) << endl
                       << "Configuration::LoadFile   ***ERROR***    LineNumber[" << lastLineNum << "]  Improper Section Name[" << line << "]." << endl
                       << endl;
        formatGood = false;
      }
    }

    else
    {
      if  (!curSection)
      {
        log.Level (-1) << endl
                       << "Configuration::LoadFile   ***ERROR***  Format Error LineNumber[" << lastLineNum << "]" << endl
                       << "                            No Section Defined."  << endl 
                       << endl;
     
        formatGood = false;

        curSectionName = "GLOBAL";
        curSection = new ConfSection (curSectionName, lastLineNum);
        sections->AddConfSection (curSection);
      }

      auto equalIdx = line.LocateCharacter ('=');
      if  (!equalIdx)
      {
        // We have a improperly formated line.
        log.Level (-1) << endl
                       << "Configuration::LoadFile   ***ERROR***   LineNumber[" << lastLineNum << "] Improperly Formated Line[" << line << "]." 
                       << endl;
        formatGood = false;
      }

      else
      {
        KKStr  settingName (line.SubStrSeg (0, equalIdx));
        settingName.TrimLeft ();
        settingName.TrimRight ();
        settingName.Upper ();

        KKStr  settingValue (line.SubStrPart (equalIdx + 1));
        settingValue.TrimLeft ();
        settingValue.TrimRight ();

        log.Level (30) << "LoadFile   SectionName[" << curSectionName << "], "
                       << "Setting[" << settingName << "], Value[" << settingValue   << "]."
                       << endl;

        curSection->AddSetting (settingName, settingValue, lastLineNum);
      }

      lineCount++;
    }
  }

  std::fclose (inFile);
}  /* LoadFile */



kkuint32  Configuration::NumOfSections ()
{
  return  sections->QueueSize ();
}



OptionUInt32  Configuration::NumOfSettings (const KKStr&  sectionName)  const
{
  ConfSectionPtr  section = sections->LookUp (sectionName);

  if  (!section)
    return {};

  return  section->NumOfSettings ();
}



OptionUInt32  Configuration::NumOfSettings (kkuint32  sectionNum)  const
{
  if  (sectionNum >= sections->QueueSize ())
    return {};

  return  sections->IdxToPtr (sectionNum) ->NumOfSettings ();
}


   
bool  Configuration::SectionDefined (const KKStr&  sectionName)  const
{
  ConfSectionPtr  section = sections->LookUp (sectionName);
  return (section != NULL);
}



KKStrConstPtr   Configuration::SectionName (kkuint32 sectionNum)  const
{
  ConfSectionPtr  section = sections->IdxToPtr (sectionNum);

  if  (!section)
    return NULL;
  else
    return  section->Name ();
}


OptionUInt32  Configuration::SectionNum (const KKStr&  sectionName)  const
{
  if  (!sections)
    return {};

  kkuint32  idx = 0;
  while  (idx < sections->size ())
  {
    if  (sections->IdxToPtr(idx)->Name ()->EqualIgnoreCase (sectionName))
      return  idx;
    ++idx;
  }
  return {};
}



OptionUInt32  Configuration::SectionLineNum (kkuint32 sectionNum)  const
{
  ConfSectionPtr  section = sections->IdxToPtr (sectionNum);

  if  (!section)
    return  {};
  else
    return  section->LineNum ();
}



KKStrConstPtr   Configuration::SettingName (const KKStr&  sectionName,
                                            kkuint32      settingNum
                                           )  const
{
  ConfSectionPtr  section = sections->LookUp (sectionName);
  if  (!section)
    return NULL;

  return  section->SettingName (settingNum);
}



KKStrConstPtr   Configuration::SettingName (kkuint32  sectionNum,
                                            kkuint32  settingNum
                                           )  const
{
  if  (sectionNum >= sections->QueueSize ())
    return NULL;

  if  (settingNum >= (*sections)[sectionNum].NumOfSettings ())
    return NULL;

  return  sections->IdxToPtr (sectionNum)->SettingName (settingNum);
}



KKStrConstPtr   Configuration::SettingValue (kkuint32      sectionNum,
                                             const KKStr&  settingName,
                                             OptionUInt32& lineNum
                                            )  const

{
  KKStrConstPtr  result = NULL;
  ConfSectionPtr  section = sections->IdxToPtr (sectionNum);
  if  (!section)
  {
    lineNum = {};
  }
  else
  {
    result = section->LookUpValue (settingName, lineNum);
  }
  return  result;
}



KKStr   Configuration::SettingValueToStr (kkuint32      sectionNum,
                                          const KKStr&  settingName,
                                          OptionUInt32& lineNum
                                         )  const

{
  KKStrConstPtr  result = NULL;
  ConfSectionPtr  section = sections->IdxToPtr (sectionNum);
  if  (!section)
  {
    lineNum = {};
  }
  else
  {
    result = section->LookUpValue (settingName, lineNum);
  }

  if  (result == NULL)
    return KKStr::EmptyStr ();
  else
    return KKStr (*result);
}



KKStrConstPtr   Configuration::SettingValue (kkuint32      sectionNum,
                                             kkuint32      settingNum,
                                             OptionUInt32& lineNum
                                            )  const
{
  if  (sectionNum >= sections->QueueSize ())
    return NULL;

  if  (settingNum >= (*sections)[sectionNum].NumOfSettings ())
    return NULL;

  return  sections->IdxToPtr (sectionNum)->SettingValue (settingNum, lineNum);
}



KKStrConstPtr   Configuration::SettingValue (const KKB::KKStr&  sectionName,
                                             const KKB::KKStr&  settingName,
                                             OptionUInt32&      lineNum
                                            )  const
{
  auto  sectionNum = SectionNum (sectionName);
  if  (!sectionNum)
    return NULL;

  return  SettingValue (sectionNum.value (), settingName, lineNum);
}



KKStr  Configuration::SettingValueToStr (const KKB::KKStr&  sectionName,
                                         const KKB::KKStr&  settingName,
                                         OptionUInt32&      lineNum
                                        )  const
{
  auto  sectionNum = SectionNum (sectionName);
  if  (!sectionNum)
    return KKStr::EmptyStr ();

  return  SettingValueToStr (sectionNum.value (), settingName, lineNum);
}



void  Configuration::GetSetting (const char*     sectionName,
                                 kkuint32        settingNum,
                                 KKStrConstPtr&  name,
                                 KKStrConstPtr&  value,
                                 OptionUInt32&   lineNum
                                )
{
  ConfSectionPtr  section = sections->LookUp (sectionName);
  if  (section)
  {
    section->GetSettings (settingNum, name, value, lineNum);
  }
  else
  {
    name    = NULL;
    value   = NULL;
    lineNum = {};
  }
}



void  Configuration::FormatErrorsAdd (OptionUInt32  lineNum,
                                      const KKStr&  error
                                     )
{
  formatErrors.push_back (error);
  formatErrorsLineNums.push_back (lineNum);
}



void  Configuration::FormatErrorsClear ()  /**< Call this to clear all format error messages. */
{
  formatErrors.clear ();
  formatErrorsLineNums.clear ();
}



VectorKKStr  Configuration::FormatErrorsWithLineNumbers ()  const
{
  VectorKKStr  errorMsgs;
  for  (kkuint32 i = 0;  i < formatErrors.size ();  i++)
  {
    KKStr  lineNumStr = "    ";
    if  (i < formatErrorsLineNums.size ())
      lineNumStr = StrFormatInt (formatErrorsLineNums[i].value (), "0000");
    errorMsgs.push_back (lineNumStr + ":" + formatErrors[i]); 
  }

  return  errorMsgs;
}  /* FormatErrorsWithLineNumbers */
