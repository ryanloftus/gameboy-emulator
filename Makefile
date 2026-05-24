# Compiler and flags
CC       = clang
CFLAGS   = -Wall -Wextra -std=c11 -Iinclude $(shell sdl2-config --cflags)
LDFLAGS  = $(shell sdl2-config --libs)

# Build type: default to release
BUILD    ?= release

# Debug flags
ifeq ($(BUILD),debug)
    CFLAGS += -g -O0 -fsanitize=address,undefined
    LDFLAGS += -fsanitize=address,undefined
else
    CFLAGS += -O2
endif

# Build output directory
BUILD_DIR ?= build

# Source files
SRC_DIR  = src
SRCS     = $(wildcard $(SRC_DIR)/*.c)
OBJS     = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/$(SRC_DIR)/%.o,$(SRCS))

# Output binary
TARGET   = gameboy

# Testing
TEST_DIR   = tests
TEST_SRCS  = $(filter-out $(TEST_DIR)/test_main.c, $(wildcard $(TEST_DIR)/*.c)) $(TEST_DIR)/test_main.c
TEST_OBJS  = $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/$(TEST_DIR)/%.o,$(TEST_SRCS))
TEST_BIN   = run_tests
UNITY_DIR  = vendor/unity
UNITY_OBJ  = $(BUILD_DIR)/$(UNITY_DIR)/unity.o

OBJ_DIRS = $(sort $(dir $(OBJS) $(TEST_OBJS) $(UNITY_OBJ)))

# Test CFLAGS: same as CFLAGS but without SDL (tests don't need it)
TEST_CFLAGS = -Wall -Wextra -std=c11 -Iinclude -I$(UNITY_DIR)
ifeq ($(BUILD),debug)
    TEST_CFLAGS += -g -O0 -fsanitize=address,undefined
else
    TEST_CFLAGS += -O2
endif

# Default build
all: $(TARGET)

# Link step
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Compile step
$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)/$(SRC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Run the emulator (example usage: make run ROM=roms/pokemon_red.gb)
run: $(TARGET)
	./$(TARGET) $(ROM)

# Run tests
tests: $(TEST_OBJS) $(UNITY_OBJ) $(filter-out $(BUILD_DIR)/$(SRC_DIR)/main.o, $(OBJS))
	$(CC) -o $(TEST_BIN) $^ $(LDFLAGS)

$(BUILD_DIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/%.c | $(BUILD_DIR)/$(TEST_DIR)
	$(CC) $(TEST_CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(UNITY_DIR)/unity.o: $(UNITY_DIR)/unity.c | $(BUILD_DIR)/$(UNITY_DIR)
	$(CC) $(TEST_CFLAGS) -c $< -o $@

test: tests
	./$(TEST_BIN)

$(OBJ_DIRS):
	mkdir -p $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(SRC_DIR)/*.o $(TEST_DIR)/*.o $(UNITY_DIR)/*.o $(TARGET) $(TEST_BIN)

# Full rebuild
rebuild: clean all

.PHONY: all clean rebuild run tests test
