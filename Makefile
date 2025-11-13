# ==============================
#  Makefile for Static Library
# ==============================

# Compiler and flags
CC := gcc
CFLAGS := -Wall -Wextra -O2 -Iinclude

# Directories
SRC_DIR := src
OBJ_DIR := obj
BUILD_DIR := build

# Library name
LIB_NAME := fastest.a
LIB_PATH := $(BUILD_DIR)/$(LIB_NAME)

# Collect all .c files and corresponding .o files
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

# Default target
all: $(LIB_PATH)

# Rule to create the static library
$(LIB_PATH): $(OBJ)
	@mkdir -p $(BUILD_DIR)
	ar rcs $@ $^

# Compile each .c file into .o inside obj/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -rf $(OBJ_DIR) $(BUILD_DIR)

# Print all files (for debugging)
print:
	@echo "SRC: $(SRC)"
	@echo "OBJ: $(OBJ)"

.PHONY: all clean print
