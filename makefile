CXX = g++
CXXFLAGS = -std=c++11 -Wall -Iinclude
TARGET = Addmissions
SRC = $(wildcard src/*.cpp)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
	clear
	@echo "Cleaned.🌞"

run: $(TARGET)
	./$(TARGET)

valgrind: $(TARGET)
	clear
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

gdb: $(TARGET) 
	clear
	gdb ./$(TARGET)