int getTracker(struct bencode_module*, char*, char*, struct hostent*);
int testTracker(struct hostent*, char*);

int parseHostname(char*, char*, char*, char*);
int parsePort(char*, char*, char*, char*, char*);
