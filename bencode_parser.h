#define FOREACH_STATE(STATE) \
		STATE(announce)			\
		STATE(announce-list)	\
		STATE(comment)			\
		STATE(created-by)		\
		STATE(creation-date)	\
		STATE(bencode-info)		\
		STATE(url-list)			\

//#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

const char* state[] = { FOREACH_STATE(GENERATE_STRING) };

//enum PARSE_ENUM { FOREACH_STATE(GENERATE_ENUM) };

struct info_file {
	char* length;
	char* path;
};

struct bencode_info {
	struct info_file* files;
	char* name;
	char* piece_length;
	char* pieces;
};

// Only parse necessary + announce_list since it is arguably necessary, otherwise ignore on initial pass. Can go back and parse out optionals later on if necessary but they can't be consistently implemented with state machine scanning

// Pointers to arrays are fixed length so this works with heap
struct bencode_module {
	char* announce;
	char** announce_list;
	//char* comment;
	//char* created_by;
	//char* creation_date;
	//char* encoding;
	struct bencode_info info;
	char** url_list;
};

typedef int (*BlockID)(char*, size_t*, const char**, int*, struct bencode_module*, FILE*, int*, int*);

int pstr(char*, size_t*, FILE*);
int plist(char*, size_t*, const char**, int*, struct bencode_module*, FILE*, int*, int*);
int pdict(char*, size_t*, const char**, int*, struct bencode_module*, FILE*, int*, int*);
int end(char*, size_t*, const char**, int*, struct bencode_module*, FILE*, int*, int*);

/* Tools */
void printBencode(struct bencode_module *bencode, int *index) {
	printf("Announce: %s\n", bencode->announce);
	for (int i = 0; i < *index; i++) {
		printf("Announce-List: %s\n", bencode->announce_list[i]);
	}
}
