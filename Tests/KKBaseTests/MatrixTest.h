#if !defined(_MATRIXTEST_)
#define _MATRIXTEST_

#include "BaseTest.h"

#include "Matrix.h"

namespace KKTest
{
  class MatrixTest: public BaseTest
  {
  public:
    MatrixTest (TestManagerPtr _testManager);
    virtual ~MatrixTest ();

    virtual void RunTest ();

  protected:
    virtual KKB::KKStr  ClassName () const { return "MatrixTest"; }


  private:
    void TestMultiplication ();

  };
}

#endif
