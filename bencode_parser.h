enum parse_state { announce, announce_list, comment, created_by, creation_date, bencode_info, url_list };

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
