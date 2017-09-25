#if  !defined(_SCANNERHEADERFIELDS_)
#define  _SCANNERHEADERFIELDS_
#include <map>
#include "DateTime.h"
#include "GoalKeeper.h"
#include "KKStr.h"
#include "KKQueue.h"

namespace  KKLSC
{
  /**
   *@brief  Represents a list of header fields from a Scanner File.
   *@details  All ScannerFiles will start with a Header section that consists of text
   *          with each line consisting of a "FieldName" allowed by a "FieldValue" separated by the
   *          tab character.
   */
  class  ScannerHeaderFields: public std::map<KKStr,KKStr>
  {
  public:
    typedef  ScannerHeaderFields*  ScannerHeaderFieldsPtr;

    ScannerHeaderFields ();

    ScannerHeaderFields (const ScannerHeaderFields&  fields);

    ~ScannerHeaderFields ();

    kkMemSize  MemoryConsumedEstimated () const;

    void  Add (ScannerHeaderFieldsPtr  fields);

    void  Add (const KKB::KKStr&  fieldName,
               const KKB::KKStr&  fieldValue
              );

    void  Add (const KKB::KKStr&  fieldName,
               bool               fieldValue
              );

    void  Add (const KKStr&  fieldName,
               kkint32       fieldValue
              );

    void  Add (const KKStr&  fieldName,
               kkint64       fieldValue
              );

    void  Add (const KKStr&  fieldName,
               double        fieldValue
              );

    void  Add (const KKStr&   fieldName,
               KKB::DateTime  fieldValue
              );

    void  Clear ();  /**< Erases contents */

    bool  FieldExists (const KKStr&  fieldName)  const;

    const KKStr&  GetValue (const KKStr&  fieldName)  const;

    float  GetValueFloat (const KKStr&  fieldName)  const;

    kkint32  GetValueInt32 (const KKStr&  fieldName)  const;

    void  StartBlock ();
    void  EndBlock ();

    typedef  map<KKStr,KKStr>::iterator  iterator;

  private:
    GoalKeeperPtr           goalie;
    iterator                idx1;
    mutable const_iterator  idx2;
  };

  typedef  ScannerHeaderFields*  ScannerHeaderFieldsPtr;

}  /* KKLSC */

#endif
