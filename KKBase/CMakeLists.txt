set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB source_files
	"*.cpp"
    )


add_library(KKBase
	${source_files}
	)

install (TARGETS KKBase DESTINATION bin)
