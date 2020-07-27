#pragma once
#include <stdint.h>

#pragma pack(1)

struct packet_subroute_header {
	uint16_t	full_size;// size doesnt contais this 2 bytes.
	uint8_t		type;
	uint32_t	packet_index;
	uint64_t	checksun;
	uint8_t* get_payload_ptr() {
		return ((uint8_t*)(this)) + sizeof(packet_subroute_header);
	}
};

#pragma pack()