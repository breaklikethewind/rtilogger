
CC=gcc
CFLAGS=-c -Wall
LDFLAGS=-lwiringPi -lpthread
SOURCES=rtilogger.c transport.c 
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=rtilogger

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
