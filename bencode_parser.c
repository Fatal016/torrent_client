#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./bencode_parser.h"

int main() {
	struct bencode_module bencode;
	enum parse_state state = announce;;

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
	printf("Buffer Length: %d\n", readBufferIndex);
	
	stringLength = atoi(readBuffer);
	printf("String Length: %d\n", stringLength);
	
	memset(readBuffer, 0, sizeof(readBuffer));
	for (int i = 0; i < stringLength; i++) {
		readBuffer[i] = fgetc(file);
	}

	printf("%s\n", readBuffer);

	if (strcmp(readBuffer, "announce") == 0) {
		
		bencode.announce = readBuffer;
		printf("Length of announce: %ld\n", (sizeof(bencode.announce) / sizeof(char)));
		//printf("Found announcement\n");
//	} else if (strcmp(readBuffer, ))
	}
	fclose(file);
	exit(0);
}
