// Implementation of xoshiro256plusplus based on
// https://prng.di.unimi.it/xoshiro256plusplus.c

#include "rng.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static inline uint64_t rotl(const uint64_t x, int k) {
  return (x << k) | (x >> (64 - k));
}

// Calling this function is equivalent to calling rng_next 2^128 times.
//
// Every new thread receives a copy of the main state, so calling `next` would
// return the same result.
// To prevent this the parent thread must call `jump` to leave a gap big enough
// that values returned from `rng_next` don't overlap.
static void jump(RNGState* state) {
  static const uint64_t JUMP[] = {0x180ec6d33cfd0aba, 0xd5a61266f0c9392c,
                                  0xa9582618e03fc9aa, 0x39abdc4529b1661c};

  uint64_t s0 = 0;
  uint64_t s1 = 0;
  uint64_t s2 = 0;
  uint64_t s3 = 0;

  for (uintptr_t i = 0; i < (sizeof JUMP / sizeof *JUMP); i++) {
    for (int b = 0; b < 64; b++) {
      if (JUMP[i] & UINT64_C(1) << b) {
        s0 ^= state->data[0];
        s1 ^= state->data[1];
        s2 ^= state->data[2];
        s3 ^= state->data[3];
      }

      rng_next(state);
    }
  }

  state->data[0] = s0;
  state->data[1] = s1;
  state->data[2] = s2;
  state->data[3] = s3;
}

// Returns the next random value for the given state.
uint64_t rng_next(RNGState* state) {
  const uint64_t result =
      rotl(state->data[0] + state->data[3], 23) + state->data[0];

  const uint64_t tmp = state->data[1] << 17;

  state->data[2] ^= state->data[0];
  state->data[3] ^= state->data[1];
  state->data[1] ^= state->data[2];
  state->data[0] ^= state->data[3];

  state->data[2] ^= tmp;

  state->data[3] = rotl(state->data[3], 45);

  return result;
}

// Creates a new `RNGState` with the provided seed.
RNGState* rng_new_main_state(uint64_t seed) {
  RNGState* state = malloc(sizeof(RNGState));

  // To avoid complications, we just repeat the seed over the 4 ints of state.
  for (int i = 0; i < 4; ++i) {
    state->data[i] = seed;
  }

  return state;
}

// Creates a new `RNGState` for a thread based on the current main thread
// `RNGState`.
RNGState* rng_new_thread_state(RNGState* mainState) {
  RNGState* state = malloc(sizeof(RNGState));
  memcpy(state, mainState, sizeof(RNGState));

  // Leave a gap in the mainState so that it doesn't overlap with the state in
  // the new thread.
  jump(mainState);

  return state;
}

// Cleans up RNGState resources.
void rng_drop_state(RNGState* state) {
  free(state);
}
