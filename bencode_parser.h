#define FOREACH_STATE(STATE) \
		STATE(announce)			\
		STATE(announce_list)	\
		STATE(comment)			\
		STATE(created_by)		\
		STATE(creation_date)	\
		STATE(bencode_info)		\
		STATE(url_list)			\

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum PARSE_ENUM { FOREACH_STATE(GENERATE_ENUM) };

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

struct bencode_module {
	char* announce;
	char* announce_list;
	char* comment;
	char* created_by;
	char* creation_date;
	char* encoding;
	struct bencode_info info;
	char** url_list;
};

int pstr(char* readBuffer, int readBufferIndex, int stringLength, FILE* file);
int pdict(char charIn, char* readBuffer, int readBufferIndex, enum PARSE_ENUM state, const char *parse_state[], int stringLength, struct bencode_module bencode, FILE* file);
