# Define the compiler and the linker. The linker must be defined since
# the implicit rule for linking uses CC as the linker. g++ can be
# changed to clang++.
CXX = g++
CC  = $(CXX)

# Generate dependencies in *.d files
#DEPFLAGS = -MT $@ -MMD -MP -MF $*.d

# Define preprocessor, compiler, and linker flags. Uncomment the # lines
# if you use clang++ and wish to use libc++ instead of GNU's libstdc++.
# -g is for debugging.
CPPFLAGS =  -std=c++17 -I. -pthread -fgnu-tm
CXXFLAGS = -Wall -Wextra -pedantic-errors #-Wold-style-cast
CXXFLAGS +=  -o3
CXXFLAGS += -g
CXXFLAGS += $(DEPFLAGS)
LDFLAGS =   -g -pthread -fgnu-tm

#LDLIBS = -llab1
#CPPFLAGS += -stdlib=libc++
#CXXFLAGS += -stdlib=libc++
#LDFLAGS +=  -stdlib=libc++

# Targets
PROGS = test/SingleThreadTest test/MultipleThreadsTest test/benchMarkTest
all: $(PROGS)

# Targets rely on implicit rules for compiling and linking
test/SingleThreadTest: test/SingleThreadTest.o
test/MultipleThreadsTest: test/MultipleThreadsTest.o
test/benchMarkTest: test/benchMarkTest.o



# Phony targets
.PHONY: all clean distclean

# Standard clean
clean:
	rm -f test/*.o $(PROGS)
