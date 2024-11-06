# Compiler and linker
CXX = clang++
CXXFLAGS = -std=c++14 -Wall -g -I./include -I/usr/include/llvm-14 -I/usr/include/c++/11 -I /usr/include/x86_64-linux-gnu/c++/11
LDFLAGS = -nostdlib++ -L/usr/lib/llvm-14/lib -lLLVM -lpthread /usr/lib/x86_64-linux-gnu/libstdc++.so.6

# File names
SRCS = src/main.cpp src/Executor.cpp src/ExecutionState.cpp
OBJS = $(SRCS:.cpp=.o)
EXEC = toy

# Targets and rules
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $(EXEC) $(LDFLAGS)
	rm -f $(OBJS)

# Compile .cpp files to .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJS) $(EXEC)

.PHONY: all clean
