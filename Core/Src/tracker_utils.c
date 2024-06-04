#include <stdio.h>

#include "../Inc/tracker_utils.h"
#include "../Inc/tracker.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

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

int hostname(struct tracker_properties *props, char *protocol_end, char **start, char **end) 
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
