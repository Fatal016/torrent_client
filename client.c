#include <stdio.h>

int main() {
	enum parse_state{ BENCODE_TYPE, BENCODE_LENGTH, BENCODE_CONTENTS };
		
	FILE *file = fopen("test.torrent", "r");
	if (file == NULL) {
		perror("Error opening file");
		return -1;
	}

	char id;
	enum parse_state state;

	while (1) {
		id = fgetc(file);
		switch (state) {
			case BENCODE_TYPE:
				id = fgetc(file);
				printf("%d\n", id);
				state = id >= 97 ? BENCODE_TYPE : BENCODE_LENGTH;
				break;
		}
		return 0;
	}

	
	
	
	
	fclose(file);
	return 0;
}


struct torrent_info {
	char* announce;
	char** announce_list;
	char* comment;
	char* creation_date;
	char* encoding;
	char* info;
	char** files; // length:path
	
}
