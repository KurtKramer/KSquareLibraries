CXX = g++


CXX_FLAGS = -Wfatal-errors -Wall -Wextra -Wpedantic -Wconversion -Wshadow -std=c++11 


#CXX_FLAGS = -Wfatal-errors -Wall -Wextra -Wpedantic -Wconversion -Wshadow -std=gnu++11
#CXX_FLAGS = -Wfatal-errors -Wall -Wextra -Wpedantic -Wconversion -Wshadow -std=gnu++11



# Final binary
BIN = libKKBase.a
# Put all auto generated stuff to this build dir.
BUILD_DIR = ./Debug

FFTWAPI = ../OutsidePackages/fftw-3.3.2/api
INCLUDES = ../../OutsidePackages/zlib128/
LIBS = -lfftw -lrt

# List of all .cpp source files.
CPP = $(wildcard *.cpp)

# All .o files go to build dir.
OBJ = $(CPP:%.cpp=$(BUILD_DIR)/%.o)
# Gcc/Clang will create these .d files containing dependencies.
DEP = $(OBJ:%.o=%.d)

all:	$(BUILD_DIR)/$(BIN)

# Default target named after the binary.
$(BIN) : $(BUILD_DIR)/$(BIN)

# Actual target of the binary - depends on all .o files.
$(BUILD_DIR)/$(BIN) : $(OBJ)
	# Create build directories - same structure as sources.
	mkdir -p $(@D)
	# Just link all the object files.
	#$(CXX) $(CXX_FLAGS) $^ -o $@
	rm $(BUILD_DIR)/$(BIN) ; ar cq $(BUILD_DIR)/$(BIN)  $(BUILD_DIR)/*.o

# Include all .d files
-include $(DEP)

# Build target for every single object file.
# The potential dependency on header files is covered
# by calling `-include $(DEP)`.
$(BUILD_DIR)/%.o : %.cpp
	mkdir -p $(@D)
	# The -MMD flags additionaly creates a .d file with
	# the same name as the .o file.
	$(CXX) $(CXX_FLAGS) -I $(INCLUDES) -MMD -c $< -o $@

.PHONY : clean
clean :
	# This should remove all generated files.
	-rm $(BUILD_DIR)/$(BIN) $(OBJ) $(DEP)

