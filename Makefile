CC=gcc
LD=gcc

RM=rm
CFLAGS+=-Wall
CFLAGS+=-Wunused
CFLAGS+=-pedantic
CFLAGS+=-Werror
CFLAGS+=-pthread

OBJECTS:=bankteller.o

PROG:=bank

.PHONY: clean

all: bank

$(OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< 

$(PROG): $(OBJECTS)
	$(LD) -o $@ $^

clean:
	$(RM) -f *.o $(PROG)
