#if !defined(_KKB_OPTION_)
#define  _KKB_OPTION_
#include <optional>
#include <type_traits>

#include "KKBaseTypes.h"
#include "KKException.h"

#undef max
#undef min


namespace KKB 
{
  void  ValidateValidUint32 (kkint64 newValue);


  template<typename T>
  class  Option: std::optional<T>
  {
  public:

    template<typename U>
    Option<T>& operator=(const U& rhs)
    {
      if  (std::is_unsigned<T>::value)
      {
        if (std::is_unsigned<U>::value)
        {
          KKCheck (rhs < std::numeric_limits<T>::max (), "Option<T> operator= rhs exceeds capacity of lhs!")
        }
        else
        {
          KKCheck (rhs >= 0 && static_cast<T> (rhs) < std::numeric_limits<T>::max (),  "Option<T> operator= rhs exceeds capacity of lhs!")
        }
      }
      else
      {
        if (std::is_unsigned<U>::value)
        {
          KKCheck (rhs < static_cast<U> (std::numeric_limits<T>::max ()), "Option<T> operator= rhs exceeds capacity of lhs!")
        }
        else
        {
          //KKCheck (rhs >= 0 && (unsigned U)rhs < std::numeric_limits<T>::max (), "Option<T> operator= rhs exceeds capacity of lhs!")
        }
      }
      optional<T>::operator= (rhs);
      return *this;
    }



    template<typename U>
    Option<T> operator+(U rhs)
    {
      KKCheck (this->has_value (), "Option<T> operator+   lhs is None!");
      return Option<T> (this->value() + rhs);
    }

    

    template<typename U>
    Option<T> operator-(U rhs)
    {
      KKCheck (this->has_value (), "Option<T> operator+   lhs is None!");
      return Option<T> (this->value () - rhs);
    }
  };


  
  template<typename T>
  OptionUInt32  operator+ (const OptionUInt32& lhs, T rhs)
  {
    KKCheck (lhs, "OptionUInt32::operator+  Can not add to NONE!")
    const kkint64 newValue = static_cast<kkint64> (lhs.value()) + static_cast<kkint64> (rhs);
    ValidateValidUint32 (newValue);
    return OptionUInt32(static_cast<kkuint32> (newValue));
  }


  
  template<typename T>
  OptionUInt32 operator+ (T lhs, const OptionUInt32& rhs)
  {
    KKCheck(rhs, "OptionUInt32::operator+  Can not subtract from NONE!")
    kkint64 newValue = static_cast<kkint64> (lhs) + static_cast<kkint64> (rhs.value ());
    ValidateValidUint32 (newValue);
    return OptionUInt32(static_cast<kkuint32> (newValue));
  }



  template<typename T>
  OptionUInt32 operator- (const OptionUInt32& lhs, T rhs)
  {
    KKCheck(lhs, "OptionUInt32::operator-  Can not subtract from NONE!")
    kkint64 newValue = static_cast<kkint64> (lhs.value ()) - static_cast<kkint64> (rhs);
    ValidateValidUint32 (newValue);
    return OptionUInt32(static_cast<kkuint32> (newValue));
  }



  template<typename T>
  OptionUInt32 operator- (T lhs, const OptionUInt32& rhs)
  {
    KKCheck(rhs, "OptionUInt32::operator-  Can not subtract from NONE!")
    kkint64 newValue = static_cast<kkint64> (lhs) - static_cast<kkint64> (rhs.value ());
    ValidateValidUint32 (newValue);
    return OptionUInt32(static_cast<kkuint32> (newValue));
  }



  template<typename T>
  OptionUInt32& operator+= (OptionUInt32& lhs, T rhs)
  {
    KKCheck(lhs, "OptionUInt32::operator+=  Con not add to NONE!")
    kkint64 newValue = static_cast<kkint64> (lhs.value ()) + static_cast<kkint64> (rhs);
    ValidateValidUint32 (newValue);
    lhs = static_cast<kkuint32> (newValue);
    return lhs;
  }



  template<typename T>
  OptionUInt32& operator-= (OptionUInt32& lhs, T rhs)
  {
    KKCheck(lhs, "OptionUInt32::operator+=  Con not add to NONE!")
    kkint64 newValue = static_cast<kkint64> (lhs.value ()) - static_cast<kkint64> (rhs);
    ValidateValidUint32 (newValue);
    lhs = static_cast<kkuint32> (newValue);
    return lhs;
  }
  


  template<typename T>
  std::ostream&  operator<< (std::ostream& s, const std::optional<T>&  o)
  {
    if (o.has_value())
      s << o.value ();
    else
      s << "*NO_Value*";

    return s;
  }


  void  ValidateValidUint32 (kkint64 newValue);
}

#endif
