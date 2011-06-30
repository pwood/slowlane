RM=/bin/rm
CC=gcc

INCLUDEDIR=-I../include

SOURCES=main.c crc32.c dvb.c si.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=slowlane

build: all

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) ${LDFLAGS} -o ../$@ $(OBJECTS)

clean:
	$(RM) -f $(OBJECTS) *~

.c.o:
	$(CC) $(CFLAGS) $(INCLUDEDIR) $< -c
