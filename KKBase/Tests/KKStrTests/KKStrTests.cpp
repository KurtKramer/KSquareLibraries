#include "../../KKStr.h"

using namespace KKB;

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
  TestIndexing ();

}