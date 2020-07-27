#ifndef MEMORY_CHUNCK_H
#define MEMORY_CHUNCK_H

#include <stdint.h>
#include <config.h>

#include <wpp/net/ip/socket/buffers.hpp>

#pragma pack(push)
class memory_chunk : public wpp::net::ip::socket::buffers::buffer_data
{
public:
	bool initialized;
	uint16_t current_code;
	uint32_t bytes_received;

public:
	void set_total_size(uint32_t _size);

	memory_chunk(size_t size);
};
#pragma pack(pop)

#endif
