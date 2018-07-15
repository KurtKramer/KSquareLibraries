#include "../../KKStr.h"

#include "Matrix.h"

using namespace KKB;


void  TestMatrixMultiplication ()
{
  auto zed = KKB::osGetKernalTimeUsed ();
  //auto zed2 = KKB::osGetSystemTimeInMiliSecs ();

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
  std::cout << x[0][0] << "\t" << x[0][1] << "\t" << x[0][2] << std::endl;
  std::cout << x[1][0] << "\t" << x[1][1] << "\t" << x[1][2] << std::endl;
}


bool TestIndexing ()
{
  KKStr  s = "TestString.";

  bool validIdexesPasses = (s[0] == 'T')  &&  (s[4] == 'S')  &&  (s[9] == '.');
  bool negativeIndexes = (s[-1] == 0);
  bool beyoundEndOfStr = (s[11] == 0);

  kkint64  idx64 = 9999999999;
  bool zed = s[idx64];

  return  validIdexesPasses  &&  negativeIndexes  &&  beyoundEndOfStr;
}


int main (int argv, char* args[])
{
  TestMatrixMultiplication ();
  TestIndexing ();

}