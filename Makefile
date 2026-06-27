CXX = g++
CXXFLAGS = -Wall -Wextra -DNDEBUG
DEBUG_CXXFLAGS = -Wall -Wextra -g

SRC_DIR = src
BUILD_DIR = build
DEBUG_BUILD_DIR = build/debug
BIN_DIR = build/bin

COMMON_DIR = src/common
DISASM_DIR = src/disasm
CORE_DIR = src/core

SRCS = $(CORE_DIR)/cpu.cpp $(COMMON_DIR)/binary_reader.cpp $(CORE_DIR)/invaders/main.cpp $(CORE_DIR)/invaders/invaders.cpp
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS)) 
DEBUG_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(DEBUG_BUILD_DIR)/%.o, $(SRCS)) 

TARGET = $(BIN_DIR)/main
DSM_TARGET = $(BIN_DIR)/dsm

DEBUG_TARGET = $(BIN_DIR)/dbgmain

all: $(TARGET)
disasm: $(DSM_TARGET)

debug: $(DEBUG_TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)  
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)
	@echo "Built target: $@"  

$(DSM_TARGET): $(BUILD_DIR)/disasm/disasm.o $(BUILD_DIR)/common/binary_reader.o
	@mkdir -p $(BIN_DIR)  
	$(CXX) $(CXXFLAGS) -o $@ $^ 
	@echo "Built disassembler: $@"  

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# DEBUG 
$(DEBUG_TARGET): $(DEBUG_OBJS)
	@mkdir -p $(BIN_DIR)  
	$(CXX) $(DEBUG_CXXFLAGS) -o $(DEBUG_TARGET) $(DEBUG_OBJS)
	@echo "Built debugging target: $@"  

$(DEBUG_BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(DEBUG_CXXFLAGS) -c $< -o $@

.PHONY: clean remake

remake: clean all

clean:
	rm -rf $(BUILD_DIR)
