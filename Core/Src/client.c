#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <netdb.h>
#include <sys/socket.h>
#include <libwebsockets.h>

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

	struct bencode_module bencode = {
		.announce = NULL,
		.announce_list = NULL,
		.comment = NULL,
		.created_by = NULL,
		.encoding = NULL,
		.info = NULL,
		.url_list = NULL,
		.head_pointer = NULL,
		.size_pointer = NULL,
		.announce_list_index = 0,
		.info_file_index = 0,
		.file_path_index = 0,
		.url_list_index = 0
	};
	
	parse_single(filepath, &bencode);
	return 0;

	
	/* Opening a socket file descriptor */
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd < 0) {
		fprintf(stderr, "Error socket: %s\n", strerror(errno));
	}

	result = getTracker(&bencode, hostname, port, server);

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

			memset(hostname, 0, HOSTNAME_SIZE);
			memset(port, 0, PORT_SIZE);
		
			result = parseHostname(bencode->announce_list[tracker], &hostname_start, &hostname_end, &hostname);
			
			/* Error handling for hostname parsing */
			if (result == 1) {
				printf("Error in parsing %s", bencode->announce_list[tracker]);
				return 1;
			}

			/* Check for existence/accessibility of tracker */
			testTracker(server, hostname);

			result = parsePort(bencode->announce_list[tracker], &hostname_end, &port_start, &port_end, &port);
		}
			
	} else {
		result = parseHostname(bencode->announce, &hostname_start, &hostname_end, &hostname);

		if (result == 1) {
			printf ("Error in parsing %s\n", bencode->announce);
			return 1;
		}

		testTracker(server, hostname);

		result = parsePort(bencode->announce, &hostname_end, &port_start, &port_end, &port);
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

int parseHostname(char *url, char **hostname_start, char **hostname_end, char **hostname) {
	
	*hostname_start = strstr(url, "//");
	
	/* Error handling for lack of http[s]// in URL */
	if (hostname_start == NULL) {
		printf("Error in parsing URL: No '//' present\n");
		return 1;
	}	

	*hostname_end = strstr(*hostname_start + 2, ":");
	
	/* Error handling for lack of port specification in URL */
	/* Assuming wss as of now */
	if (*hostname_end == NULL) {
		strncpy(*hostname, *hostname_start + 2, strlen(url) - 2);
		return 0;	
	}	

	// Checking length constraints
	/*
	if ((&hostname_end - &hostname_start - 2) - (hostname_start + 2) > 255) {
		printf("Error in parsing URL: Hostname exceeds maximum possible length\n");
		return 1;
	}	
	*/

	strncpy(*hostname, *hostname_start + 2, *hostname_end - *hostname_start - 2);
	
	return 0;	
}

int parsePort(char *url, char **hostname_end, char **port_start, char **port_end, char **port) {

	/* If wss url */
	if (*hostname_end == '\0') {
		strcpy(*port, "NULL");
		return 0;
	}

	/* Parsing port */
	*port_start = *hostname_end + 1;
	
	*port_end = strstr(*hostname_end, "/");

	/* Capturing until end of url if no filepath is listed */
	if (*port_end == NULL) {
		*port_end = url + strlen(url);
	}

	strncpy(*port, *port_start, *port_end - *port_start);
	
	return 0;
}
