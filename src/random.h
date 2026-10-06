#ifndef ULID_FASTGEN_RANDOM_H
#define ULID_FASTGEN_RANDOM_H

#include <stdint.h>

uint32_t get_random_32(void);

// The special 16bit version which has no any atomics inside
uint16_t get_nonzero_random_16(uint32_t *state);

#endif
