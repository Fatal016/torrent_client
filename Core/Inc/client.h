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

int parseHostname(char*, char**, char**, char**);
int parsePort(char*, char**, char**, char**, char**);
