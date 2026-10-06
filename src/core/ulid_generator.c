#include <stdatomic.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdalign.h>
#include <time.h>

#include <ulid_generator.h>

#include "random.h"

#if (defined(__GNUC__) || defined(__clang__)) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define BSWAP64(x) __builtin_bswap64(x)
#elif defined(_MSC_VER)
#define BSWAP64(x) _byteswap_uint64(x)
#else
#include <endian.h>
#define BSWAP64(x) htobe64(x)
#endif

static uint64_t get_ms_timestamp()
{
	struct timespec now;

	// Any idea what to do?
	if (timespec_get(&now, TIME_UTC) != TIME_UTC) {
		// Let it be problem of caller
		return 0;
	}

	return (now.tv_sec * 1000 + now.tv_nsec / 1000000);
}

int32_t ulid_generate_core(ulid_aligned_state_t *atomic_storage, ulid_t *out_buffer, uint16_t count, uint16_t node,
						   uint8_t shard)
{
	const uint64_t timestamp = get_ms_timestamp();

	if (!timestamp) {
#ifdef DEBUG
		// Panic is good idea in debug mode, otherwise - sorry :)
		fputs("get_ms_timestamp failed to return value, can not continue\n", stderr);
		abort();
#endif
		// The failed timestamp function is the caller problem
		return GET_TIMESTAMP_FAILED;
	}

	if (!out_buffer || !atomic_storage) {
		return INVALID_PARAMETERS;
	}

	if (((uint64_t)atomic_storage) & 0x3F) {
		return MEMORY_NOT_ALIGNED;
	}

	const uint64_t timestamp_with_space = timestamp << 16;
	ulid_t current						= atomic_load_explicit(&atomic_storage[shard].f, memory_order_relaxed);
	// The start of the sequence
	uint64_t start;
	// It is separate state, not related to global one. So, initialize it with
	// something plus or minus random (check in the loop)
	uint32_t state;
	ulid_t next;

	while (1) {
		uint64_t next_ts, next_random;

		// Only time may differ, so drop the random part and substract
		const uint64_t diff = (current.a & 0xFFFFFFFFFFFF0000) - timestamp_with_space;

		// And here are three variants:
		// 1. Nothing changed - diff == 0, it is good
		// 2. current_ts > timestamp_with_space - (diff >> 63) is not 0 and need to update stored timestamp
		// 3. current_ts < timestamp_with_space - diff is positive integer, do not touch timestamp
		//                                       if the diff is small enough, like 5 secs.

		// Just need to change timestamp?
		if (!current.a || (diff & ((uint64_t)1 << 63))) {
			next_ts		= timestamp_with_space;
			next_random = get_random_32();
		} else {
			// We can survive ~4 seconds.
			// btw, last 16 bits is random, clean them and do not check of course
			if (diff & 0xFFFFFFFFF0000000) {
#ifdef DEBUG
				// No way to continue. Sorry.
				fprintf(stderr, "Oops: time drift is too large in the past. [0x%016llx]\n", diff);
				abort();
#endif
				// Let it be problem of caller
				return TIME_DRIFT_TOO_LARGE;
			}

			// Update values, current may be changed in previous write attempt
			next_ts		= current.a & 0xFFFFFFFFFFFF0000;
			next_random = ((current.a & 0xFFFF) << 40) | current.b >> 24;
		}

		start = next_random;

		state = (uint32_t)(next_random ^ next_ts);
		// Do not let it be zero.
		state = (state & 0x7FFFFFFF) + 1;

		// Tradeoff - one timestamp per ALL generated ULIDs even if ms updated by OS.
		// other threads may update some values, not interested in. Bulk generation works in one ms.
		// Reserve count * maximum random for next step + some random value
		next_random += (uint64_t)count * 0x10000 + get_nonzero_random_16(&state);

		// Fill next values
		next.a = next_ts | ((next_random >> 40) & 0xFFFF);
		next.b = (next_random << 24) | (node << 8) | shard;

		// Set back calculated variable via CMPXCHG
		if (atomic_compare_exchange_weak_explicit(&atomic_storage[shard].f, &current, next, memory_order_release,
												  memory_order_relaxed)) {
			break;
		}
	}

	// Copy as two uint64 values via memcpy (i can't guarantee that user sent us aligned buffer)
	uint64_t temp_buffer[2];
	uint8_t *buffer = (uint8_t *)out_buffer;
	uint64_t ts		= next.a & 0xFFFFFFFFFFFF0000;

	for (int i = 0; i < count; i++) {
		temp_buffer[0] = BSWAP64(ts | ((start >> 40) & 0xFFFF));
		temp_buffer[1] = BSWAP64((start << 24) | (node << 8) | shard);

		memcpy(buffer, &temp_buffer[0], sizeof(ulid_t));
		buffer += sizeof(ulid_t);

		start += get_nonzero_random_16(&state);
	}

	return count;
}
