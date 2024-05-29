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

int getTracker(struct bencode_module*, struct hostent*, struct tracker_properties*);
int testTracker(struct hostent*, char*);

int protocol(char*, struct tracker_properties*, char**);
int hostname(struct bencode_module*, struct tracker_properties*, char*, char**, char**);
int port(struct bencode_module*, struct tracker_properties*, int*, char*, char**, char**);
int path(struct bencode_module*, struct tracker_properties*, int*, char*);
