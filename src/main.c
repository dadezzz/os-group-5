#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "lib/config.h"
#include "lib/data/dishes/dishes.h"
#include "lib/data/resources.h"
#include "lib/result.h"
#include "lib/rng.h"
#include "lib/sigusr1.h"
#include "lib/state/restaurant.h"
#include "lib/status.h"
#include "lib/vec.h"

#define PRINT_STATUS_INTERVAL_TICKS 5000       // ~50 seconds at game speed 1
#define CUSTOMER_SPAWN_INTERVAL_MAX_TICKS 500  // in game ticks

static Result main_loop(Restaurant* restaurant) {
  int next_customer_ticks =
      rng_next_range(&restaurant->rng, 0, CUSTOMER_SPAWN_INTERVAL_MAX_TICKS);
  int next_status_ticks = PRINT_STATUS_INTERVAL_TICKS;

  Result result = RESULT_OK;

  while (!restaurant_has_finished(restaurant)) {
    if (next_customer_ticks <= 0 &&
        restaurant->spawned_customers < restaurant->config->total_customers) {
      result = restaurant_spawn_customer(restaurant);

      if (result != RESULT_RESTAURANT_FULL && result != RESULT_OK) {
        return result;
      }

      if (result == RESULT_OK) {
        next_customer_ticks = rng_next_range(&restaurant->rng, 0,
                                             CUSTOMER_SPAWN_INTERVAL_MAX_TICKS);
      }

      // Try to drop customers roughly at the same rate (or a bit faster) than
      // spawning customers.
      result = restaurant_try_drop_first_customer(restaurant);
      if (result != RESULT_OK) {
        return result;
      }
    }

    if (sigusr1_get_raised()) {
      status_print(restaurant, true);
      // Set to false to avoid checking again on next loop.
      sigusr1_set_raised(false);
    }

    if (next_status_ticks < 0) {
      status_print(restaurant, false);
      next_status_ticks = PRINT_STATUS_INTERVAL_TICKS;
    }

    --next_customer_ticks;
    --next_status_ticks;

    // Loop every hundredth of a second (for game speed = 1).
    usleep((unsigned int)(1e4 / restaurant->config->game_speed));
  }

  return result;
}

int main() {
  Result result = RESULT_OK;

  Config config;
  result = config_load(&config);
  if (result != RESULT_OK) {
    goto cleanup_config;
  }

  Vec resources;
  vec_init(&resources, sizeof(Resource));
  result = resources_load(config.resources_file, &resources);
  if (result != RESULT_OK) {
    goto cleanup_resources;
  }

  Vec dishes;
  vec_init(&dishes, sizeof(Dish));
  result = dishes_load(config.menu_file, &dishes, &resources);
  if (result != RESULT_OK) {
    goto cleanup_dishes;
  }

  Restaurant restaurant;
  result = restaurant_init(&restaurant, &config, &resources, &dishes);
  if (result != RESULT_OK) {
    goto cleanup_restaurant;
  }

  int tmp = mkdir("/tmp", 0775);
  if (tmp != 0 && errno != EEXIST) {
    result = RESULT_MKDIR_FAILED;
    goto cleanup_restaurant;
  }

  FILE* file = fopen("/tmp/restaurant.pid", "w");
  if (file == nullptr) {
    result = RESULT_FILE_OPENING_FAILED;
    goto cleanup_restaurant;
  }

  fprintf(file, "%d\n", getpid());
  fclose(file);

  result = sigusr1_register_handler();
  if (result != RESULT_OK) {
    goto cleanup_restaurant;
  }

  result = restaurant_spawn_cooks(&restaurant, config.num_cooks);
  if (result != RESULT_OK) {
    goto cleanup_restaurant;
  }

  result = restaurant_spawn_waiters(&restaurant, config.num_waiters);
  if (result != RESULT_OK) {
    goto cleanup_restaurant;
  }

  result = main_loop(&restaurant);

cleanup_restaurant:
  Result restaurant_result = restaurant_drop(&restaurant);
  if (restaurant_result != RESULT_OK) {
    fprintf(stderr, "Something went wrong during restaurant cleanup: %d\n",
            (int)restaurant_result);
  }
cleanup_dishes:
  vec_drop(&dishes, dish_drop);
cleanup_resources:
  vec_drop(&resources, resource_drop);
cleanup_config:
  config_drop(&config);

  if (result != RESULT_OK) {
    fprintf(stderr, "Something went wrong: %d\n", (int)result);
  }

  return (int)result;
}
