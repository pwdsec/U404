CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c99 -Iinclude
LDFLAGS ?= -lm -lreadline

SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)
TARGET := u404shell

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
