#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>

#include <sys/socket.h>

#include "../Inc/bencode.h"
#include "../Inc/client.h"

#define DEFAULT_PORT 6969
#define HOSTNAME_SIZE 128
#define PORT_SIZE 128

#define BUFFER_SIZE 256

int main(int argc, char **argv) {
	
	int result;
	int socket_fd;
	struct hostent *server = NULL;
	//struct sockaddr_in server_addr;
	//char buffer[BUFFER_SIZE];
	//int n;
	//int time_to_exit = 0;
	//unsigned int serverlen;
	//char hostname_start[256], hostname_end[256], port_start[256], port_end[256];
	//char hostname[256], port[5];
	char *hostname, *port;


	if (argc != 2) {
		printf("passed wrong amount\n");
		exit(-1);
	}
	
	hostname = (char *)malloc(HOSTNAME_SIZE * sizeof(char));
	port = (char *)malloc(PORT_SIZE * sizeof(char));
	
	char *filepath = argv[1];

	struct bencode_module bencode;
	parse_single(filepath, &bencode);
	
	/* Opening a socket file descriptor */
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd < 0) {
		fprintf(stderr, "Error socket: %s\n", strerror(errno));
	}

	result = getTracker(&bencode, port, hostname, server);

	if (result == 0) {
		printf("Good!\n");
	}
	printf("Connecting to server %s port %d\n", hostname, atoi(port));

	return 1;
}

int getTracker(struct bencode_module *bencode, char *hostname, char *port, struct hostent *server) {
	
	char *hostname_start = NULL;
	char *hostname_end = NULL;
	char *port_start = NULL;
	char *port_end = NULL;

	int result;	
	
	if (bencode->announce_list != NULL) {
		for (int tracker = 0; tracker < bencode->announce_list_index; tracker++) {
		
			result = parseHostname(bencode->announce_list[tracker], hostname_start, hostname_end, hostname);
			
			/* Error handling for hostname parsing */
			if (result == 1) {
				printf("Error in parsing %s", bencode->announce_list[tracker]);
				return 1;
			}

			/* Check for existence/accessibility of tracker */
			testTracker(server, hostname);

			result = parsePort(bencode->announce_list[tracker], hostname_end, port_start, port_end, port);
		}
			
	} else {
		
	}
	
	return 0;
}


int testTracker(struct hostent *server, char *hostname) {
	server = gethostbyname(hostname);

	/* Should switch to prioritization of more trustworthy/valuable trackers */
	if (server == NULL) {
		printf("Server unavailable!\n");
		fprintf(stderr, "ERROR! No such host!\n");
		return 1;
	} else {

		return 0;
	}
}

int parseHostname(char *url, char *hostname_start, char *hostname_end, char *hostname) {
	
	hostname_start = strstr(url, "//");
	
	/* Error handling for lack of http[s]// in URL */
	if (hostname_start == NULL) {
		printf("Error in parsing URL: No '//' present\n");
		return 1;
	}	

	hostname_end = strstr(hostname_start + 2, ":");
	
	/* Error handling for lack of port specification in URL */
	if (hostname_end == NULL) {
		printf("Error in parsing URL: No ':' present\n");
		return 1;
	}	

	// Checking length constraints
	/*
	if ((&hostname_end - &hostname_start - 2) - (hostname_start + 2) > 255) {
		printf("Error in parsing URL: Hostname exceeds maximum possible length\n");
		return 1;
	}	
	*/

	strncpy(hostname, hostname_start + 2, hostname_end - hostname_start - 2);
	
	return 0;	
}

int parsePort(char *url, char *hostname_end, char *port_start, char *port_end, char *port) {

	/* Parsing port */
	port_start = hostname_end + 1;
	port_end = strstr(url, "/");
	/*
	if (port_end == NULL) {
		port_end = url + sizeof(url) - 1;
	}
	*/
	printf("Start: %p End: %p", &port_start, &port_end);
	
	/* Error handling for lack of port termination in URL */
	if (port_end == NULL) {
		printf("Error in parsing URL: No '/' present\n");
		return 1;
	}	

	strncpy(port, port_start, port_end - port_start);
	printf("Port: %s\n", port);
	
	return 0;
}
