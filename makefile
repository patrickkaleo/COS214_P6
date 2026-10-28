CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g -Iinclude

SRCDIR = src
TESTDIR = tests
OBJDIR = obj
TARGET = tividy
TEST_TARGET = tividy_tests

# Demo application sources
MAIN_SRC = $(SRCDIR)/main.cpp
MAIN_OBJ = $(OBJDIR)/main.o

# Common engine sources compiled into both the demo and tests (all src/*.cpp except main.cpp)
COMMON_SRCS = $(filter-out $(MAIN_SRC), $(wildcard $(SRCDIR)/*.cpp))
COMMON_OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(COMMON_SRCS))

# Test suite sources (all tests/*.cpp)
TEST_SRCS = $(wildcard $(TESTDIR)/*.cpp)
TEST_OBJS = $(patsubst $(TESTDIR)/%.cpp, $(OBJDIR)/$(TESTDIR)/%.o, $(TEST_SRCS))

.PHONY: all run test valgrind gdb docs clean

all: $(TARGET) $(TEST_TARGET)

# Main interactive TUI application
$(TARGET): $(MAIN_OBJ) $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Master modular unit test suite runner
$(TEST_TARGET): $(TEST_OBJS) $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Pattern rule for engine and demo objects
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Pattern rule for modular test objects
$(OBJDIR)/$(TESTDIR)/%.o: $(TESTDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

valgrind: $(TEST_TARGET)
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./$(TEST_TARGET)

gdb: $(TARGET)
	gdb ./$(TARGET)

docs:
	doxygen Doxyfile
	@echo "Doxygen documentation generated in docs/html/ 📚"

clean:
	rm -rf $(OBJDIR) $(TARGET) $(TEST_TARGET)
