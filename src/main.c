#include <stdio.h>

#include "lib/rng.h"

int main(void) {
  RNGState* rng_state = rng_new_main_state(42);
  printf("%lu\n", rng_next(rng_state));

  rng_drop_state(rng_state);
  return 0;
}
