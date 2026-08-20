#include <stddef.h>
#include <stdint.h>

#include "lib/config.h"
#include "lib/dishes/dishes.h"
#include "lib/resources.h"
#include "lib/result.h"
#include "lib/rng.h"
#include "lib/threads/cook.h"
#include "lib/threads/waiter.h"
#include "lib/vec.h"

int main() {
  Result result = RESULT_OK;

  Config config;
  result = config_load(&config);

  Vec resources;
  if (result == RESULT_OK) {
    vec_init(&resources, sizeof(Resource));
    result = resources_load(config.resources_file, &resources);
  }

  Vec dishes;
  if (result == RESULT_OK) {
    vec_init(&dishes, sizeof(Dish));
    result = dishes_load(config.menu_file, &dishes);
  }

  RNGState* rng_state;
  if (result == RESULT_OK) {
    rng_state = rng_new_main_state((uint64_t)config.random_seed);
  }

  Vec cooks;
  if (result == RESULT_OK) {
    vec_init(&cooks, sizeof(Cook));
    for (int i = 0; i < config.num_cooks; i++) {
      Cook cook;
      cook_init(&cook);
      vec_push(&cooks, &cook);
    }
  }

  Vec waiters;
  if (result == RESULT_OK) {
    vec_init(&waiters, sizeof(Waiter));
    for (int i = 0; i < config.num_waiters; i++) {
      Waiter waiter;
      waiter_init(&waiter);
      vec_push(&waiters, &waiter);
    }
  }

  // Cleanup.
  for (size_t i = 0; i < waiters.length; i++) {
    Result local_result = waiter_drop(vec_at(&waiters, i));
    if (local_result != RESULT_OK) {
      result = local_result;
    }
  }
  vec_drop(&waiters, nullptr);

  for (size_t i = 0; i < cooks.length; i++) {
    Result local_result = cook_drop(vec_at(&cooks, i));
    if (local_result != RESULT_OK) {
      result = local_result;
    }
  }
  vec_drop(&cooks, nullptr);

  rng_drop_state(rng_state);
  vec_drop(&dishes, dish_drop);
  vec_drop(&resources, resource_drop);
  config_drop(&config);

  return result;
}
