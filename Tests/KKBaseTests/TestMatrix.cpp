#include "stdafx.h"
#include "CppUnitTest.h"

//#include "C:\code\KSquareLibraries\KKBase\Matrix.h"
#include "Matrix.h"

using namespace  KKB;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{		
	TEST_CLASS(TestMatrix)
	{
	public:
		
		TEST_METHOD(InitializeMatrix)
		{
			// TODO: Your test code here

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

      //std::cout << x << std::endl;
		}

	};
}