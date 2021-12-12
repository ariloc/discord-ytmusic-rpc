SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := .

EXE := $(BIN_DIR)/discord-ytmusic-rpc
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

CXX = g++
CPPFLAGS := $(shell python3-config --cflags) -Iinclude -MMD -MP
CXXFLAGS := -Wall
LDLIBS := -lpthread -lpython3.9 -ldiscord-rpc
LDFLAGS := -Llib $(shell python3-config --ldflags)

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJS) | $(BIN_DIR)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@ 
	
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
	
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@
	
clean:
	@$(RM) -rvf $(OBJS) $(BIN_DIR) $(OBJ_DIR)
	
-include $(OBJ:.o=.d)
