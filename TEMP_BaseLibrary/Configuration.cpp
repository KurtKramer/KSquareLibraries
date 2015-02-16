/* Configuration.cpp -- Generic Configuration file manager.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#include  "FirstIncludes.h"
#include  <stdio.h>
#include  <stdlib.h>
#include  <string>
#include  <iostream>
#include  <fstream>
#include  <vector>
#include  "MemoryDebug.h"
using namespace std;


#ifdef  WIN32
#include <windows.h>
#endif


#include  "Configuration.h"
#include  "OSservices.h"
#include  "RunLog.h"
using namespace KKU;



namespace  KKU
{
  class  Configuration::Setting
  {
  public:
    Setting (const KKStr&  _name,
             const KKStr&  _value,
             int32         _lineNum
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

    int32          LineNum ()  const {return  lineNum;}
    KKStrConstPtr  Name    ()  const {return  &name;}
    KKStrConstPtr  Value   ()  const {return  &value;}

    int32 MemoryConsumedEstimated ()  const  {return  sizeof (Setting) + name.MemoryConsumedEstimated () + value.MemoryConsumedEstimated ();}

  private:
    int32  lineNum;
    KKStr  name;
    KKStr  value;
  };  /* Setting */



  class  Configuration::SettingList:  public KKQueue<Configuration::Setting>
  {
  public:
    SettingList (): KKQueue<Setting> (true, 5)  {}
  
    SettingList (const  SettingList&  sl):
        KKQueue<Setting> (true, sl.QueueSize ())
    {
      int32 x;
      for  (x = 0;  x < sl.QueueSize ();  x++)
      {
        SettingPtr  setting = sl.IdxToPtr (x);
        PushOnBack (new Configuration::Setting (*setting));
      }
    }

    int32 MemoryConsumedEstimated ()  const
    {
      int32  memoryConsumedEstimated = sizeof (SettingList);
      SettingList::const_iterator  idx;
      for  (idx = begin ();  idx != end ();  ++idx)
        memoryConsumedEstimated += (*idx)->MemoryConsumedEstimated ();
      return  memoryConsumedEstimated;
    }


    SettingPtr  LookUp (const KKStr&  name)
    {
      int32        idx;
      int32        qSize = QueueSize ();
      SettingPtr   setting = NULL;
      SettingPtr   tempSetting;
  
      for  (idx = 0; ((idx < qSize) && (!setting)); idx++)
      {
        tempSetting = IdxToPtr (idx);
        if  (name.EqualIgnoreCase (tempSetting ->Name ()))
          setting = tempSetting;
      }

      return  setting;
    }  /* LookUp */



    int32  LookUpLineNum (const KKStr&  name)  const
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
                      int32         _lineNum
                     )
    {
      PushOnBack (new Setting (_name, _value, _lineNum));
    }
  };  /* SettingList */




  class  Configuration::ConfSection
  {
  public:
    ConfSection (const KKStr& _name,
                 int32        _lineNum
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


    int32  LineNum ()  const {return lineNum;}

    KKStrConstPtr  Name ()  {return  &name;}

    int32 MemoryConsumedEstimated ()  const
    {
      return sizeof (lineNum) + name.MemoryConsumedEstimated () + settings.MemoryConsumedEstimated ();
    }

    int32 NumOfSettings ()  {return  settings.QueueSize ();}


    KKStrConstPtr  SettingName (int32 settingNum)  const
    {
      if  (settingNum >= (int32)settings.size ())
        return NULL;
      return  settings[settingNum].Name ();
    }

    KKStrConstPtr  SettingValue (int32 settingNum, int32& lineNum)  const
    {
      if  (settingNum >= (int32)settings.size ())
        return NULL;
      lineNum = settings[settingNum].LineNum ();
      return  settings[settingNum].Value ();
    }

    void  GetSettings (int32           settingNum,
                       KKStrConstPtr&  name,
                       KKStrConstPtr&  value,
                       int32&          lineNum
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
                      int32         _lineNum
                     )
    {
      settings.AddSetting (_name, _value, _lineNum);
    }


    KKStrConstPtr  LookUpValue (KKStr  _name, int32& lineNum)
    {
      SettingPtr  setting = settings.LookUp (_name);
      if  (setting)
      {
        lineNum = setting->LineNum ();
        return  setting->Value ();
      }
      else
        return NULL;
    }

  private:
    int32        lineNum;   // Text Line Number where section starts.
    KKStr        name;
    SettingList  settings;
  };  /* ConfSection */



  class  Configuration::ConfSectionList:  public KKQueue<ConfSection>
  {
  public:
    ConfSectionList (): KKQueue<ConfSection> (true, 5)  {}


    int32 MemoryConsumedEstimated ()  const
    {
      int32  memoryConsumedEstimated = sizeof (ConfSectionList);
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
                          int32         _lineNum
                         )
    {
      PushOnBack (new ConfSection (_name, _lineNum));
    }
  };  /* ConfSectionList */
}  /* KKU */



