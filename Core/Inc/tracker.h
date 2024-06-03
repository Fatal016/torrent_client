#include <stdint.h>

#include "tracker_utils.h"

#ifndef H_TRACKER
#define H_TRACKER

#define INVALID 0
#define VALID 1
#define MALFORMED_PROTOCOL 100
#define INVALID_PROTOCOL 2

#if __BIG_ENDIAN__
# define htonll(x) (x)
# define ntohll(x) (x)
#else
# define htonll(x) (((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
# define ntohll(x) (((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#endif
struct connect_request {
	uint64_t protocol_id;
	uint32_t action;
	uint32_t transaction_id;
} __attribute__((packed));

struct connect_response {
	uint32_t action;
	uint32_t transaction_id;
	uint64_t connection_id;
} __attribute__((packed));

int getTracker(struct bencode_module*, struct tracker_properties*);

uint32_t buffer_to_u32(char**);

#endif
