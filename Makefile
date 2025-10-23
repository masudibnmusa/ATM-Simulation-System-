
## 3. `Makefile`
```makefile
# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
TARGET = atm
SOURCES = atm_simulation.c

# Platform detection
ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET).exe
    RM = del /Q
else
    RM = rm -f
endif

# Default target
all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

# Clean build artifacts
clean:
	$(RM) $(TARGET)
	$(RM) accounts.txt transactions.txt

# Install (copy to /usr/local/bin on Unix-like systems)
install: $(TARGET)
ifeq ($(OS),Windows_NT)
	copy $(TARGET) C:\Windows\System32\
else
	cp $(TARGET) /usr/local/bin/
endif

# Run the program
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean debug install run
