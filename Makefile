CXX = c++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17 $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs)

SRC_DIR = src
OBJ_DIR = obj
BIN = renderer

SRCS = $(shell find $(SRC_DIR) -name "*.cpp")
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $(OBJS) -o $(BIN) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN)

run: all
	./$(BIN)

.PHONY: all clean run
