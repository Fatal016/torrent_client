#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./bencode_parser.h"

int main() {
	enum PARSE_ENUM { FOREACH_STATE(GENERATE_ENUM) };
	static const char *PARSE_STATE[] = { FOREACH_STATE(GENERATE_STRING) };

	struct bencode_module bencode;
	enum PARSE_ENUM state = announce;

	int charIterator;
	char id;
	char readBuffer[128];
	char readBufferIndex;
	
	int stringLength;


	/* Accessing .torrent file in read-only mode */	
	FILE *file = fopen("test.torrent", "r");
	if (file == NULL) {
		perror("Error opening file");
		return -1;
	}


	/* Checking first character for start of dictionary */
	id = fgetc(file);
	if (id != 'd') {
		printf("Improper Bencode format: First char is not dictionary");
		exit(-1);
	}

	for (readBufferIndex = 0; readBufferIndex < sizeof(readBuffer) / sizeof(char); readBufferIndex++) {
		id = fgetc(file);
		if (id != 58) {
			readBuffer[readBufferIndex] = id;
			continue;
		}
		break;
	}
	
	pstr(readBuffer, readBufferIndex, stringLength, file);
	printf("%s\n", readBuffer);

	if (strcmp(readBuffer, PARSE_STATE[state]) == 0) {		
		bencode.announce = readBuffer;
		printf("Length of announce: %ld\n", (sizeof(bencode.announce) / sizeof(char)));
	} else {
		state++;
	}
	fclose(file);
	exit(0);
}

/* (p)arse (str)ing */
void pstr(char* readBuffer, int readBufferIndex, int stringLength, FILE* file) {
    stringLength = atoi(readBuffer);
	memset(readBuffer, 0, sizeof(readBuffer));
    for (readBufferIndex = 0; readBufferIndex < stringLength; readBufferIndex++) {
	    readBuffer[readBufferIndex] = fgetc(file);
    }
}
