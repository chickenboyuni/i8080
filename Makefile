CXX = g++
CXXFLAGS = -std=c++17 -Imodules/imgui_club/imgui_memory_editor -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -Wall -Wextra -DNDEBUG
TEST_CXXFLAGS = -std=c++17 -Wall -Wextra
DEBUG_CXXFLAGS = $(filter-out -DNDEBUG, $(CXXFLAGS)) -g

SRC_DIR = src
BUILD_DIR = build
DEBUG_BUILD_DIR = build/debug
BIN_DIR = build/bin

GUI_DIR = src/gui
COMMON_DIR = src/common
DSM_DIR = src/disasm
CORE_DIR = src/core

IMGUI_DIR = modules/imgui

SRCS = $(CORE_DIR)/cpu.cpp $(CORE_DIR)/invaders/main.cpp $(CORE_DIR)/invaders/invaders.cpp $(CORE_DIR)/invaders/shift_register.cpp 
SRCS += $(DSM_DIR)/disasm.cpp
SRCS += $(COMMON_DIR)/binary_reader.cpp 
SRCS += $(GUI_DIR)/gui.cpp 
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS)) 

IMGUI_SRCS = $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
IMGUI_SRCS += $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
IMGUI_OBJS := $(patsubst $(IMGUI_DIR)/%.cpp, $(BUILD_DIR)/imgui/%.o, $(IMGUI_SRCS)) 

TEST_SRCS = $(CORE_DIR)/cpu.cpp $(CORE_DIR)/cpu-test/main.cpp $(COMMON_DIR)/binary_reader.cpp 
TEST_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(TEST_SRCS)) 

DEBUG_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(DEBUG_BUILD_DIR)/%.o, $(SRCS)) 

TARGET = $(BIN_DIR)/main
TEST_TARGET = $(BIN_DIR)/cpu_tests

DEBUG_TARGET = $(BIN_DIR)/dbgmain

UNAME_S := $(shell uname -s)
LINUX_GL_LIBS = -lGL

LIBS =

ifeq ($(UNAME_S), Linux) #LINUX
	LIBS += $(LINUX_GL_LIBS) -ldl `sdl2-config --libs`

	CXXFLAGS += `sdl2-config --cflags`
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo `sdl2-config --libs`
	LIBS += -L/usr/local/lib -L/opt/local/lib

	CXXFLAGS += `sdl2-config --cflags`
	CXXFLAGS += -I/usr/local/include -I/opt/local/include
endif

ifeq ($(OS), Windows_NT)
    LIBS += -lgdi32 -lopengl32 -limm32 `pkg-config --static --libs sdl2`

    CXXFLAGS += `pkg-config --cflags sdl2`
endif

all: $(TARGET)
test: $(TEST_TARGET)

$(TARGET): $(IMGUI_OBJS) $(OBJS)
	@mkdir -p $(BIN_DIR)  
	$(CXX) $(CXXFLAGS) $(LIBS) -o $(TARGET) $(OBJS) $(IMGUI_OBJS) 
	@echo "Built target: $@"  

$(TEST_TARGET): $(TEST_OBJS)
	@mkdir -p $(BIN_DIR)  
	$(CXX) $(TEST_CXXFLAGS) -o $(TEST_TARGET) $(TEST_OBJS)
	@echo "Built cpu tests: $@"  

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# DEBUG 

debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(IMGUI_OBJS) $(DEBUG_OBJS)
	@mkdir -p $(BIN_DIR)  
	$(CXX) $(DEBUG_CXXFLAGS) $(LIBS) -o $(DEBUG_TARGET) $(DEBUG_OBJS) $(IMGUI_OBJS)
	@echo "Built debugging target: $@"  

$(DEBUG_BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(DEBUG_CXXFLAGS) -c $< -o $@

# IMGUI
$(BUILD_DIR)/imgui/%.o:$(IMGUI_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/imgui/backends/%.o:$(IMGUI_DIR)/backends/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

.PHONY: clean remake

remake: clean all

clean:
	rm -rf $(BUILD_DIR)
