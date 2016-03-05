
CC=/usr/local/x86_64-pc-linux-gnu/bin/x86_64-pc-linux-gnu-gcc
CFLAGS=-c -Wall
LDFLAGS=-pthread
SOURCES=rtilogger.c transport.c 
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=rtilogger

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
