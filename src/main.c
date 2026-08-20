#include <stdint.h>

#include "lib/config.h"
#include "lib/dishes/dishes.h"
#include "lib/resources.h"
#include "lib/result.h"
#include "lib/rng.h"
#include "lib/vec.h"

int main() {
  Result result;

  Config config;
  result = config_load(&config);
  if (result != RESULT_OK) {
    config_drop(&config);
    return result;
  }

  Vec resources;
  result = resources_load(config.resources_file, &resources);
  if (result != RESULT_OK) {
    vec_drop(&resources, resource_drop);
    config_drop(&config);
    return result;
  }

  Vec dishes;
  result = dishes_load(config.menu_file, &dishes);
  if (result != RESULT_OK) {
  vec_drop(&dishes, dish_drop);
  vec_drop(&resources, resource_drop);
    config_drop(&config);
  }

  RNGState* rng_state = rng_new_main_state((uint64_t)config.random_seed);

  // Cleanup.
  rng_drop_state(rng_state);
  vec_drop(&dishes, dish_drop);
  vec_drop(&resources, resource_drop);
  config_drop(&config);

  return 0;
}
