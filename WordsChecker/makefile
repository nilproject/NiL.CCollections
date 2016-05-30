CC=gcc
CFLAGS=-c -O2 -std=c99 -Wall
SOURCES=prefixTree.c HashMap.c main.c
OBJECTS=$(SOURCES:.c=.o)
OUTPUT=collections.a

all: $(OUTPUT)
	
$(OUTPUT): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

$(OBJECTS): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES)

clean:
	rm $(OUTPUT)
	rm $(OBJECTS) 
