struct info_file {
	int* length;
	char** path;
};

struct bencode_info {
	struct info_file** files;
	char* name;
	int* length;
	int* piece_length;
	char* pieces;
};

struct bencode_module {
	char* announce;
	char** announce_list;
	char* comment;
	char* created_by;
	int* creation_date;
	char* encoding;
	struct bencode_info *info;	
	char** url_list;
	
	int announce_list_index;
	int info_file_index;
	int file_path_index;
	int url_list_index;

	char* buffer;
	size_t buffer_size;

	void* head_pointer;
	int* index_pointer;
};

typedef int (*id)(struct bencode_module*, FILE*);

int parse_single(char*);
int dictionary(struct bencode_module*, FILE*);
int list(struct bencode_module*, FILE*);
int integer(struct bencode_module*, FILE*);
int end(struct bencode_module* __attribute__((unused)), FILE* __attribute__((unused)));

id identify(char c) {
	switch (c) {
		case 'd':
			return dictionary;
			break;
		case 'l':
			return list;
			break;
		case 'i':
			return integer;
			break;
		case 'e':
			return end;
			break;
		default:
			return NULL;
			break;
	}
}


/* Tools */
void printBencode(struct bencode_module *bencode) {
	printf("Announce: %s\n\n", bencode->announce);
	for (int i = 0; i < bencode->announce_list_index; i++) {
		printf("Announce-List %d: %s\n", i, bencode->announce_list[i]);
	}
	printf("\nComment: %s\n", bencode->comment);
	printf("Created By: %s\n", bencode->created_by);
	printf("Creation Date: %d\n", *bencode->creation_date);
	if (bencode->encoding != NULL) printf("Encoding: %s\n\n", bencode->encoding);
	for (int i = 0; i < bencode->info_file_index; i++) {
		printf("Info File %d: Length: %d Path: %s\n", i, *bencode->info->files[i]->length, *bencode->info->files[i]->path);
	}
	printf("\nName: %s\n", bencode->info->name);
	printf("Piece Length: %d\n", *bencode->info->piece_length);
	printf("Pieces: %s\n\n", bencode->info->pieces);
	for (int i = 0; i < bencode->url_list_index; i++) {
		printf("Url List %d: %s\n", i, bencode->url_list[i]);
	}
}
