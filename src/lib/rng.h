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

RNGState* rng_new_main_state(uint64_t seed);

RNGState* rng_new_thread_state(RNGState* mainState);

void rng_drop_state(RNGState* state);

#endif
