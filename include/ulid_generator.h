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
 * 64 bits - some incremented value
 * 16 bits - node id
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

typedef struct _ulid_t {
	uint64_t a, b;
} ulid_t;

#ifdef __cplusplus
extern "C" {
#endif

// You can wrap this in any way you want, here is generic with following parameters
// out_buffer -> output to array of uuids
// node -> the node which generate uuid
// count -> how much uuids need to generate
// return count of generated ULIDs
uint16_t ulid_generate(ulid_t *out_buffer, uint16_t node, uint16_t count);

#ifdef __cplusplus
}
#endif

// And some inline helpers
static __inline uint16_t ulid_generate_one(ulid_t *out, uint16_t node)
{
	return ulid_generate(out, node, 1);
}

#endif
