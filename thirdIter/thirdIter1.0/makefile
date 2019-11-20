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
CPPFLAGS =  -std=c++17 -I. -pthread
CXXFLAGS = -Wall -Wextra -pedantic-errors #-Wold-style-cast
CXXFLAGS +=  -o3
CXXFLAGS += -g
CXXFLAGS += $(DEPFLAGS)
LDFLAGS =   -g -pthread
#<<<<<<< HEAD:lab3/Makefile
#LDLIBS = -llab1
#>>>>>>> 66f8aecc57cd547e4d720bb76d2dc5643db909ab:Makefile
#CPPFLAGS += -stdlib=libc++
#CXXFLAGS += -stdlib=libc++
#LDFLAGS +=  -stdlib=libc++
HASH_PROGS = HashTable.o HashNode.o  Bucket.o
HASH_TEST_PROGS = HashTableTest.o HashNode.o  Bucket.o
# Targets
PROGS = SingleThreadTest MultipleThreadsTest
all: $(PROGS)

# Targets rely on implicit rules for compiling and linking
SingleThreadTest: SingleThreadTest.o $(HASH_TEST_PROGS)
MultipleThreadsTest: MultipleThreadsTest.o $(HASH_PROGS)


# Phony targets
.PHONY: all clean distclean

# Standard clean
clean:
	rm -f *.o $(PROGS)
