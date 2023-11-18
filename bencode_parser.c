#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "./bencode_parser.h"

int main() {

	struct bencode_module bencode;
	
	//enum PARSE_ENUM state = announce;
	int state_index = 0;
//static const char *parse_state[] = { FOREACH_STATE(GENERATE_STRING) };

	char charIn;
	char readBuffer[128];
	char readBufferIndex;
	
	int stringLength;
	int result;


/*
	for (int i = 0; i < sizeof(state) / sizeof(char); i++) {
		printf("State: %s\n", state[i]);
	}
*/


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

	size_t bufferLength = sizeof(readBuffer);

	/* Beginning root dictionary parse */
	result = pdict(readBuffer, &bufferLength, state, &state_index, &bencode, file);
	
	/* Error handling for root dictionary parse */
	if (result < 0) {
		printf("Error encountered while parsing dictionary\n");
		exit(-1);
	}

	fclose(file);
	printf("Closing");
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

int plist(char *readBuffer, size_t *sizeof_buffer, const char **state, int *state_index, struct bencode_module *bencode, FILE* file) {
	
	BlockID idresult;

	int charIn;

	int result;

	// All checking for start of new struct, can probably make this into a better method

	
	for (int readBufferIndex = 0; readBufferIndex < *sizeof_buffer / sizeof(char); readBufferIndex++) {	
	
		charIn = fgetc(file);

		idresult = ID(charIn);
		//printf("Inside List: %c\n", charIn);
			
		if (idresult != NULL) {
		//	printf("Non null ID result inside list!\n");
			
			result = idresult(readBuffer, sizeof_buffer, state, state_index, bencode, file);
		} else if (charIn != 58) {
			readBuffer[readBufferIndex] = charIn;
		} else {
			pstr(readBuffer, sizeof_buffer, file);
		}
	}

	//printf("In parse list!\n");
	exit(0);
}

/* (p)arse (dict)ionary */
int pdict(char *readBuffer, size_t *sizeof_buffer, const char **state, int *state_index, struct bencode_module *bencode, FILE *file) {

	BlockID idresult;
	
	int result;
	int stringLength;

	char charIn;
	
	int write = 0;

	/* Iterates until the entire file is parsed */
	while (charIn != EOF) {
		memset(readBuffer, 0, *sizeof_buffer);

		/* Iterates character by character, upper bound of this loop is of arbitrary length and may need to be increased or made dynamic */
		for (int readBufferIndex = 0; readBufferIndex < *sizeof_buffer / sizeof(char); readBufferIndex++) {
			
			/* Reading in character from file */
			charIn = fgetc(file);

			/* Running ID method to determine whether or not the character read is the start of a Bencode data type */
			idresult = ID(charIn);

			/* If the starting character of a data type was detected */
			if (idresult != NULL) {
			//	printf("Non null ID result!\n");

				/* Executing the method pointed to by the return of ID */
				result = idresult(readBuffer, sizeof_buffer, state, state_index, bencode, file);
				
			} else if (charIn != 58) {	// If still reading in length of datablock
				readBuffer[readBufferIndex] = charIn;
			} else {
				pstr(readBuffer, sizeof_buffer, file);
	
				printf("Buffer: %s Looking for: %s\n", readBuffer, state[*state_index]);	
				if (strcmp(readBuffer, state[*state_index]) == 0) {		
					write = 0;
					*(state_index) = *(state_index) + 1;
					
					printf("New state: %s Index: %d\n", state[*state_index], *state_index);
				}
				if (write == 1) {
					printf("Writing\n");
					bencode->announce = readBuffer;
					printBencode(bencode);
				//	printf("Set announce: %s\n", bencode->announce);
				}
				write = 1;
				break;
			}
		}
	}
	
	return 0;

}
