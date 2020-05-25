cmake_minimum_required(VERSION 3.10.2)
#cmake_minimum_required(VERSION 3.13)
project(KSquareUtilities VERSION 0.1.0)

#enable_testing()
#
#  To run on WSL ubuntu 18
#    sudo add-apt-repository ppa:ubuntu-toolchain-r/test
#    sudo apt-get update
#    sudo apt-get install gcc-8 g++-8
#    gcc-8 --version
#
#    befotre running cmake
#    export CXX="g++-8" CC="gcc-8"
#
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
#set_property(TARGET KKBase PROPERTY CXX_STANDARD 17)
#target_compile_features(KKBase PUBLIC cxx_std_11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -lstdc++fs")

include_directories(KKBase)
include_directories($ENV{OutsidePackages}/zlib-1.2.11)
message(STATUS,"\n\n  $ENV{OutsidePackages}")


subdirs(KKBase)
subdirs(KKLineScanner)
subdirs(KKMachineLearning)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