Configuration::Configuration (const KKStr&  _fileName,
                              RunLog&       _log
                             ):
  curSectionName (),
  fileName       (_fileName),
  formatGood     (true),
  log            (_log),
  sections       (NULL)
{
  sections = new ConfSectionList ();
  LoadFile ();
}


Configuration::Configuration (RunLog&  _log):
  curSectionName (),
  fileName       (),
  formatGood     (true),
  log            (_log),
  sections       (NULL)
{
  sections = new ConfSectionList ();
}



Configuration::Configuration (const Configuration&  c):
  
  curSectionName (c.curSectionName),
  fileName       (c.fileName),
  formatGood     (c.formatGood),
  log            (c.log),
  sections       (NULL)
{
  sections = new ConfSectionList ();

  int32  x;

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



int32 Configuration::MemoryConsumedEstimated ()  const
{
  int32  memoryConsumedEstimated = sizeof (Configuration)
    + curSectionName.MemoryConsumedEstimated ()
    + fileName.MemoryConsumedEstimated ()
    + formatErrors.size () * 100
    + formatErrorsLineNums.size () * sizeof (int32);

  if  (sections)
    memoryConsumedEstimated += sections->MemoryConsumedEstimated ();

  return  memoryConsumedEstimated;
}  /* MemoryConsumedEstimated */



void  StripOutAnyComments (KKStr&  line)
{
  bool found = false;
  int32  len   = line.Len ();
  int32  x     = 0;
 

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

  for  (uint32 idx = 0;  idx < formatErrors.size ();  ++idx)
  {
    o << idx << "\t" << formatErrorsLineNums[idx]  << "\t"  << formatErrors[idx] << endl;
  }
}  /* PrintFormatErrors */



void  Configuration::LoadFile ()
{
  log.Level (10) << "Configuration::LoadFile[" << fileName << "]." << endl;

  int32  lastLineNum = 0;

  if  (fileName == "")
  {
    log.Level (-1) << "***ERROR*** Configuration::LoadFile   **** Opening File[" << fileName << "]. ****  Name is Blank ****" 
                   << endl
                   << endl;
    FormatGood (false);
    return;
  }

  FILE*  inFile = osFOPEN (fileName.Str (), "r");

  if  (!inFile)
  {
    log.Level (-1) << endl << endl << endl
                   << "***ERROR*** Configuration::LoadFile   **** Opening File[" << fileName << "]. ****" << endl
                   << endl
                   << endl;

    FormatGood (false);
    return;
  }

  char  buff[10240];
  int32 lineCount = 0;

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
        log.Level (-1) << "Configuration::LoadFile   ***ERROR***    LineNumber[" << lastLineNum << "]  Improper Section Name[" << line << "]." 
                       << endl << endl;
        formatGood = false;
      }
    }

    else
    {
      if  (!curSection)
      {
        log.Level (-1) << "Configuration::LoadFile   ***ERROR***  Format Error LineNumber[" << lastLineNum << "]" << endl;
        log.Level (-1) << "                            No Section Defined."  << endl << endl;
     
        formatGood = false;

        curSectionName = "GLOBAL";
        curSection = new ConfSection (curSectionName, lastLineNum);
        sections->AddConfSection (curSection);
      }

      int32  equalIdx = line.LocateCharacter ('=');

      if  (equalIdx < 0)
      {
        // We have a improperly formated line.
        log.Level (-1) << "Configuration::LoadFile ***ERROR***   LineNumber[" << lastLineNum << "] Impropperly Formated Line[" 
                       << line << "]." 
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




int32  Configuration::NumOfSections ()
{
  return  sections->QueueSize ();
}



int32  Configuration::NumOfSettings (const KKStr&  sectionName)
{
  ConfSectionPtr  section = sections->LookUp (sectionName);

  if  (!section)
    return (-1);

  return  section->NumOfSettings ();
}




int32  Configuration::NumOfSettings (int32  sectionNum)  const
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



KKStrConstPtr  Configuration::SectionName (int32 sectionNum)  const
{
  ConfSectionPtr  section = sections->IdxToPtr (sectionNum);

  if  (!section)
    return NULL;
  else
    return  section->Name ();
}


int32  Configuration::SectionNum (const KKStr&  sectionName)  const
{
  if  (!sections)
    return -1;

  uint32  idx = 0;
  while  (idx < sections->size ())
  {
    if  (sections->IdxToPtr(idx)->Name ()->EqualIgnoreCase (sectionName))
      return  (int32)idx;
    idx++;
  }
  return -1;
}



int32  Configuration::SectionLineNum (int32 sectionNum)  const
{
  ConfSectionPtr  section = sections->IdxToPtr (sectionNum);

  if  (!section)
    return  -1;
  else
    return  section->LineNum ();
}



KKStrConstPtr  Configuration::SettingName (const KKStr&  sectionName,
                                           int32         settingNum
                                          )  const
{
  ConfSectionPtr  section = sections->LookUp (sectionName);
  if  (!section)
    return NULL;

  return  section->SettingName (settingNum);
}




KKStrConstPtr  Configuration::SettingName (int32  sectionNum,
                                           int32  settingNum
                                          )  const
{
  if  ((sectionNum < 0)  ||  (sectionNum >= sections->QueueSize ()))
    return NULL;

  if  ((settingNum < 0)  ||  (settingNum >= (*sections)[sectionNum].NumOfSettings ()))
    return NULL;


  return  sections->IdxToPtr (sectionNum)->SettingName (settingNum);
}



KKStrConstPtr  Configuration::SettingValue (int32         sectionNum,
                                            const KKStr&  settingName,
                                            int32&        lineNum
                                           )  const

{
  ConfSectionPtr  section = sections->IdxToPtr (sectionNum);
  if  (!section)
    return NULL;

  return section->LookUpValue (settingName, lineNum);
}




KKStrConstPtr  Configuration::SettingValue (int32   sectionNum,
                                            int32   settingNum,
                                            int32&  lineNum
                                           )  const
{
  if  ((sectionNum < 0)  ||  (sectionNum >= sections->QueueSize ()))
    return NULL;

  if  ((settingNum < 0)  ||  (settingNum >= (*sections)[sectionNum].NumOfSettings ()))
    return NULL;


  return  sections->IdxToPtr (sectionNum)->SettingValue (settingNum, lineNum);
}




KKStrConstPtr  Configuration::SettingValue (const KKU::KKStr&  sectionName,
                                            const KKU::KKStr&  settingName,
                                            int32&             lineNum
                                           )  const
{
  int32  sectionNum = SectionNum (sectionName);
  if  (sectionNum < 0)
    return NULL;

  return  SettingValue (sectionNum, settingName, lineNum);
}






void  Configuration::GetSetting (const char*     sectionName,
                                 int32           settingNum,
                                 KKStrConstPtr&  name,
                                 KKStrConstPtr&  value,
                                 int32&          lineNum
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




void  Configuration::FormatErrorsAdd (int32         lineNum,  
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
  for  (uint32 i = 0;  i < formatErrors.size ();  i++)
  {
    KKStr  lineNumStr = "    ";
    if  (i < formatErrorsLineNums.size ())
      lineNumStr = StrFormatInt (formatErrorsLineNums[i], "0000");
    errorMsgs.push_back (lineNumStr + ":" + formatErrors[i]); 
  }

  return  errorMsgs;
}  /* FormatErrorsWithLineNumbers */


