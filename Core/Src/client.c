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


int protocol(char *url, struct tracker_properties *props, char **end) 
{
	int protocol_index;
	ptrdiff_t length = 0;
	
	char valid_protocols[3][5] = { 
		"udp", 
		"http", 
		"https"
	};
	int len_protocols = 3;
	
	/* Capturing and error checking delimiter */	
	*end = strstr(url, "://");
	if (*end == NULL) return MALFORMED_PROTOCOL;

	/* Determining length of protocol and storing in struct */
	length = *end - url;	
	strncpy(props->protocol, url, length);
	props->protocol[length] = '\0';

	/* Checking if valid protocol detected */
	for (protocol_index = 0; protocol_index < len_protocols; protocol_index++) {
		if (strcmp(props->protocol, valid_protocols[protocol_index]) == 0) {
			break;
		}
	}
	if (protocol_index == len_protocols) return INVALID_PROTOCOL;
	
	return PARSE_SUCCESS;
}

int hostname(struct bencode_module *bencode, struct tracker_properties *props, char *protocol_end, char **start, char **end) 
{
	ptrdiff_t length = 0;

	/* Offset from protocol_end by length of delimiter */
	*start = protocol_end + 3*sizeof(char);	
	*end = strstr(*start, ":");
	
	/* If no port specified */
	if (*end == NULL) {
		//strncpy(props->hostname, "NULL\0", 5);
		*end = strstr(*start, "/");

		/* If no port or path is present */
		if (*end == NULL) {
			*end = *start + strlen(*start) - sizeof(char);
		}
	}
	
	length = *end - *start;
	strncpy(props->hostname, *start, length);
	props->hostname[length] = '\0';

	return PARSE_SUCCESS;	
}

int port(struct bencode_module *bencode, struct tracker_properties *props, int *tracker, char *hostname_end, char **start, char **end)
{
	ptrdiff_t length;

	*start = hostname_end + sizeof(char);
	*end = strstr(*start, "/");
	if (*end == NULL) {
		*end = &bencode->announce_list[*tracker][strlen(bencode->announce_list[*tracker])];
	}

	length = *end - *start;
	
	strncpy(props->port, *start, length);
	props->port[length] = '\0';

	return PARSE_SUCCESS;
}
int path(struct bencode_module *bencode, struct tracker_properties *props, int *tracker, char *port_end)
{
	ptrdiff_t length;

	length = &bencode->announce_list[*tracker][strlen(bencode->announce_list[*tracker])] - port_end;
	strncpy(props->path, port_end, length);

	return PARSE_SUCCESS;
}

int getTracker(struct bencode_module *bencode, struct hostent *server, struct tracker_properties *props) {
	
	int tracker, result;
	
	char *protocol_end = NULL;
	char *hostname_start = NULL;
	char *hostname_end = NULL;
	
	char *port_start = NULL;
	char *port_end = NULL;


	/* Ignoring announce if announce_list present since index 1 in list will be announce anyway */
	if (bencode->announce_list != NULL) {
		for (tracker = 0; tracker < bencode->announce_list_index; tracker++) {

			result = protocol(bencode->announce_list[tracker], props, &protocol_end);
			if (result != PARSE_SUCCESS) return result;

			result = hostname(bencode, props, protocol_end, &hostname_start, &hostname_end);			
			if (result != PARSE_SUCCESS) return result;

			result = port(bencode, props, &tracker, hostname_end, &port_start, &port_end);
			if (result != PARSE_SUCCESS) return result;

			result = path(bencode, props, &tracker, port_end);
			if (result != PARSE_SUCCESS) return result;

			printf("Tracker: %d\n\tProtocol: %s\n\tHostname: %s\n\tPort: %s\n\tPath: %s\n", tracker, props->protocol, props->hostname, props->port, props->path);

			/* Check for existence/accessibility of tracker */
			testTracker(server, props->hostname);
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
