.PHONY = all clean rebuild
CCOMP ?= gcc
CFLAGS = -Wall -Werror -fsanitize=undefined -fsanitize=address -g -std=gnu11
LDFLAGS =
TARGET = ./demo

all: $(TARGET)

$(TARGET): demo.c
	$(CCOMP) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	-rm $(TARGET)

rebuild: clean all
