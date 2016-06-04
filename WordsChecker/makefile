CC=gcc
CFLAGS=-c -O2 -std=c99 -Wall -mtune=k8
SOURCES=*.c
JUDYLIB=./judy/libJudy.a
OBJECTS=$(SOURCES:.c=.o)
OUTPUT=collections.a

all: $(OUTPUT)
	
$(OUTPUT): $(OBJECTS)
	$(CC) $(OBJECTS) $(JUDYLIB) -o $@

$(OBJECTS): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES)

clean:
	rm $(OBJECTS) 
	rm $(OUTPUT)
