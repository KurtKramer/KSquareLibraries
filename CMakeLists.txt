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

    add_compile_options(-Wall
                        -Werror # treat all warnings as errors
                        -Wextra # reasonable and standard
                        -Wshadow # warn the user if a variable
                                 # declaration shadows one from a
                                 # parent context
                        -Wnon-virtual-dtor # warn the user if a class
                                           # with virtual functions
                                           # has a non-virtual
                                           # destructor. This helps
                                           # catch hard to track down
                                           # memory errors
                        -Wold-style-cast # warn for c-style casts
                        -Wcast-align # warn for potential performance
                                     # problem casts
                        -Wunused # warn on anything being unused
                        -Woverloaded-virtual # warn if you overload
                                             # (not override) a
                                             # virtual function
                        -Wpedantic # warn if non-standard C++ is used
                        -Wconversion # warn on type conversions that
                                     # may lose data
                        -Wsign-conversion # warn on sign conversions
                        -Wnull-dereference # warn if a null
                                           # dereference is detected
                        -Wdouble-promotion # warn if float is
                                           # implicit promoted to
                                           # double
                        -Wformat=2 # warn on security issues around
                                   # functions that format output (ie
                                   # printf)
                       )

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
