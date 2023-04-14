# Compiler and compiler flags
CC = gcc
CFLAGS = -Wall -Wextra -pthread

# Source files
SRC = main.c graph.c echo.c

# Executable
EXEC = echo_algorithm

all: $(EXEC)

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean

clean:
	rm -f $(EXEC)
