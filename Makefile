CC = gcc
CFLAGS = -Wall -g -Wextra -pedantic

CC_SOURCES = \
Core/Src/bencode.c \
Core/Src/tracker.c \
Core/Src/tracker_utils.c

HEADERS = \
Core/Inc/bencode.h \
Core/Inc/tracker.h \
Core/Inc/tracker_utils.h

EXECUTABLE = bencode

OBJECTS = $(CC_SOURCES:.c=.o)

all: $(EXECUTABLE)
	rm -f $(OBJECTS)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
