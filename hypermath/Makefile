# Variables
TARGET = libhypermath.so
SOURCES = hypermath.c

local_CFLAGS += -O3 -Wall -Iinclude/

VPATH = src
PREFIX = /usr/local


# Building
OBJECTS = $(SOURCES:.c=.o)

$(shell mkdir -p lib)
$(shell mkdir -p build)

.PHONY: clean all install uninstall

all: lib/$(TARGET)

lib/$(TARGET): $(addprefix build/, $(OBJECTS))
	$(CC) -shared -Wl,-soname,$(TARGET) $(local_LDFLAGS) $(LDFLAGS) $(addprefix build/, $(OBJECTS)) -o lib/$(TARGET)

build/%.o: %.c
	$(CC) -c -fPIC $(local_CFLAGS) $(CFLAGS) $< -o $@

# Installation
install: lib/$(TARGET)
	install lib/$(TARGET) $(PREFIX)/lib
	cp -R include/ $(PREFIX)/include/
	ldconfig $(PREFIX)/lib

uninstall:
	-rm $(PREFIX)/lib/$(TARGET)
	-rm -R $(PREFIX)/include/hypergiant

# Cleaning
clean:
	-rm -R lib/ build/
