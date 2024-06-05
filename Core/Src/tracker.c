#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <netdb.h>
#include <sys/socket.h>

#include "../Inc/bencode.h"
#include "../Inc/tracker.h"

#define DEFAULT_PORT 6969
#define HOSTNAME_SIZE 128
#define PORT_SIZE 128

#define BUFFER_SIZE 256

/* One instance allocated per .torrent file/magnet */

int main(int argc, char **argv) {
	
	int result;

	if (argc != 2) {
		printf("**TEST CHECK**\npassed wrong amount\n");
		exit(-1);
	}

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

	result = parse_single(filepath, &bencode);

	printBencode(&bencode);

	if (result != 0) {
		printf("Failed to parse file\n");
		exit(-1);
	}

	struct tracker_properties props = {
		.protocol 	= (char *)malloc(sizeof(char) * 255),
		.hostname 	= (char *)malloc(sizeof(char) * 253),
		.port 		= (char *)malloc(sizeof(char) * 5),
		.path 		= (char *)malloc(sizeof(char) * 2048)
	};
	
	result = getTracker(&bencode, &props);

	if (result == 1) {
		return 1;
	} else {
		return 0;
	}
}

int getTracker(struct bencode_module *bencode, struct tracker_properties *props) {
	
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

			result = hostname(props, protocol_end, &hostname_start, &hostname_end);			
			if (result != PARSE_SUCCESS) return result;

			result = port(bencode, props, &tracker, hostname_end, &port_start, &port_end);
			if (result != PARSE_SUCCESS) return result;

			result = path(bencode, props, &tracker, port_end);
			if (result != PARSE_SUCCESS) return result;

			if (strcmp(props->protocol, "udp") == 0) {
			
				int socket_fd;

				uint32_t transaction_id = rand();
			//	uint32_t transaction_id = 0;
			//	transaction_id = rand() & 0xff;
			//	transaction_id |= (rand() & 0xff) << 8;
			//	transaction_id |= (rand() & 0xff) << 16;
			//	transaction_id |= (rand() & 0xff) << 24;

				//printf("Transaction ID: %lu\n", transaction_id);

				/* Opening a socket file descriptor */
				socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
				if (socket_fd < 0) {
					fprintf(stderr, "Error socket: %s\n", strerror(errno));
				}
		

				char *return_buffer = (char *)malloc(sizeof(char)*16);
			
				struct connect_request connect_req = {
					.protocol_id = htonll(0x41727101980),
					.action = 0,
					.transaction_id = htonl(transaction_id)
				};

//				printf("Before %lx After: %lx\n", 0x41727101980, connect_packet.protocol_id);
				
				struct addrinfo hints;
				struct addrinfo *result, *rp;
				int sfd, s;
				ssize_t nread;

				memset(&hints, 0, sizeof(struct addrinfo));
    			hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    			hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
   		 		hints.ai_flags = 0;
    			hints.ai_protocol = 0;          /* Any protocol */				

				s = getaddrinfo(props->hostname, props->port, &hints, &result);
				if (s != 0) {
        			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        			continue;
    			}			

				for (rp = result; rp != NULL; rp = rp->ai_next) {
        			sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        			if (sfd == -1) continue;

       				if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
           				printf("Success! Transaction ID: %x\n", connect_req.transaction_id);
			 			break;                  /* Success */
					}
       				close(sfd);
    			}

				if (rp == NULL) {
					fprintf(stderr, "Could not connect\n");
					return -2;
				}

				freeaddrinfo(result);


				struct timeval timeout;
    			timeout.tv_sec = 2;  // 5 seconds timeout
    			timeout.tv_usec = 0;

    			if (setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        			perror("setsockopt");
        			close(sfd);
        			continue;
    			}
				


				if (send(sfd, (void *)&connect_req, sizeof(connect_req), 0) != sizeof(connect_req)) {
					fprintf(stderr, "partial/failed write\n");
					return -2;
				}

				nread = recv(sfd, return_buffer, 16, 0);
				if (nread == -1) {
					perror("read");
					continue;
				}
					
				struct connect_response connect_res = {
					.action = buffer_to_u32(return_buffer),
					.transaction_id = buffer_to_u32(return_buffer + 4),
					.connection_id = buffer_to_u64(return_buffer + 8)
				};
	


				printf("Response:\n\tAction: %x\n\tTransaction ID: %x\n\tConnection ID: %lx\n", connect_res.action, htonl(connect_res.transaction_id), htonll(connect_res.connection_id));

				if (transaction_id == connect_res.transaction_id) {
					printf("Transaction IDs match!\n");
				} else {
					printf("Transaction IDs do not match...\n");
					continue;
				}

				if (connect_res.action != 0) {
					printf("Not action???\n");
					continue;
				}

				
				struct announce_request announce_req = {
					.connection_id = connect_res.connection_id,
					.action = 1,
					.transaction_id = connect_res.transaction_id
			//		.info_hash = 
				};
				

				exit(0);
			}
		}
	}
	return 0;
}

uint32_t buffer_to_u32(char *buf) {
	uint32_t ret = 0;	
    
	ret = 	((uint32_t)(uint8_t)buf[3] << 24) |
            ((uint32_t)(uint8_t)buf[2] << 16) |
            ((uint32_t)(uint8_t)buf[1] << 8)  |
            ((uint32_t)(uint8_t)buf[0]);
    asm ("bswap %0" : "=r" (ret) : "0" (ret));
	
	return ret;
}

uint64_t buffer_to_u64(char *buf) {
    uint64_t ret = 0;

	ret =	((uint64_t)(uint8_t)buf[7] << 56) |
			((uint64_t)(uint8_t)buf[6] << 48) |
			((uint64_t)(uint8_t)buf[5] << 40) |
			((uint64_t)(uint8_t)buf[4] << 32) |
			((uint64_t)(uint8_t)buf[3] << 24) |
			((uint64_t)(uint8_t)buf[2] << 16) |
			((uint64_t)(uint8_t)buf[1] << 8)  |
			((uint64_t)(uint8_t)buf[0]);
	asm ("bswap %0" : "=r" (ret) : "0" (ret));

    return ret;
}
