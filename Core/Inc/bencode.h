struct info_file {
	int* length;
	char* path;
};

struct bencode_info {
	struct info_file* files;
	char* name;
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
	int url_list_index;

	/* Should convert to a void pointer */
	void* head_pointer;
	int* index_pointer;
};

typedef int (*id)(struct bencode_module*, FILE*);

int parse_single(char*);
int dictionary(struct bencode_module*, FILE*);
int list(struct bencode_module*, FILE*);
int integer(struct bencode_module*, FILE*);
int end(struct bencode_module* __attribute__((unused)), FILE* __attribute__((unused)));

void allocate(struct bencode_module*, char**);

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
void printBencode(struct bencode_module *bencode, int *index) {
	printf("Announce: %s\n", (char *)&bencode->announce);
	/*
	for (int i = 0; i < *index; i++) {
		printf("Announce-List: %s\n", bencode->announce_list[i]);
	}
	printf("Comment: %s\n", bencode->comment);
	printf("Created By: %s\n", bencode->created_by);
	printf("Creation Date: %d\n", *bencode->creation_date);
	printf("Encoding: %s\n", bencode->encoding);
	*/
}
