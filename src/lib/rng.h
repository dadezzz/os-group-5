#ifndef RNG_H
#define RNG_H

#include <stdint.h>

// Holds the state of the RNG.
//
// This struct doesn't support concurrent access and thus must not be shared
// between threads. If you want to pass an RNG to a new thread use
// `rng_new_thread_state`.
typedef struct RNGState {
  uint64_t data[4];
} RNGState;

uint64_t rng_next(RNGState* state);

void rng_init_main(RNGState* state, uint64_t seed);

void rng_init_thread(RNGState* main_state, RNGState* thread_state);

uint64_t rng_next_range(RNGState* state, uint64_t min, uint64_t max);

#endif
