#if !defined(_KKB_OPTION_)
#define  _KKB_OPTION_
#include <optional>

#include "KKBaseTypes.h"
#include "KKException.h"


namespace KKB 
{
  void  ValidateValidUint32 (kkint64 newValue);


  template<typename T>
  OptionUInt32  operator+ (const OptionUInt32& lhs, T rhs)
  {
    KKCheck(lhs, "OptionUInt32::operator+  Can not add to NONE!")
    kkint64 newValue = (kkint64)lhs.value() + (kkint64)rhs;
    ValidateValidUint32 (newValue);
    return OptionUInt32((kkuint32)newValue);
  }


  
  template<typename T>
  OptionUInt32 operator+ (T lhs, const OptionUInt32& rhs)
  {
    KKCheck(rhs, "OptionUInt32::operator+  Can not subtract from NONE!")
    kkint64 newValue = (kkint64)lhs + (kkint64)rhs.value ();
    ValidateValidUint32 (newValue);
    return OptionUInt32((kkuint32)newValue);
  }



  template<typename T>
  OptionUInt32 operator- (const OptionUInt32& lhs, T rhs)
  {
    KKCheck(lhs, "OptionUInt32::operator-  Can not subtract from NONE!")
    kkint64 newValue = (kkint64)lhs.value () - (kkint64)rhs;
    ValidateValidUint32 (newValue);
    return OptionUInt32((kkuint32)newValue);
  }



    template<typename T>
  OptionUInt32 operator- (T lhs, const OptionUInt32& rhs)
  {
    KKCheck(rhs, "OptionUInt32::operator-  Can not subtract from NONE!")
    kkint64 newValue = (kkint64)lhs - (kkint64)rhs.value ();
    ValidateValidUint32 (newValue);
    return OptionUInt32((kkuint32)newValue);
  }



  template<typename T>
  OptionUInt32& operator+= (OptionUInt32& lhs, T rhs)
  {
    KKCheck(lhs, "OptionUInt32::operator+=  Con not add to NONE!")
    kkint64 newValue = (kkint64)lhs.value () + (kkint64)rhs;
    ValidateValidUint32 (newValue);
    lhs = (kkuint32)newValue;
    return lhs;
  }



  template<typename T>
  OptionUInt32& operator-= (OptionUInt32& lhs, T rhs)
  {
    KKCheck(lhs, "OptionUInt32::operator+=  Con not add to NONE!")
    kkint64 newValue = (kkint64)lhs.value () - (kkint64)rhs;
    ValidateValidUint32 (newValue);
    lhs = (kkuint32)newValue;
    return lhs;
  }
  


  template<typename T>
  std::ostream&  operator<< (std::ostream& s, const std::optional<T>&  o)
  {
    if (o.has_value())
      s << o.value ();
    else
      s << "NONE";

    return s;
  }


  void  ValidateValidUint32 (kkint64 newValue);
}

#endif
