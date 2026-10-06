#ifndef ULID_FASTGEN_RANDOM_H
#define ULID_FASTGEN_RANDOM_H

#include <stdint.h>

// Execute one round of xorshift
uint32_t xorshift_round(uint32_t value);

// Get random value with a bit of entropy from CPU, stores own state in variable (not safe way!)
uint32_t get_random_32(void);

// The special 16bit version which has no any atomics inside
uint16_t get_nonzero_random_16(uint32_t *state);

#endif
