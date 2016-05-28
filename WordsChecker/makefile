CC=gcc
CFLAGS=-c -O2 -std=c99
SOURCES=prefixTree.c main.c
OBJECTS=$(SOURCES:.c=.o)
OUTPUT=wordchecker.a

all: $(OUTPUT)
	echo all
	
$(OUTPUT): $(OBJECTS) 
	echo output
	$(CC) $(OBJECTS) -o $@

$(OBJECTS): $(SOURCES)
	echo objects
	$(CC) $(CFLAGS) $(SOURCES)
