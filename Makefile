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
TEST_DIR = tests
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS = $(TEST_SRCS:.c=.o)
TEST_BIN  = run_tests

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
tests: $(TEST_OBJS) $(filter-out $(SRC_DIR)/main.o, $(OBJS))
	$(CC) -o $(TEST_BIN) $^ $(LDFLAGS)

# Clean build artifacts
clean:
	rm -f $(SRC_DIR)/*.o $(TARGET) $(TEST_DIR)/*.o $(TEST_BIN)

# Full rebuild
rebuild: clean all

.PHONY: all clean rebuild run
