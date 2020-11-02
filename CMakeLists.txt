cmake_minimum_required(VERSION 3.10.2)
project(KSquareUtilities VERSION 0.2.0)

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

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic -Werror)


    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options(-Wmisleading-indentation # warn if
                                                     # identation
                                                     # implies blocks
                                                     # where blocks do
                                                     # not exist
                            -Wduplicated-cond # warn if if / else chain
                                              # has duplicated
                                              # conditions
                            -Wduplicated-branches # warn if if / else
                                                  # branches have
                                                  # duplicated code
                            -Wlogical-op # warn about logical
                                         # operations being used where
                                         # bitwise were probably wanted
                            -Wuseless-cast # warn if you perform a cast
                                           # to the same type
                           )
    else()
        if(LIBCPP)
            add_compile_options(-stdlib=libc++)
        else()
        endif()
    endif()
endif()


include_directories(KKBase)
include_directories($ENV{OutsidePackages}/zlib-1.2.11)
message(STATUS,"OutsidePackages: $ENV{OutsidePackages}")


subdirs(KKBase)
subdirs(KKLineScanner)
subdirs(KKMachineLearning)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
