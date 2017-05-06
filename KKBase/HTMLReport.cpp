#include "FirstIncludes.h"

#include <stdlib.h>
#include <cstdio>
#include <fcntl.h>

#include <string>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <fstream>
#include <vector>

#include "MemoryDebug.h"
#include "KKBaseTypes.h"


#include <sys/types.h>
#ifdef  WIN32
#include <io.h>
#include <windows.h>
#else
//#include  <sys/loadavg.h>
#include <unistd.h>
#endif


using namespace std;
using namespace KKB;

#include "HTMLReport.h"



HTMLReport::HTMLReport (KKStr          _fileName,
                        KKStr          _title,
                        AlignmentType  _alignment
                       ):
  curAlignment (_alignment),
  fileName     (_fileName),
  opened       (false),
  r            (),
  title        (_title)
{
  r.open (fileName.Str (), std::ios_base::out);
  opened = true;

  r << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">" << std::endl
    << "<html  xmlns=\"http://www.w3.org/1999/xhtml\">"                   << std::endl
    << "<head>"                                                           << std::endl
    << "<title>" << title << "</title>"                                   << std::endl
    << "</head>"                                                          << std::endl
    << "<body " << CurStyleStr () << ">"                                  << std::endl;
}




KKStr  HTMLReport::CurStyleStr ()
{
  KKStr  styleStr = "stype=\"";

  styleStr << "text-align:";

  switch  (curAlignment)
  {
  case  AlignmentType::Left:   styleStr << "left";    break;
  case  AlignmentType::Center: styleStr << "center";  break;
  case  AlignmentType::Right:  styleStr << "right";   break;
  }

  styleStr << "\"";

  return   styleStr; 
}  /* CurStyleStr */




HTMLReport::~HTMLReport (void)
{
  if  (opened)
    Close ();
}


  
void  HTMLReport::Append (const char*  str)
{
  r << str;
}



void  HTMLReport::Close ()
{
  if  (opened)
  {
    r.close ();
    opened = false;
  }
}



HTMLReport&  KKB::operator<< (HTMLReport&  htmlReport,
                              kkint32      right
                             )
{
  KKStr  s (30);
  s = StrFormatInt (right, "0");
  htmlReport.Append (s.Str ());
  return  htmlReport;
}



HTMLReport&  KKB::operator<< (HTMLReport&  htmlReport,
                              kkuint32     right
                             )
{
  KKStr  s (30);
  s = StrFormatInt (right, "0");
  htmlReport.Append (s.Str ());
  return  htmlReport;
}



HTMLReport&  KKB::operator<< (HTMLReport&  htmlReport,
                              kkint64      right
                             )
{
  KKStr  s (30);
  s = StrFormatInt64 (right, "0");
  htmlReport.Append (s.Str ());
  return  htmlReport;
}



HTMLReport&  KKB::operator<< (HTMLReport&  htmlReport,
                              kkuint64     right
                             )
{
  KKStr  s (30);
  s = StrFormatInt64 (right, "0");
  htmlReport.Append (s.Str ());
  return  htmlReport;
}



HTMLReport&  KKB::operator<< (HTMLReport&  htmlReport,
                              double       right
                             )
{
  char  buff[50];

# ifdef  USE_SECURE_FUNCS
    sprintf_s (buff, sizeof (buff), "%f", right);
# else
    std::sprintf (buff, "%f", right);
# endif

  htmlReport.Append (buff);
  return  htmlReport;
}



HTMLReport&  KKB::operator<< (HTMLReport&  htmlReport,
                              char         right
                             )
{
  char  buff[20];
  buff[0] = right;
  buff[1] = 0;
  htmlReport.Append (buff);
  return  htmlReport;
}



HTMLReport&  KKB::operator<< (HTMLReport&  htmlReport,
                              const char*  right
                             )
{
  htmlReport.Append (right);
  return  htmlReport;
}




HTMLReport&  KKB::operator<< (HTMLReport&   htmlReport,
                              const KKStr&  right
                             )
{
  htmlReport.Append (right.Str ());
  return  htmlReport;
}




HTMLReport&  KKB::operator<< (HTMLReport&    htmlReport,
                              KKStrConstPtr  right
                             )
{
  if  (right)
    htmlReport.Append (right->Str ());
  return  htmlReport;
}



HTMLReport&  KKB::operator<< (HTMLReport&      htmlReport,  
                              const DateTime&  right
                             )
{
  KKStr s = right.Date ().YYYY_MM_DD () + "-" + right.Time ().HH_MM_SS ();
  htmlReport.Append (s.Str ());
  return  htmlReport;
}
 



HTMLReport& __cdecl KKB::operator<< (KKB::HTMLReport&   htmlReport,
                                     KKB::HTMLReport&   (__cdecl* mf)(KKB::HTMLReport &)
                                    )
{
  mf (htmlReport);
  return  htmlReport;
}



HTMLReport& __cdecl  KKB::endl (HTMLReport &  htmlReport)
{
  htmlReport.Append ("\n");
  return htmlReport;
}
