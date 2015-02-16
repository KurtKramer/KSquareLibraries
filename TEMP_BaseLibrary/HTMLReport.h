#ifndef  _HTMLREPORT_
#define  _HTMLREPORT_

#include  "DateTime.h"
#include  "Str.h"


#ifndef  WIN32
#define  __cdecl  
#endif



namespace KKU 
{
  class  HTMLReport
  {
  public:
    typedef  HTMLReport*  HTMLReportPtr;
    typedef  enum  {Left, Center, Right}  AlignmentType;

    HTMLReport (KKStr          _fileName,
                KKStr          _title,
                AlignmentType  _alignment
               );

    ~HTMLReport ();


    void  Append (const char*  str);
    void  Close ();

    ostream&  OStream ()  {return  r;}


    friend  HTMLReport&  operator<< (HTMLReport&  htmlReport,  int32           right);
    friend  HTMLReport&  operator<< (HTMLReport&  htmlReport,  uint32          right);
    friend  HTMLReport&  operator<< (HTMLReport&  htmlReport,  int64           right);
    friend  HTMLReport&  operator<< (HTMLReport&  htmlReport,  uint64          right);
    friend  HTMLReport&  operator<< (HTMLReport&  htmlReport,  double          right);

    friend  HTMLReport&  operator<< (HTMLReport&  htmlReport,  char            right);
    friend  HTMLReport&  operator<< (HTMLReport&  htmlReport,  const char*     right);
    friend  HTMLReport&  operator<< (HTMLReport&  htmlReport,  const KKStr&    right);
    friend  HTMLReport&  operator<< (HTMLReport&  htmlReport,  KKStrConstPtr   right);

    friend  HTMLReport&  operator<< (HTMLReport&  htmlReport,  const DateTime&  dateTime);

    friend  HTMLReport&  endl (HTMLReport& _outs);


  private:
    KKStr  CurStyleStr ();

    AlignmentType  curAlignment;

    KKStr          fileName;
    bool           opened;
    ofstream       r;
    KKStr          title;
  };

  typedef  HTMLReport::HTMLReportPtr  HTMLReportPtr;


  #ifdef  WIN32
  HTMLReport& __cdecl operator<< (HTMLReport&  htmlReport,
                                  HTMLReport& (__cdecl*)(HTMLReport &)
                                 );

  HTMLReport& __cdecl endl (HTMLReport&  htmlReport);
  #else

  HTMLReport&  operator<< (HTMLReport &  htmlReport,
                           HTMLReport & (*)(HTMLReport &)
                          );


  HTMLReport&  endl (HTMLReport &  htmlReport);

  #endif


  HTMLReport&  operator<< (HTMLReport&  htmlReport,
                           int32        right
                          );

  HTMLReport&  operator<< (HTMLReport&     htmlReport,
                       uint32      right);

  HTMLReport&  operator<< (HTMLReport&  htmlReport,
                           long         right
                          );

  HTMLReport&  operator<< (HTMLReport&  htmlReport,
                           long long    right
                          );

  HTMLReport&  operator<< (HTMLReport&  htmlReport,
                           double       right
                          );

  HTMLReport&  operator<< (HTMLReport&    htmlReport,
                           unsigned long  right
                          );

  HTMLReport&  operator<< (HTMLReport&  htmlReport,
                           char         right
                          );

  HTMLReport&  operator<< (HTMLReport&   htmlReport,
                           const char*   right
                          );

  HTMLReport&  operator<< (HTMLReport&   htmlReport,
                           const KKStr&  right
                          );

  HTMLReport&  operator<< (HTMLReport&    htmlReport,
                           KKStrConstPtr  right
                          );

  HTMLReport&  operator<< (HTMLReport&      htmlReport,
                           const DateTime&  right
                          );

}  /* namespace KKU */

#endif
