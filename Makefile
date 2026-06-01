CXX = g++
CXXFLAGS= -Wall -Wextra -g 

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = build/bin

DISASM_SRCS = $(SRC_DIR)/disasm/disasm.cpp
SRCS = src/disasm/disasm.cpp
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS)) 

TARGET = $(BIN_DIR)/main

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)  
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)
	@echo "Built target: $@"  

disasm: $(BIN_DIR)/disasm

$(BIN_DIR)/disasm: $(BUILD_DIR)/disasm/disasm.o
	@mkdir -p $(BIN_DIR)  
	$(CXX) $(CXXFLAGS) -o $@ $< 

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
