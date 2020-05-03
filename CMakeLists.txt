cmake_minimum_required(VERSION 3.0.0)
project(KSquareUtilities VERSION 0.1.0)

enable_testing()

include_directories(KKBase)
include_directories($ENVtsidePackages}/zlib128)


subdirs(KKBase)
subdirs(KKLineScanner)
subdirs(KKMachineLearning)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
