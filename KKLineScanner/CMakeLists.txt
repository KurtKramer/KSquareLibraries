set(CMAKE_CXX_STANDARD 17)


file(GLOB kkbase_headers
	"../KKBase/*.h"
    )

include_directories(../KKBase)


add_library(KKLineScanner 
  ${kkbase_headers}
  FlatFieldCorrection.cpp
  FlowMeterTracker.cpp
  ScannerClock.cpp
  ScannerFile2BitEncoded.cpp
  ScannerFile3BitEncoded.cpp
  ScannerFile4BitEncoded.cpp
  ScannerFile.cpp
  ScannerFileEntry.cpp
  ScannerFileSimple.cpp
  ScannerFileSipper3.cpp
  ScannerFileZLib3BitEncoded.cpp
  ScannerFrame.cpp
  ScannerHeaderFields.cpp
  StartStopPoint.cpp
  Variables.cpp
)

