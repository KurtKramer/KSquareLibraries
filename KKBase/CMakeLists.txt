set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB source_files
	"*.cpp"
    )




add_library(KKBase
	${source_files}
	)

target_include_directories(KKBase PUBLIC $ENV{OutsidePackages}/zlib-1.2.11)

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    target_compile_features(KKBase PRIVATE cxx_lambda_init_captures cxx_variadic_templates)

    target_compile_options(KKBase PRIVATE 
                        -Wall
                        -Wextra # reasonable and standard
                        -Wshadow # warn the user if a variable`
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
endif ()

install (TARGETS KKBase DESTINATION bin)
