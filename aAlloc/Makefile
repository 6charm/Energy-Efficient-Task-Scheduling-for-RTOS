CXX := g++
CXXFLAGS := -std=c++11 -Wall -Wextra

# Get list of test source files
TEST_SRCS := $(wildcard tests/test*.cc)

# Default target to build all test executables
all: $(TEST_SRCS:%.cc=%)

# Target to link each test object file and aAlloc.o
$(TEST_SRCS:%.cc=%): %: %.cc aAlloc.o
	$(CXX) $(CXXFLAGS) $^ -o $@ -lm

# Target to compile aAlloc.cc into aAlloc.o
aAlloc.o: aAlloc.cc aAlloc.hh
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target to remove object files and executables
clean:
	rm -f tests/*.o tests/test?? *.o
