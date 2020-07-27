#include "memory_chunk.h"


memory_chunk::memory_chunk(size_t size) :
	buffer_data(size)
{
	initialized = false;
	current_code = 0;
	bytes_received = 0;
}

void memory_chunk::set_total_size(uint32_t _size)
{
	swap_size(_size);
}