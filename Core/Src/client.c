#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <pthread.h>

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

	struct tracker_properties props;
	props.protocol = malloc(sizeof(char) * 128);
	props.hostname = malloc(sizeof(char) * 128);
	props.port = malloc(sizeof(char) * 128);
	props.path = malloc(sizeof(char) * 128);

	if (argc != 2) {
		printf("passed wrong amount\n");
		exit(-1);
	}
	
//	hostname = (char *)malloc(HOSTNAME_SIZE * sizeof(char));
//	port = (char *)malloc(PORT_SIZE * sizeof(char));
	
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

	/* Opening a socket file descriptor */
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd < 0) {
		fprintf(stderr, "Error socket: %s\n", strerror(errno));
	}

	result = getTracker(&bencode, server, &props);

	if (result == 1) {
		return 1;
	} else {
		return 0;
	}
}





int getTracker(struct bencode_module *bencode, struct hostent *server, struct tracker_properties *props) {
	
	int tracker, protocol_index;	
	ptrdiff_t length = 0;
	
	char *protocol_end;
	char *hostname_start;
	char *hostname_end;
	
	char *port_start;
	char *port_end;
	char *path_start;

	char valid_protocols[3][5] = { 
		"udp", 
		"http", 
		"https"
	};
	int len_protocols = 3;	

	/* Ignoring announce if announce_list present since index 1 in list will be announce anyway */
	if (bencode->announce_list != NULL) {
		for (tracker = 0; tracker < bencode->announce_list_index; tracker++) {

			length = 0;

			/* Capturing protocol of tracker */	
			protocol_end = strstr(bencode->announce_list[tracker], "://");
			
			/* Error checking for protocol detection */
			if (protocol_end == NULL) return MALFORMED_PROTOCOL;

			length = protocol_end - bencode->announce_list[tracker];
			
			strncpy(props->protocol, bencode->announce_list[tracker], length);
			props->protocol[length] = '\0';
		
			/* Checking if valid protocol detected */	
			for (protocol_index = 0; protocol_index < len_protocols; protocol_index++) {
				if (strcmp(props->protocol, valid_protocols[protocol_index]) == 0) {
					break;
				}
			}
			if (protocol_index == len_protocols) return INVALID_PROTOCOL;

			hostname_start = protocol_end + 3*sizeof(char);	
			hostname_end = strstr(hostname_start, ":");
			
			if (hostname_end == NULL) {
				strncpy(props->hostname, "NULL\0", 4);
				hostname_end = strstr(hostname_start, "/") - sizeof(char);

				if (hostname_end == NULL) {
					hostname_end = strstr(hostname_start, "\0") - sizeof(char);
				}
			} else {
				length = hostname_end - hostname_start;
				strncpy(props->hostname, hostname_start, length);
				props->hostname[length] = '\0';
			}			

			//printf("Hostname: %s\n", props->hostname);

			port_start = hostname_end + sizeof(char);
			port_end = strstr(port_start, "/");
			if (port_end == NULL) {
				
				/* Needs fixing */
				port_end = &bencode->announce_list[tracker][strlen(bencode->announce_list[tracker]) - 1];
			}

			length = port_end - port_start;
			
			strncpy(props->port, port_start, length);
			props->port[length] = '\0';

			length = &bencode->announce_list[tracker][strlen(bencode->announce_list[tracker])] - port_end;
			strncpy(props->path, port_end, length);

			printf("Tracker: %d\n\tProtocol: %s\n\tHostname: %s\n\tPort: %s\n\tPath: %s\n", tracker, props->protocol, props->hostname, props->port, props->path);

			/* Check for existence/accessibility of tracker */
	//		testTracker(server, hostname);
		}
			
	} else {
//		result = parseHostname(bencode->announce, &hostname_start, &hostname_end, &hostname);

//		if (result == 1) {
//			printf ("Error in parsing %s\n", bencode->announce);
//			return 1;
//		}

//		testTracker(server, hostname);

//		result = parsePort(bencode->announce, &hostname_end, &port_start, &port_end, &port);
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
