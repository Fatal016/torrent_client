CC = gcc
CFLAGS = -Wall -g -Wextra -std=gnu11

CC_SOURCES = \
Core/Src/bencode.c \
Core/Src/client.c

HEADERS = \
Core/Inc/bencode.h \
Core/Inc/client.h

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
