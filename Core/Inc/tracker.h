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

struct announce_request {
	uint64_t connection_id;
	uint32_t action;
	uint32_t transaction_id;
	char info_hash[20];
	char peer_id[20];
	uint64_t downloaded;
	uint64_t left;
	uint64_t uploaded;
	uint32_t event;
	uint32_t ip_address;
	uint32_t key;
	uint32_t num_want;
	uint16_t port;
} __attribute__((packed));

int getTracker(struct bencode_module*, struct tracker_properties*);

uint32_t buffer_to_u32(char*);
uint64_t buffer_to_u64(char*);

#endif
