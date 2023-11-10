#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "./bencode_parser.h"

int main() {

	struct bencode_module bencode;
	
	enum PARSE_ENUM state = announce;
	static const char *parse_state[] = { FOREACH_STATE(GENERATE_STRING) };

	int charIterator;
	char charIn;
	char readBuffer[128];
	char readBufferIndex;
	
	int stringLength;
	int result;


	/* Accessing .torrent file in read-only mode */	
	FILE *file = fopen("test.torrent", "r");
	if (file == NULL) {
		perror("Error opening file");
		return -1;
	}

	/* Checking first character for start of dictionary */
	charIn = fgetc(file);

	/* Error checking for if file contents are not standard Bencode layout */
	if (charIn != 'd') {
		printf("Improper Bencode Format: First character is not start of dictionary\n");
		return -1;
	}

	/* Beginning root dictionary parse */
	result = pdict(charIn, readBuffer, readBufferIndex, state, parse_state, stringLength, bencode, file);
	
	/* Error handling for root dictionary parse */
	if (result == -1) {
		printf("Error encountered while parsing dictionary\n");
		return -1;
	}

	fclose(file);
	return 0;
}

void plist() {
	printf("into list");
}


/* (p)arse (str)ing */
int pstr(char* readBuffer, int readBufferIndex, int stringLength, FILE* file) {
    stringLength = atoi(readBuffer); // Converting parsed data-segment length into int
	memset(readBuffer, 0, sizeof(readBuffer)); // Clearing readBuffer index in case there's remnants of previous segment
    for (readBufferIndex = 0; readBufferIndex < stringLength; readBufferIndex++) {
	    readBuffer[readBufferIndex] = fgetc(file);
    }
}

BlockID ID(char charIn) {
	switch (charIn) {
		case 'd':
			return pdict;
			break;
		default:
			return NULL;
			break;
	}
}


/* Bencode dictionary parser */
int pdict(char charIn, char* readBuffer, int readBufferIndex, enum PARSE_ENUM state, const char *parse_state[], int stringLength, struct bencode_module bencode, FILE* file) {
	BlockID idresult;
	while (charIn != EOF) {
		for (readBufferIndex = 0; readBufferIndex < sizeof(readBuffer) / sizeof(char); readBufferIndex++) {
			charIn = fgetc(file);
			idresult = ID(charIn);

			if (idresult != NULL) idresult(charIn, readBuffer, readBufferIndex, state, parse_state, stringLength, bencode, file);
			printf("Past function!\n");

			if (charIn != 58) { // Colon is delimiter for separation between data-type and data
				readBuffer[readBufferIndex] = charIn;
				continue;
			}
			break;
		}
		pstr(readBuffer, readBufferIndex, stringLength, file);
		printf("%s\n", readBuffer);	
		exit(1);
		
		if (strcmp(readBuffer, parse_state[state]) == 0) {		
			bencode.announce = readBuffer;
		} else {
			state++;
			printf("%s\n", parse_state[state]);
		}
	}
}
