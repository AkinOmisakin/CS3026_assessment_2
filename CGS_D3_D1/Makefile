# Makefile for your C program

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra

# Executable name
TARGET = FAT

# Source files
SRCS = filesys.c shell.c 

# Object files (auto-generated from source files)
OBJS = $(SRCS:.c=.o)

# Default rule: build the executable
all: $(TARGET)

# Rule to link the object files into the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to compile individual source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean up generated files
clean:
	rm -f $(TARGET) $(OBJS)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Phony targets (targets that are not actual files)
.PHONY: all clean run