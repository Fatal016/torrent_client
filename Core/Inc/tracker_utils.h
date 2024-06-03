#include "bencode.h"
#include <stdint.h>

#ifndef H_TRACKER_UTILS
#define H_TRACKER_UTILS

struct tracker_properties {
    char *protocol;
    char *hostname;
    char *port;
	char *path;
};

int protocol(char*, struct tracker_properties*, char**);
int hostname(struct tracker_properties*, char*, char**, char**);
int port(struct bencode_module*, struct tracker_properties*, int*, char*, char**, char**);
int path(struct bencode_module*, struct tracker_properties*, int*, char*);

#endif
