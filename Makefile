CXX = g++
CXXFLAGS= -Wall -Wextra -g 

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = build/bin

COMMON_DIR = src/common
DISASM_DIR = src/disasm
CORE_DIR = src/core

SRCS = $(CORE_DIR)/cpu.cpp $(COMMON_DIR)/binary_reader.cpp $(CORE_DIR)/invaders/main.cpp $(CORE_DIR)/invaders/invaders.cpp
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS)) 

TARGET = $(BIN_DIR)/main

all: $(TARGET)
disasm: $(BIN_DIR)/disasm

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)  
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)
	@echo "Built target: $@"  
$(BIN_DIR)/disasm: $(BUILD_DIR)/disasm/disasm.o $(BUILD_DIR)/common/binary_reader.o
	@mkdir -p $(BIN_DIR)  
	$(CXX) $(CXXFLAGS) -o $@ $^ 
	@echo "Built disassembler: $@"  

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
