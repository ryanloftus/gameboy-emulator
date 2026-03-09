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

# Source files
SRC_DIR  = src
SRCS     = $(wildcard $(SRC_DIR)/*.c)
OBJS     = $(SRCS:.c=.o)

# Output binary
TARGET   = gameboy

# Testing
TEST_DIR   = tests
TEST_SRCS  = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS  = $(TEST_SRCS:.c=.o)
TEST_BIN   = run_tests
UNITY_DIR  = vendor/unity
UNITY_OBJ  = $(UNITY_DIR)/unity.o

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
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run the emulator (example usage: make run ROM=roms/pokemon_red.gb)
run: $(TARGET)
	./$(TARGET) $(ROM)

# Run tests
tests: $(TEST_OBJS) $(UNITY_OBJ) $(filter-out $(SRC_DIR)/main.o, $(OBJS))
	$(CC) -o $(TEST_BIN) $^ $(LDFLAGS)

$(TEST_DIR)/%.o: $(TEST_DIR)/%.c
	$(CC) $(TEST_CFLAGS) -c $< -o $@

$(UNITY_DIR)/unity.o: $(UNITY_DIR)/unity.c
	$(CC) $(TEST_CFLAGS) -c $< -o $@

test: tests
	./$(TEST_BIN)

# Clean build artifacts
clean:
	rm -f $(SRC_DIR)/*.o $(TARGET) $(TEST_DIR)/*.o $(UNITY_DIR)/*.o $(TEST_BIN)

# Full rebuild
rebuild: clean all

.PHONY: all clean rebuild run tests test
