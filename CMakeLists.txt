cmake_minimum_required(VERSION 3.8.0)
project(KSquareUtilities VERSION 0.1.0)

#enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
#set_property(TARGET KKBase PROPERTY CXX_STANDARD 17)
#target_compile_features(KKBase PUBLIC cxx_std_11)

include_directories(KKBase)
include_directories($ENV{OutsidePackages}/zlib-1.2.11)
message(STATUS,"\n\n  $ENV{OutsidePackages}")


subdirs(KKBase)
subdirs(KKLineScanner)
subdirs(KKMachineLearning)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
