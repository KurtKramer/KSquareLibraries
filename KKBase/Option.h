#if !defined(_KKB_OPTION_)
#define  _KKB_OPTION_
#include <optional>

#include "KKBaseTypes.h"
#include "KKException.h"


namespace KKB 
{
  template<typename T>
  OptionUInt32 operator+ (const OptionUInt32& lhs, T rhs)
  {
    KKCheck(lhs.valid (), "OptionUInt32::operator+  Can not add to NONE!")
    kkint64 newValue = (kkint64)lhs.value() + (kkint64)rhs;
    ValidateValidUint32 (newValue);
    return OptionUInt32(newValue);
  }



  template<typename T>
  OptionUInt32 operator- (const OptionUInt32& lhs, T rhs)
  {
    KKCheck(lhs.valid (), "OptionUInt32::operator+  Can not add to NONE!")
    kkint64 newValue = (kkint64)lhs.value() - (kkint64)rhs;
    ValidateValidUint32 (newValue);
    return OptionUInt32(newValue);
  }



  template<typename T>
  void  OptionUInt32& operator+= (OptionUInt32& lhs, T rhs)
  {
    KKCheck(lhs.valid (), "OptionUInt32::operator+=  Con not add to NONE!")
    kkint64 newValue = (kkint64)lhs.value() + (kkint64)rhs;
    ValidateValidUint32 (newValue);
    lhs = (kkuint32)newValue
    return lhs;
  }



  template<typename T>
  void  OptionUInt32& operator-= (OptionUInt32& lhs, T rhs)
  {
    KKCheck(lhs.valid (), "OptionUInt32::operator+=  Con not add to NONE!")
    kkint64 newValue = (kkint64)lhs.value() - (kkint64)rhs;
    ValidateValidUint32 (newValue);
    lhs = (kkuint32)newValue
    return lhs;
  }



  void  ValidateValidUint32 (kkint64 newValue)
  {
    KKCheck(newValue >= 0, "OptionUInt32  result: " << newValue << " is nagative!")
    KKCheck(newValue <= uint32_max,"OptionUInt32  result: " << newValue << "  exceeds capacity of uint32!")
  }
}




#endif
