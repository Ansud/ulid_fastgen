#include <stdint.h>

#include <ulid_generator.h>
#include "../core/random.h"

// If you have multicpu arch and real cpu nodes you can get cache bouncing
// if all nodes will generate ULIDs simultaneously with one global variable
// So, divide variables for CPUs. I can't do it automatically unfortunately, because
// ARM do not like reading some registries
#define MAX_ULID_SHARD_COUNT 256

static ulid_aligned_state_t sharded_state[MAX_ULID_SHARD_COUNT];

static void fill_random(ulid_t *out_buffer, uint16_t count, uint32_t mask)
{
	// Random enough, from my point of view
	uint32_t random_state = (uint32_t)((out_buffer[0].a >> 32) ^ (out_buffer[0].a & 0xFFFFFFFF));

	// Fill node and shard with something, hopefully not zeroes
	for (uint32_t i = 0; i < count; i++) {
		random_state = xorshift_round(random_state);
		// Move to the node + shard. But they are swapped (endianess), so they are in upper bits
		// I.e. by bytes in register: shard - node low - node high - other stuff
		uint64_t update = (uint64_t)random_state;
		update &= mask;
		update <<= 32;

		out_buffer[i].b |= update;
	}
}

// Only one shard and node (for non distributed non heavy loads, memory provides automatically)
int32_t ulid_generate_simple(ulid_t *out_buffer, uint16_t count)
{
	int32_t result = ulid_generate_core(sharded_state, out_buffer, count, 0, 0);

	if (result != count) {
		return result;
	}

	fill_random(out_buffer, count, 0xFFFFFF00);

	return result;
}

// Only one node, but prevent cache line bouncing (nice for single instance with multi cpu, like nginx router)
int32_t ulid_generate_unbounced(ulid_t *out_buffer, uint16_t count, uint8_t shard)
{
	int32_t result = ulid_generate_core(sharded_state, out_buffer, count, 0, shard);

	if (result != count) {
		return result;
	}

	fill_random(out_buffer, count, 0x00FFFF00);

	return result;
}

// Just use preallocated memory
int32_t ulid_generate(ulid_t *out_buffer, uint16_t count, uint16_t node, uint8_t shard)
{
	return ulid_generate_core(sharded_state, out_buffer, count, node, shard);
}
