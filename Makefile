CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude

SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:.cpp=.o)
EXEC = L1simulate

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f src/*.o $(EXEC)
