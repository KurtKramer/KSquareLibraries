set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB source_files
	"*.cpp"
    )




add_library(KKBase
	${source_files}
	)

add_compile_options(-Wall -Wextra -pedantic -Werror)

target_compile_features(KKBase PRIVATE cxx_lambda_init_captures cxx_variadic_templates)

install (TARGETS KKBase DESTINATION bin)



