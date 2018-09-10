#if  !defined(_KKOPTION_)
#define _KKOPTION_

#include "KKBaseTypes.h"

namespace  KKB
{
  class KKNone {};

  /**
   *@brief Enspired by the Option type in Scala; idea is to use as a way
   * of returning a value from a method; or a flag indicating no return.
   */
  template<typename T>
  class KKOption
  {
  public:
    KKOption (): none (true), value ()
    {
    }


    KKOption (const T& _value) : none (false), value (_value)  
    {
    }

    
    KKOption (const KKNone& _none) : none (true), value ()  
    {
    }


    bool None () const { return none; }

    bool Exists () const { return !none; }
   
    T    Value ()       { return value; }

    T    OrElse (const T& defaultValueIfNode) { return none ? defaultValueIfNode : value; }  

    T& operator= (T& _value)
    {
      none = false;
      value = _value;
      return *this;
    }

    KKOption<T>& operator= (const KKOption<T>& right)
    {
      this->none  = right.none;
      this->value = right.value;
      return *this;
    }

    KKOption<T>& operator= (bool right)
    {
      this->none  = right;
      return *this;
    }

    KKOption<T>& operator= (KKNone& kkNone)
    {
      none = true;
    }

    bool operator== (const T& right)
    {
      if  (none)
        return false;
      else
        return value == right;
    }

    bool operator== (const KKNone& n)  { return none; }

    bool none;
    T    value;
  };

  extern  KKNone  kkNone;
  
  typedef KKOption<kkint32>  OptionInt32;

  typedef KKOption<kkuint32> OptionUInt32;

  template<typename T>
  KKOption<T>  Max(const KKOption<T>& a, const KKOption<T>& b)
  {
    if  (a.None ())
      return b;

    else if (b.None ())
      return a;

    return Max (a.value, b.value);
  }



  template<typename T>
  bool operator> (const KKOption<T>& a, const KKOption<T>& b)
  {
    if  (a.None ())
      return false;

    else if  (b.None ())
      return true;

    else
      return  a.value > b.value;
  }


  template<typename T>
  bool operator< (const KKOption<T>& a, const KKOption<T>& b)
  {
    if  (b.None ())
      return false;

    else if  (a.None ())
      return true;

    else
      return  a.value < b.value;
  }


}

#endif