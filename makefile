CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g -Iinclude

SRCDIR = src
OBJDIR = obj
TARGET = tividy

# Recursively find all .cpp files inside src/
SRCS = $(wildcard $(SRCDIR)/*.cpp) $(wildcard $(SRCDIR)/**/*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

.PHONY: all run clean valgrind gdb docs

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

valgrind: $(TARGET)
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./$(TARGET)

gdb: $(TARGET)
	gdb ./$(TARGET)

docs:
	doxygen Doxyfile
	@echo "Doxygen documentation generated in docs/html/ 📚"

clean:
	rm -rf $(OBJDIR) $(TARGET)