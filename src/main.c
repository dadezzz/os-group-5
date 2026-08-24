#include <bits/time.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "lib/config.h"
#include "lib/data/dishes/dishes.h"
#include "lib/data/resources.h"
#include "lib/result.h"
#include "lib/state/restaurant.h"
#include "lib/timer.h"
#include "lib/vec.h"

static double timespec_difference(struct timespec a, struct timespec b) {
  return (a.tv_sec - b.tv_sec) + (a.tv_nsec + b.tv_nsec) / 1e9;
}

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
    result = dishes_load(config.menu_file, &dishes, &resources);
  }

  Timer timer;
  timer_init(&timer);

  Restaurant restaurant;
  if (result == RESULT_OK) {
    restaurant_init(&restaurant, &timer, config.random_seed,
                    config.max_customers, &resources, &dishes);
  }

  if (result == RESULT_OK) {
    result = restaurant_spawn_cooks(&restaurant, config.num_cooks);
  }

  if (result == RESULT_OK) {
    result = restaurant_spawn_waiters(&restaurant, config.num_waiters);
  }

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  struct timespec next_status_at = now;
  struct timespec next_tick_at = now;
  int next_customer_at = 0;
  unsigned int spawned_customers = 0;
  while (result == RESULT_OK && (spawned_customers < config.total_customers ||
                                 !restaurant_is_empty(&restaurant))) {
    if (timer_get(&timer) >= next_customer_at &&
        spawned_customers < config.total_customers) {
      result = restaurant_spawn_customer(&restaurant);

      if (result == RESULT_RESTAURANT_FULL) {
        // Do nothing and retry on the next cycle.
        result = RESULT_OK;
      } else if (result == RESULT_OK) {
        ++spawned_customers;
      }
    }

    if (timespec_difference(now, next_tick_at) > 0) {
      timer_tick(&timer);
      next_tick_at = now;
      next_tick_at.tv_nsec += (long)(1e9 * config.game_speed);
    }

    if (timespec_difference(now, next_status_at) > 0) {
      // Print status.
      // next_status_at = time_now + config.print_status_interval
    }

    usleep(1000);
  }

  // Cleanup.

  restaurant_drop(&restaurant);
  timer_drop(&timer);
  vec_drop(&dishes, dish_drop);
  vec_drop(&resources, resource_drop);
  config_drop(&config);

  return (int)result;
}
