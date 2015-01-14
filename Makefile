# Variables
TARGET = libhyperscene.so
SOURCES = hypermath.c vector.c pools.c aabb-tree.c camera.c scene.c lighting.c particles.c

local_CFLAGS += -O3 -Wall -Iinclude/ -Ihypermath/include/

VPATH = src:hypermath/src
PREFIX = /usr/local


# Building
OBJECTS = $(SOURCES:.c=.o)

$(shell mkdir -p lib)
$(shell mkdir -p build)

.PHONY: clean all install uninstall debug

all: lib/$(TARGET)

lib/$(TARGET): $(addprefix build/, $(OBJECTS))
	$(CC) -shared -Wl,-soname,$(TARGET) $(local_LDFLAGS) $(LDFLAGS) $(addprefix build/, $(OBJECTS)) -o lib/$(TARGET)

build/%.o: %.c
	$(CC) -c -fPIC $(local_CFLAGS) $(CFLAGS) $< -o $@

debug: local_CFLAGS += -g -DDEBUG
debug: lib/$(TARGET)

# Installation
install: lib/$(TARGET)
	install lib/$(TARGET) $(PREFIX)/lib
	cp -R include/ $(PREFIX)/include/
	ldconfig $(PREFIX)/lib

uninstall:
	-rm $(PREFIX)/lib/$(TARGET)
	-rm -R $(PREFIX)/include/hypergiant

test:
	$(CC) -Wno-builtin-macro-redefined -I . -D __BASE_FILE__=\"test.c\" -o tests test.c src/vector.c src/pools.c
	./tests

# Cleaning
clean:
	-rm -R lib/ build/ tests
