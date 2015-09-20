/* Configuration.cpp -- Generic Configuration file manager.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"
#include <stdio.h>
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
#include "RunLog.h"
using namespace KKB;



namespace  KKB
{
  class  Configuration::Setting
  {
  public:
    Setting (const KKStr&  _name,
             const KKStr&  _value,
             kkint32       _lineNum
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

    kkint32         LineNum  ()  const {return  lineNum;}
    KKStrConstPtr   Name     ()  const {return  &name;}
    KKStrConstPtr   Value    ()  const {return  &value;}

    kkint32 MemoryConsumedEstimated ()  const  {return  sizeof (Setting) + name.MemoryConsumedEstimated () + value.MemoryConsumedEstimated ();}

  private:
    kkint32  lineNum;
    KKStr  name;
    KKStr  value;
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

    kkint32 MemoryConsumedEstimated ()  const
    {
      kkint32  memoryConsumedEstimated = sizeof (SettingList);
      SettingList::const_iterator  idx;
      for  (idx = begin ();  idx != end ();  ++idx)
        memoryConsumedEstimated += (*idx)->MemoryConsumedEstimated ();
      return  memoryConsumedEstimated;
    }


    SettingPtr  LookUp (const KKStr&  name)
    {
      kkint32  idx;
      kkint32  qSize = QueueSize ();
      for  (idx = 0;  idx < qSize;  idx++)
      {
        SettingPtr setting = IdxToPtr (idx);
        if  (name.EqualIgnoreCase (setting->Name ()))
          return setting;
      }
      return  NULL;
    }  /* LookUp */



    kkint32  LookUpLineNum (const KKStr&  name)  const
    {
      const_iterator  idx;
      for  (idx = begin ();  idx != end ();  idx++)
      {
        SettingPtr s = *idx;
        if  (s->Name ()->EqualIgnoreCase (name))
          return s->LineNum ();
      }
      return -1;
    }  /* LookUpLineNum */

  
    void  AddSetting (SettingPtr  setting)
    {
      PushOnBack (setting);
    }


    void  AddSetting (const KKStr&  _name,
                      const KKStr&  _value,
                      kkint32       _lineNum
                     )
    {
      PushOnBack (new Setting (_name, _value, _lineNum));
    }
  };  /* SettingList */




  class  Configuration::ConfSection
  {
  public:
    ConfSection (const KKStr& _name,
                 kkint32      _lineNum
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


    kkint32  LineNum ()  const {return lineNum;}

    KKStrConstPtr   Name ()  {return  &name;}

    kkint32 MemoryConsumedEstimated ()  const
    {
      return sizeof (lineNum) + name.MemoryConsumedEstimated () + settings.MemoryConsumedEstimated ();
    }


    kkint32 NumOfSettings ()  const {return  settings.QueueSize ();}


    KKStrConstPtr   SettingName (kkint32 settingNum)  const
    {
      if  (settingNum >= (kkint32)settings.size ())
        return NULL;
      return  settings[settingNum].Name ();
    }

    KKStrConstPtr   SettingValue (kkint32 settingNum, kkint32& lineNum)  const
    {
      if  (settingNum >= (kkint32)settings.size ())
        return NULL;
      lineNum = settings[settingNum].LineNum ();
      return  settings[settingNum].Value ();
    }

    void  GetSettings (kkint32         settingNum,
                       KKStrConstPtr&  name,
                       KKStrConstPtr&  value,
                       kkint32&        lineNum
                      )
    {
      SettingPtr  setting = settings.IdxToPtr (settingNum);
      if  (setting)
      {
        name    = setting->Name ();
        value   = setting->Value ();
        lineNum = setting->LineNum ();
      }
      else
      {
        name  = NULL;
        value = NULL;
        lineNum = -1;
      }
    } 


    void  AddSetting (const KKStr&  _name,
                      const KKStr&  _value,
                      kkint32       _lineNum
                     )
    {
      settings.AddSetting (_name, _value, _lineNum);
    }

    KKStrConstPtr   LookUpValue (const KKStr&  _name, kkint32& lineNum)
    {
      SettingPtr  setting = settings.LookUp (_name);
      if  (setting)
      {
        lineNum = setting->LineNum ();
        return  setting->Value ();
      }
      else
      {
        lineNum = -1;
        return NULL;
      }
    }

  private:
    kkint32      lineNum;   // Text Line Number where section starts.
    KKStr        name;
    SettingList  settings;
  };  /* ConfSection */



  class  Configuration::ConfSectionList:  public KKQueue<ConfSection>
  {
  public:
    ConfSectionList (): KKQueue<ConfSection> (true)  {}


    kkint32 MemoryConsumedEstimated ()  const
    {
      kkint32  memoryConsumedEstimated = sizeof (ConfSectionList);
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
                          kkint32       _lineNum
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

  kkint32  x;

  for  (x = 0;  x < sections->QueueSize ();  x++)
  {
    const ConfSectionPtr  cs = sections->IdxToPtr (x);
    sections->AddConfSection (new ConfSection (*cs));
  }
}



Configuration::~Configuration ()
{
  delete  sections;
}



kkint32 Configuration::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = sizeof (Configuration)
    + curSectionName.MemoryConsumedEstimated ()
    + fileName.MemoryConsumedEstimated ()
    + (kkint32)formatErrors.size () * 100
    + (kkint32)formatErrorsLineNums.size () * sizeof (kkint32);

  if  (sections)
    memoryConsumedEstimated += sections->MemoryConsumedEstimated ();

  return  memoryConsumedEstimated;
}  /* MemoryConsumedEstimated */



void  StripOutAnyComments (KKStr&  line)
{
  bool found = false;
  kkint32  len   = line.Len ();
  kkint32  x     = 0;
 

  while  ((x < (len - 1))  &&  (!found))
  {
    if  ((line[x]     == '/')  &&
         (line[x + 1] == '/'))
      found = true;
    else
      x++;
  }

  if  (found)
  {
    if  (x == 0)
      line = "";
    else
      line = line.SubStrPart (0, x - 1);
  }

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

  kkint32  lastLineNum = 0;

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

  while  (fgets (buff, sizeof (buff), inFile))
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
        curSectionName = line.SubStrPart (1, line.Len () - 2);
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

      kkint32  equalIdx = line.LocateCharacter ('=');

      if  (equalIdx < 0)
      {
        // We have a improperly formated line.
        log.Level (-1) << endl
                       << "Configuration::LoadFile   ***ERROR***   LineNumber[" << lastLineNum << "] Improperly Formated Line[" << line << "]." 
                       << endl;
        formatGood = false;
      }

      else
      {
        KKStr  settingName (line.SubStrPart (0, equalIdx - 1));
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

  fclose (inFile);
}  /* LoadFile */




kkint32  Configuration::NumOfSections ()
{
  return  sections->QueueSize ();
}



kkint32  Configuration::NumOfSettings (const KKStr&  sectionName)  const
{
  ConfSectionPtr  section = sections->LookUp (sectionName);

  if  (!section)
    return (-1);

  return  section->NumOfSettings ();
}




kkint32  Configuration::NumOfSettings (kkint32  sectionNum)  const
{
  if  ((sectionNum < 0)  ||  (sectionNum >= sections->QueueSize ()))
    return -1;

  return  sections->IdxToPtr (sectionNum) ->NumOfSettings ();
}





   
bool  Configuration::SectionDefined (const KKStr&  sectionName)  const
{
  ConfSectionPtr  section = sections->LookUp (sectionName);
  return (section != NULL);
}



KKStrConstPtr   Configuration::SectionName (kkint32 sectionNum)  const
{
  ConfSectionPtr  section = sections->IdxToPtr (sectionNum);

  if  (!section)
    return NULL;
  else
    return  section->Name ();
}


kkint32  Configuration::SectionNum (const KKStr&  sectionName)  const
{
  if  (!sections)
    return -1;

  kkuint32  idx = 0;
  while  (idx < sections->size ())
  {
    if  (sections->IdxToPtr(idx)->Name ()->EqualIgnoreCase (sectionName))
      return  (kkint32)idx;
    idx++;
  }
  return -1;
}



kkint32  Configuration::SectionLineNum (kkint32 sectionNum)  const
{
  ConfSectionPtr  section = sections->IdxToPtr (sectionNum);

  if  (!section)
    return  -1;
  else
    return  section->LineNum ();
}



KKStrConstPtr   Configuration::SettingName (const KKStr&  sectionName,
                                            kkint32       settingNum
                                           )  const
{
  ConfSectionPtr  section = sections->LookUp (sectionName);
  if  (!section)
    return NULL;

  return  section->SettingName (settingNum);
}




KKStrConstPtr   Configuration::SettingName (kkint32  sectionNum,
                                            kkint32  settingNum
                                           )  const
{
  if  ((sectionNum < 0)  ||  (sectionNum >= sections->QueueSize ()))
    return NULL;

  if  ((settingNum < 0)  ||  (settingNum >= (*sections)[sectionNum].NumOfSettings ()))
    return NULL;


  return  sections->IdxToPtr (sectionNum)->SettingName (settingNum);
}



KKStrConstPtr   Configuration::SettingValue (kkint32       sectionNum,
                                             const KKStr&  settingName,
                                             kkint32&      lineNum
                                            )  const

{
  KKStrConstPtr  result = NULL;
  ConfSectionPtr  section = sections->IdxToPtr (sectionNum);
  if  (!section)
  {
    lineNum = -1;
  }
  else
  {
    result = section->LookUpValue (settingName, lineNum);
  }
  return  result;
}




KKStrConstPtr   Configuration::SettingValue (kkint32 sectionNum,
                                             kkint32 settingNum,
                                             kkint32&  lineNum
                                            )  const
{
  if  ((sectionNum < 0)  ||  (sectionNum >= sections->QueueSize ()))
    return NULL;

  if  ((settingNum < 0)  ||  (settingNum >= (*sections)[sectionNum].NumOfSettings ()))
    return NULL;


  return  sections->IdxToPtr (sectionNum)->SettingValue (settingNum, lineNum);
}




KKStrConstPtr   Configuration::SettingValue (const KKB::KKStr&  sectionName,
                                             const KKB::KKStr&  settingName,
                                             kkint32&           lineNum
                                            )  const
{
  kkint32  sectionNum = SectionNum (sectionName);
  if  (sectionNum < 0)
    return NULL;

  return  SettingValue (sectionNum, settingName, lineNum);
}






void  Configuration::GetSetting (const char*     sectionName,
                                 kkint32         settingNum,
                                 KKStrConstPtr&  name,
                                 KKStrConstPtr&  value,
                                 kkint32&        lineNum
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
    lineNum = -1;
  }
}




void  Configuration::FormatErrorsAdd (kkint32       lineNum,  
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
      lineNumStr = StrFormatInt (formatErrorsLineNums[i], "0000");
    errorMsgs.push_back (lineNumStr + ":" + formatErrors[i]); 
  }

  return  errorMsgs;
}  /* FormatErrorsWithLineNumbers */


