struct BencodeValue {
	enum { BENCODE_INTEGER, BENCODE_STRING, BENCODE_LIST, BENCODE_DICT } type;
	union {
		long long integer;
		char* string;
		struct BencodeValue* list;
		struct BencodeDictPair *dict;
	} data;
};

struct BencodeValue* parse_bencode(const char* input);
