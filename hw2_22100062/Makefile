# Makefile for the readers-writers problem assignment 

# Compiler
CC = gcc

# Compiler flags
# -g: for debugging
# -Wall: to turn on most warnings
# -pthread: to link the pthread library
CFLAGS = -g -Wall -pthread

# The target executable
TARGET = rwlock

# The source file
SRC = rwlock.c

# Default rule
all: $(TARGET)

# Rule to build the target
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Rule to clean up generated files
clean:
	rm -f $(TARGET)