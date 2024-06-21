CXX = g++
CXXFLAGS = -Wall -g -std=c++20

INCLUDES = -Ideps/includes/SDL2 -Ideps/includes/SDL2_image -Ideps/includes/SDL2_ttf
LIBS = -Ldeps/libs -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2main

TARGET = blip

SRC_DIR = src
BUILD_DIR = build

SRCS = $(wildcard $(SRC_DIR)/*.cpp)

OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean