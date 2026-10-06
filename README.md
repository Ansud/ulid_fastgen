# ulid_fastgen

Fast generator of 128-bit time-ordered unique IDs. Plain C11, no dependencies.
One 16-byte CAS per batch of any size.

## ID layout

```
| 48 bits      | 56 bits | 16 bits | 8 bits |
| timestamp ms | counter | node    | shard  |
```

Bytes are stored big-endian, so IDs sort by time with plain `memcmp`.
An ID is 16 bytes and fits into a database `uuid` column as is.

## Guarantees

- **Unique** across nodes and shards by construction: node and shard are part
  of the ID, the counter never repeats within one (node, shard) in one process.
- **Strictly monotonic** within one (node, shard), under any concurrency,
  including bulk generation. This is more than UUIDv7 gives you: UUIDv7 is
  random within one millisecond.
- **Clock going backwards**: up to ~4 seconds the generator keeps the last
  known timestamp and stays monotonic. More than that — it returns an error
  instead of producing duplicates.

The trade-off: IDs are *less* random than UUIDv7. The next ID is the previous
one plus a random 16-bit step. Hard to guess, but not cryptographic. Do not
use the IDs as secrets.

## API

```c
#include <ulid_generator.h>

ulid_t ids[100];

// Easiest: one global generator, node/shard bytes filled with randomness
ulid_generate_simple(ids, 100);

// One generator per thread/CPU to avoid cache-line bouncing
ulid_generate_unbounced(ids, 100, cpu_id);

// Full control: distributed setups, node id + shard id
ulid_generate(ids, 100, node_id, shard_id);

// Bring your own state memory (shared, mapped, whatever)
// Memory must be 64-byte aligned, one 64-byte slot per shard you use.
static ulid_aligned_state_t state[256];
ulid_generate_core(state, ids, 100, node_id, shard_id);
```

All functions return the number of generated IDs, or a negative error
(`ulid_errors_t`): bad parameters, unaligned memory, failed clock, clock too
far in the past.

`shard` selects an independent state slot *and* goes into the ID. Use it to
keep hot threads on separate cache lines. `node` distinguishes machines or
processes: two generators with different node values can never collide.

## Requirements

128-bit atomics must be lock-free:

- x86-64: build with `-march=x86-64-v2` (or `-mcx16`). Clang inlines
  `cmpxchg16b`; GCC calls libatomic — correct, a bit slower.
- ARM64: works out of the box.

Do not use the state memory across processes if `atomic_is_lock_free`
is false: libatomic's mutex fallback is process-local.

## Build and test

```sh
cmake -B build -S . && cmake --build build
cmake -B build -S . -DFULID_BUILD_TESTS=ON && cmake --build build && ctest --test-dir build
```

Tests include a multi-threaded uniqueness check. Run it with
ThreadSanitizer enabled to check the lock-free paths.
