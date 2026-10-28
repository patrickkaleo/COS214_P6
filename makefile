CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g -Iinclude

SRCDIR = src
OBJDIR = obj
TARGET = tividy
TEST_TARGET = tividy_tests

# Separate main entry points
MAIN_SRC = $(SRCDIR)/main.cpp
TEST_SRC = $(SRCDIR)/main_tests.cpp

MAIN_OBJ = $(OBJDIR)/main.o
TEST_OBJ = $(OBJDIR)/main_tests.o

# Common engine sources compiled into both binaries (excluding both mains)
COMMON_SRCS = $(filter-out $(MAIN_SRC) $(TEST_SRC), $(wildcard $(SRCDIR)/*.cpp))
COMMON_OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(COMMON_SRCS))

.PHONY: all run test valgrind gdb docs clean

all: $(TARGET) $(TEST_TARGET)

# Main interactive TUI application
$(TARGET): $(MAIN_OBJ) $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Master unit test suite runner
$(TEST_TARGET): $(TEST_OBJ) $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
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
