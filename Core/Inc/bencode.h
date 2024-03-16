struct info_file {
	int* length;
	char** path;

	int file_path_index;
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
	int url_list_index;

	char* buffer;
	size_t buffer_size;

	void* head_pointer;
	int* index_pointer;
};

typedef int (*id)(struct bencode_module*, FILE*);

struct bencode_module* parse_single(char*, struct bencode_module*);
int dictionary(struct bencode_module*, FILE*);
int list(struct bencode_module*, FILE*);
int integer(struct bencode_module*, FILE*);
int end(struct bencode_module* __attribute__((unused)), FILE* __attribute__((unused)));

id identify(char c);

/* Tools */
void printBencode(struct bencode_module *bencode);
