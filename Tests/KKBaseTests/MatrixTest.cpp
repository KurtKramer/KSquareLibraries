#include "MatrixTest.h"
using namespace KKB;

namespace KKTest
{

  MatrixTest::MatrixTest (TestManagerPtr _testManager):
    BaseTest(_testManager)
  {
  }



  MatrixTest::~MatrixTest ()
  {
  }



  void MatrixTest::RunTest ()
  {
    TestMultiplication ();

  }



  void MatrixTest::TestMultiplication ()
  {
    MatrixD a (3, 2);
    MatrixD b (2, 3);

    a[0][0] = 1.0;
    a[0][1] = 2.0;
    a[1][0] = 3.0;
    a[1][1] = 4.0;
    a[2][0] = 5.0;
    a[2][1] = 6.0;

    b[0][0] = 1.0;
    b[0][1] = 2.0;
    b[0][2] = 3.0;

    b[1][0] = 4.0;
    b[1][1] = 5.0;
    b[1][2] = 6.0;

    auto x = a * b;

    double expected[3][3] = { {9.0, 21.0, 16.0}, {19.0, 26.0, 33.0}, {29.0, 40.0, 51.0} };

    ReportResult ((x[0][0] == expected[0][0]) && (x[0][1] == expected[0][1]) && (x[0][2] == expected[0][2]), "Multiplication", "");
  }

}
