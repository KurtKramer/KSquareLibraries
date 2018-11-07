set(CMAKE_CXX_STANDARD 17)

file(GLOB source_files
	"*.cpp"
    )


add_library(KKBase
	${source_files}
	)

