#include <stdio.h>

#include "../Inc/tracker_utils.h"

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
