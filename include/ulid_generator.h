/**
 * The ULID generator, i think it is nice for PK for databases with *mostly*
 * unpredictable primary keys but sorted by time of creation, which is definitely
 * good for DB indexing. I used plain C, because it is much clearer than any other
 * language and may be used literally everywhere.
 *
 * The magic *node* is snowflake like idea, when different generators on different
 * nodes use own ID to disable heavy sync. The node in the end, so the different
 * generators (pg processes for example) can fit into one index node.
 *
 * So, the ULID is 128 bits of data.
 * 48 bits - timestamp in milliseconds from epoch, more than enough
 * 56 bits - some incremented value
 * 16 bits - node id
 * 8 bits - shard id
 *
 * It is not cryptographically random values. It is just a bit hard to guess but
 * it increased monotonically.
 *
 * To guess first item - you need to find initial 32bit - you can't in adequate time
 * To guess next item - you need to scan 16bit, much easier, but for EVERY next item become harder
 *
 * Otherwise much more predictable than pure random 64bit ULID second part, but can be used
 * with sharded services (each service has own node value) and guarantee that they
 * will not intersect generated ULIDs.
 */
#ifndef ULID_FASTGEN_ULID_GENERATOR_H
#define ULID_FASTGEN_ULID_GENERATOR_H

#include <stdint.h>
#include <stdalign.h>

typedef struct _ulid_t {
	uint64_t a, b;
} ulid_t;

typedef struct _ulid_aligned_state_t {
	alignas(64)
#ifdef __clang__
		_Atomic
#endif
		ulid_t f;
} ulid_aligned_state_t;

typedef enum _ulid_errors {
	GET_TIMESTAMP_FAILED = -1,
	INVALID_PARAMETERS	 = -2,
	MEMORY_NOT_ALIGNED	 = -3,
	TIME_DRIFT_TOO_LARGE = -4,
} ulid_errors_t;

#ifdef __cplusplus
extern "C" {
#endif

// You can wrap this in any way you want, here is generic with following parameters

// The generic function used for all wrappers
// atomic_storage -> The memory line(s) aligned to 64 bytes to store last calculated ULID.
//                   The count of those line(s) depends of shards.
//                   The most easy way - allocate 256x64 memory buffer.
//**************************************************************************************************************
//                   THERE IS NO ANY CHECKS INSIDE. SO YOU PASS 64 LINES AND SHARD 128 - YOUR MEMORY DESTROYED.
//**************************************************************************************************************
// out_buffer -> output to array of ULIDs
// count -> how much ULIDs need to generate
// node -> the node id which generate ULID
// shard -> the cpu number (or thread id or something else) which generate ULID (for cache bounce prevention)
// return count of generated ULIDs or an error (<0)
int32_t ulid_generate_core(ulid_aligned_state_t *atomic_storage, ulid_t *out_buffer, uint16_t count, uint16_t node,
						   uint8_t shard);

// Use preallocated memory
int32_t ulid_generate(ulid_t *out_buffer, uint16_t count, uint16_t node, uint8_t shard);

// Only one shard and node (for non distributed non heavy loads, memory provides automatically)
int32_t ulid_generate_simple(ulid_t *out_buffer, uint16_t count);

// Only one node, but prevent cache line bouncing (nice for single instance with multi cpu, like nginx router)
int32_t ulid_generate_unbounced(ulid_t *out_buffer, uint16_t count, uint8_t shard);

#ifdef __cplusplus
}
#endif

#endif
