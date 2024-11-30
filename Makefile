# Directories
INCDIR = include
SRCDIR = src
TESTDIR = tests
OBJDIR = obj

# Compiler & flags
CXX = g++
CXXFLAGS = -g -Wall -Werror -Wextra -O3 -std=c++20 -m64 -I$(INCDIR)

# Source files
SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

# Include bst explicitly
BASE = $(OBJDIR)/bst.o
OBJS := $(filter-out $(OBJDIR)/bst.o, $(OBJS)) # Exclude bst.o from the default pattern
OBJS += $(BASE)

# Targets for test combinations
.PHONY: all clean multi_threaded single_threaded

# Default target
all: clean multi_threaded single_threaded

# Multi-threaded test targets
mtcg: $(OBJS) $(TESTDIR)/multi_threaded/coarse_grained
mtfg: $(OBJS) $(TESTDIR)/multi_threaded/fine_grained
mtlf: $(OBJS) $(TESTDIR)/multi_threaded/lock-free

# Single-threaded test targets
stcg: $(OBJS) $(TESTDIR)/single_threaded/coarse_grained
stfg: $(OBJS) $(TESTDIR)/single_threaded/fine_grained
stlf: $(OBJS) $(TESTDIR)/single_threaded/lock-free

# Pattern rules for tests
$(TESTDIR)/multi_threaded/%: $(TESTDIR)/multi_threaded/%.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $< -o $@ $(LDFLAGS)
	@echo "Running $@..."
	./$@

$(TESTDIR)/single_threaded/%: $(TESTDIR)/single_threaded/%.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $< -o $@ $(LDFLAGS)
	@echo "Running $@..."
	./$@

# Compile source files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(wildcard $(INCDIR)/*.h)
	@mkdir -p $(OBJDIR)
	$(CXX) -c $< -o $@ $(CXXFLAGS)

# Clean up
clean:
	rm -rf $(OBJDIR) 
	rm -f $(TESTDIR)/multi_threaded/coarse_grained 
	rm -f $(TESTDIR)/multi_threaded/fine_grained 
	rm -f $(TESTDIR)/multi_threaded/lock-free
	rm -f $(TESTDIR)/single_threaded/coarse_grained 
	rm -f $(TESTDIR)/single_threaded/fine_grained 
	rm -f $(TESTDIR)/single_threaded/lock-free