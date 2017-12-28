#if  !defined(_SCANNERFILEENTRY_)
#define  _SCANNERFILEENTRY_

#include "DateTime.h"
#include "GoalKeeper.h"
#include "KKStr.h"
#include "KKStrParser.h"
#include "RunLog.h"
using namespace KKB;


namespace  KKLSC
{
  #if  !defined(_SCANNERFILE_)
    class  ScannerFile;
    typedef  ScannerFile*  ScannerFilePtr;
  #endif
  
  class  ScannerFileEntryList;
  typedef  ScannerFileEntryList*  ScannerFileEntryListPtr;


  /**
   *@brief Class that keeps track of parameter details of a single scanner file.
   */
  class  ScannerFileEntry
  {
  public:
    typedef  ScannerFileEntry*  ScannerFileEntryPtr;

    ScannerFileEntry (const KKStr&  _rootName);

    ScannerFileEntry (const ScannerFileEntry&  entry);
  
    ~ScannerFileEntry ();

    kkMemSize  MemoryConsumedEstimated () const;
  
    // Access Methods
    const KKStr&      Description       () const  {return  description;}
    const KKStr&      FullName          () const  {return  fullName;}
    kkuint32          PixelsPerScanLine () const  {return  pixelsPerScanLine;}
    const KKStr&      RootName          () const  {return  rootName;}
    float             ScanRate          () const  {return  scanRate;}
  
    void  Description        (const KKStr&  _description)       {description       = _description;}
    void  FullName           (const KKStr&  _fullName)          {fullName          = _fullName;}
    void  PixelsPerScanLine  (kkuint32      _pixelsPerScanLine) {pixelsPerScanLine = _pixelsPerScanLine;}
    void  RootName           (const KKStr&  _rootName)          {rootName          = _rootName;}
    void  ScanRate           (float         _scanRate)          {scanRate          = _scanRate;}
  
    KKStr   ToTabDelStr ()  const;
    void    ParseTabDelStr (KKStr  parser);

    void    Assign (const ScannerFileEntry&  sf);
    
    static  void  CleanUp ();

    static  ScannerFileEntryPtr       GetOrCreateScannerFileEntry (const KKStr&  rootName);

    static  ScannerFileEntryPtr       GetOrCreateScannerFileEntry (KKLSC::ScannerFilePtr  scannerFile);

  private:
     static  ScannerFileEntryListPtr  globalList;

     KKStr     description;
     KKStr     fullName;
     kkuint32  pixelsPerScanLine;
     KKStr     rootName;
     float     scanRate;           //  Camera Scan Rate  Lines/Sec
  };  /* ScannerFileEntry */


  typedef  ScannerFileEntry::ScannerFileEntryPtr  ScannerFileEntryPtr;



  class  ScannerFileEntryList: public map<KKStr, ScannerFileEntryPtr>
  {
  public:
    typedef  ScannerFileEntryList*  ScannerFileEntryListPtr;

    typedef  map<KKStr, ScannerFileEntryPtr>::iterator  Iterator;

    typedef  pair<KKStr, ScannerFileEntryPtr>  KeyPair;
  
    ScannerFileEntryList ();
    ~ScannerFileEntryList ();

    kkMemSize  MemoryConsumedEstimated () const;


    void  AddEntry (ScannerFileEntryPtr  enytry);

    ScannerFileEntryPtr  LookUpByRootName (const KKStr&  rootName);

  private:
  };  /* ScannerFileEntryList */


  typedef  ScannerFileEntryList::ScannerFileEntryListPtr  ScannerFileEntryListPtr;

}  /* KKLSC */

#endif
