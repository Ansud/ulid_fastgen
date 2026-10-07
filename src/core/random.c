#include <stdint.h>
#include <stdalign.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

#include "random.h"

// Align to the cache line, otherwise it can be interconnected with
// another global state and it will be painful
static alignas(64) _Atomic uint32_t generator_state;

static uint64_t get_cpu_counter(void)
{
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
	unsigned int lo, hi;
	__asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
	return ((uint64_t)hi << 32) | lo;
#elif defined(__aarch64__) || defined(_M_ARM64)
	// This is ENOUGH FOR ME. I mix the counter once in ms or even more rare,
	// thus it will change between calls even on slow systems
	uint64_t tsc;
	__asm__ __volatile__("mrs %0, cntvct_el0" : "=r"(tsc));
	return tsc;
#else
#error "This architecture is not supported yet"
#endif
}

uint32_t xorshift_round(uint32_t value)
{
	value ^= value << 13;
	value ^= value >> 17;
	value ^= value << 5;

	return value;
}

uint32_t get_random_32(void)
{
	// If the generator_state is zero - it will be initialized here
	// Note: relaxed is more than enough, just tell the compiler not to optimize this variable access
	uint32_t new_state = atomic_load_explicit(&generator_state, memory_order_relaxed);
	const uint64_t tsc = get_cpu_counter();

	// Mix some more randomness to existing state. As i told - it is not real good RNG, it is nice to
	// get number from time to time, once in millisecond for example.
	new_state ^= (uint32_t)(tsc ^ (tsc >> 32));

	// It is a bit wrong attempt, but it is very rare in real world, so
	// need to write something to state
	if (new_state == 0) {
		new_state = 0x22021980;
	}

	new_state = xorshift_round(new_state);

	// If one of the threads destroy state - it is ok, random variations will be still random
	// But we need to set the state atomically
	atomic_store_explicit(&generator_state, new_state, memory_order_relaxed);

	return new_state;
}

uint16_t get_nonzero_random_16(uint32_t *state)
{
	// Two main ideas of this function:
	// 1. No any additional randomness added
	// 2. The state mutated in place without any atomic op
	//
	// And please note - NO ANY POINTER CHECKS IN RELEASE, BE CAREFUL
	//
#ifdef ULID_FASTGEN_DEBUG
	if (state == NULL) {
		fputs("get_nonzero_random_8: NULL state", stderr);
		abort();
	}
#endif

	const uint32_t new_state = xorshift_round(*state);
	*state					 = new_state;

	// Wrap to 16 bits
	const uint16_t out = (uint16_t)(new_state ^ (new_state >> 16));
	// No zeroes in return
	return (out & 0x7FFF) + 1;
}
