#if  !defined(_NO_MEMORY_LEAK_CHECK_)

#if !defined(_WINDOWS)

WarningsLowered()
#if  defined(WIN32)  &&  defined(_DEBUG)
// We need to make sure that these items are included before we override the new operator.
#include <algorithm>
#include <assert.h>
#include <ctype.h>
#include <complex>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <stdlib.h>
#include <string>
#include <vector>
#include <windows.h>
WarningsRestored()

#define new MYDEBUG_NEW
#endif
#endif
#endif

#if defined(_WIN32)
#pragma  warning (disable : 4820)

#endif
