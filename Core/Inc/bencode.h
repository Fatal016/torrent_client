#define DATA_LENGTH_EXCEEDED -2



/* Struct for bencode -> info -> files */
struct info_file {
	int* length;
	char** path;

	int file_path_index;
};

/* Struct for bencode -> info */
struct bencode_info {
	struct info_file** files;
	char* name;
	int* length;
	int* piece_length;
	char* pieces;
};

/* Core struct for storing information parsed from .torrent file */
struct bencode_module {
	
	/**************************/
	/*** Bencode components ***/
	/**************************/

	char* announce;
	char** announce_list;
	char* comment;
	char* created_by;
	int* creation_date;
	char* encoding;
	struct bencode_info *info;	
	char** url_list;


	/***********************/
	/*** List Parameters ***/
	/***********************/	

	/* Indexes for list components */
	int announce_list_index;
	int info_file_index;
	int file_path_index;
	int url_list_index;

	/* Sizes for list components */
	int announce_list_size;
	int info_file_size;
	int file_path_size;
	int url_list_size;

	/* Tracking for features of lists */
	void* head_pointer;
	int* index_pointer;
	int* size_pointer;

	
    /************************/
	/*** Other Parameters ***/
	/************************/

	/* Input buffer for filestream */
	char* buffer;
	size_t buffer_size;
};

/* Serves as template for return-type of 'identify' */
typedef int (*id)(struct bencode_module*, FILE*);

/* For identifying type of component being read */
id identify(char c);

/* Bencode component parsing functions definitions */
int dictionary(struct bencode_module*, FILE*);
int list(struct bencode_module*, FILE*);
int integer(struct bencode_module*, FILE*);
int end(struct bencode_module* __attribute__((unused)), FILE* __attribute__((unused)));

/* Root function for parsing .torrent file */
struct bencode_module* parse_single(char*, struct bencode_module*);

/* Tools */
void printBencode(struct bencode_module *bencode);
