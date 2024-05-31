#define INVALID 0
#define VALID 1

#define MALFORMED_PROTOCOL 100
#define INVALID_PROTOCOL 2

struct tracker_properties {
    char *protocol;
    char *hostname;
    char *port;
	char *path;
};

#pragma pack(push, 1)
struct connect_request {
	uint64_t protocol_id;
	uint32_t action;
	uint32_t transaction_id;
};
#pragma pack(pop)

struct connect_response {
	uint32_t action;
	uint32_t transaction_id;
	uint64_t connection_id;
};

int getTracker(struct bencode_module*, struct hostent*, struct tracker_properties*);
int testTracker(struct hostent*, char*);

int protocol(char*, struct tracker_properties*, char**);
int hostname(struct bencode_module*, struct tracker_properties*, char*, char**, char**);
int port(struct bencode_module*, struct tracker_properties*, int*, char*, char**, char**);
int path(struct bencode_module*, struct tracker_properties*, int*, char*);

uint32_t buffer_to_u32(const unsigned char*);

