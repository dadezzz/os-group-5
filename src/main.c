#include <stdint.h>
#include "lib/config.h"
#include "lib/resources.h"
#include "lib/result.h"
#include "lib/rng.h"

int main() {
  Result result;

  Config* config = config_new();
  result = config_load(config);
  if (result != RESULT_OK) {
    config_drop(config);
    return result;
  }

  Resources* resources = resources_new();
  result = resources_load(config->resources_file, resources);
  if (result != RESULT_OK) {
    resources_drop(resources);
    config_drop(config);
    return result;
  }

  // load menu file

  // load resources file

  RNGState* rng_state = rng_new_main_state((uint64_t)config->random_seed);

  // Cleanup.
  rng_drop_state(rng_state);
  resources_drop(resources);
  config_drop(config);

  return 0;
}
