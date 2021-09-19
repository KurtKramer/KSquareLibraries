#pragma once

#include <algorithm>
#include <functional>
#include <vector>

namespace  KKB
{
  template<typename T>
  class KKHeap
  {
  public:

    typedef  const std::function<bool(const T& left, const T& right)> CompFunc;


    KKHeap(CompFunc _compFunc): compFunc (_compFunc)
    {
    }
    
    void Push(T n)
    {
      contents.push_back(n);
      std::push_heap (contents.begin (), contents.end (), compFunc);
    }

    T Pop()
    {
      std::pop_heap (contents.begin (), contents.end (), compFunc);
      T result = contents.back();
      contents.pop_back();
      return result;
    }

  private:
    CompFunc        compFunc;
    std::vector<T>  contents;
  };
}
