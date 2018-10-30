set(CMAKE_CXX_STANDARD 17)

add_library(KKBase 
    Application.cpp  
    Atom.cpp  
    BitString.cpp  
    Blob.cpp  
    BMPImage.cpp  
    Chart.cpp  
    CmdLineExpander.cpp
    Compressor.cpp
    Configuration.cpp
    ContourFollower.cpp
    ConvexHull.cpp
    DateTime.cpp
    EigenVector.cpp
    GlobalGoalKeeper.cpp
    GoalKeeper.cpp
    Histogram.cpp
    ImageDirTree.cpp
    ImageIO.cpp
    KKBaseTypes.cpp
    KKException.cpp
    KKObservable.cpp
    KKStr.cpp 
    KKStrMatrix.cpp 
    KKStrParser.cpp

)
