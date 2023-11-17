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

	/* Error checking for if the file exists */
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
	result = pdict(charIn, readBuffer, sizeof(readBuffer), readBufferIndex, state, parse_state, stringLength, bencode, file);
	
	/* Error handling for root dictionary parse */
	if (result < 0) {
		printf("Error encountered while parsing dictionary\n");
		exit(-1);
	}

	fclose(file);
	exit(0);
}


BlockID ID(char charIn) {
	switch (charIn) {
		case 'd':
			return pdict;
			break;
		case 'l':
			return plist;
			break;
		default:
			return NULL;
			break;
	}
}

/* (p)arse (str)ing */
int pstr(char* readBuffer, size_t* sizeof_buffer, FILE* file) {

	/* Converting parsed data-block length into integer */
	int stringLength = atoi(readBuffer);
	
	/* Clearing readBuffer */
	memset(readBuffer, 0, *sizeof_buffer);
	
	for (int readBufferIndex = 0; readBufferIndex < stringLength; readBufferIndex++) {
		readBuffer[readBufferIndex] = fgetc(file);
	}
	
	return 0;

}

int plist(char charIn, char* readBuffer, size_t sizeof_buffer, int readBufferIndex, enum PARSE_ENUM state, const char *parse_state[], int stringLength, struct bencode_module bencode, FILE* file) {
	printf("In parse list!\n");
	exit(0);
}

/* (p)arse (dict)ionary */
int pdict(char charIn, char* readBuffer, size_t sizeof_buffer, int readBufferIndex, enum PARSE_ENUM state, const char *parse_state[], int stringLength, struct bencode_module bencode, FILE* file) {


	BlockID idresult;
	
	int result;

	int write = 0;

	/* Iterates until the entire file is parsed */
	while (charIn != EOF) {
		printf("back into while\n");
		memset(readBuffer, 0, sizeof_buffer);


		/* Iterates character by character, upper bound of this loop is of arbitrary length and may need to be increased or made dynamic */
		for (readBufferIndex = 0; readBufferIndex < sizeof_buffer / sizeof(char); readBufferIndex++) {
			
			/* Reading in character from file */
			charIn = fgetc(file);
			//printf("Char in %c\n", charIn);

			/* Running ID method to determine whether or not the character read is the start of a Bencode data type */
			idresult = ID(charIn);

			/* If the starting character of a data type was detected */
			if (idresult != NULL) {
				printf("Non null ID result!\n");

				/* Executing the method pointed to by the return of ID */
				result = idresult(charIn, readBuffer, sizeof_buffer, readBufferIndex, state, parse_state, stringLength, bencode, file);
				
				
				printf("Past Function: Return %d\n", result);
			} else if (charIn != 58) {	// If still reading in length of datablock
				readBuffer[readBufferIndex] = charIn;
			} else {
				printf("Parsing String: %s\n", readBuffer);
				pstr(readBuffer, &sizeof_buffer, file);
				printf("ReadBuffer: %s\n", readBuffer);	
		
				if (strcmp(readBuffer, parse_state[state]) == 0) {		
					write = 1;
				} else {
					if (write == 1) {
						bencode.announce = readBuffer;
						printf("Set announce: %s\n", bencode.announce);
					} else {
						state++;
						printf("New State: %s\n", parse_state[state]);
					}
					write = 0;
				}
				break;
			}
		}
		//printf("Done one!\n");
		//printf("Bencode Module %s\n", bencode.announce);
	}
}
