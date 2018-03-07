#include "TestManager.h"

#include "MatrixTest.h"

using namespace KKTest;

int main (int argc, char** argv)
{
  TestManager  testManager;

  MatrixTest matrixTest (&testManager);

  matrixTest.RunTest ();

  return 0;
}