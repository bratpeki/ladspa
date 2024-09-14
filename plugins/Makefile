CC = gcc
CFLAGS = -fPIC -Wall -Werror -Wpedantic
LDFLAGS = -shared -lm -ldl

SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
TARGETS = $(SOURCES:.c=.so)

build: $(TARGETS)

%.so: %.o
	$(CC) $(LDFLAGS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o *.so
