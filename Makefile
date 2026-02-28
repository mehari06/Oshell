# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -D_GNU_SOURCE
INC = -Iinclude

# Target executable
TARGET = oshell

# Automatically include all .c files in src/
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

# Default target
all: $(TARGET)

# Link object files into the final executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

# Compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

# Remove object files and executable
clean:
	rm -f $(OBJ) $(TARGET)

