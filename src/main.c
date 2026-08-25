#include <errno.h>
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
#include "lib/fifo-queue.h"
#include "lib/result.h"
#include "lib/rng.h"
#include "lib/sigusr1.h"
#include "lib/state/kitchen.h"
#include "lib/state/restaurant.h"
#include "lib/threads/cook.h"
#include "lib/threads/customer.h"
#include "lib/vec.h"

const unsigned int PRINT_STATUS_INTERVAL = 5;  // in seconds

static double timespec_difference(struct timespec a, struct timespec b) {
  return (a.tv_sec - b.tv_sec) + (a.tv_nsec - b.tv_nsec) / 1e9;
}

static void timespec_add(struct timespec* t,
                         unsigned int units,
                         double game_speed) {
  double interval = 1e9 * units / game_speed;
  long sec = (long)(interval / 1e9);
  long nsec = ((long)interval % (long)1e9);

  t->tv_sec += sec;
  t->tv_nsec += nsec;

  // Riporto se la somma sfora il miliardo di nanosecondi
  if (t->tv_nsec >= 1000000000L) {
    t->tv_sec += 1;
    t->tv_nsec -= 1000000000L;
  }
}

static void status_print(Config* config,
                         Restaurant* restaurant,
                         bool extended_print) {
  int current_customers_count = 0;
  int unserved_customers_count = 0;
  for (FIFOQueueNode* node = restaurant->customers.head; node != nullptr;
       node = node->next) {
    Customer* customer = node->value;

    if (!customer->has_left) {
      current_customers_count++;
    }

    if (customer->has_left &&
        customer->dishes_served < customer->order_dishes.length) {
      unserved_customers_count++;
    }
  }

  fprintf(stdout, "--- RESTAURANT STATUS ---\n");
  fprintf(stdout, "Current score:   %d\n", atomic_load(&restaurant->score));
  fprintf(stdout, "Customers:\n");
  fprintf(stdout, "--- currently in restaurant:   %d\n",
          current_customers_count);
  fprintf(stdout, "--- left unserved:   %d\n", unserved_customers_count);
  fprintf(stdout, "--- progress (spawned / total):  %lu / %d\n",
          restaurant->customers.lenght, config->total_customers);

  if (extended_print) {
    fprintf(stdout, "Lenght of cooks' dishes queues:\n");
    for (size_t i = 0; i < restaurant->cooks.length; i++) {
      Cook* cook = vec_at(&restaurant->cooks, i);

      fprintf(stdout, "--- cook %lu:   %lu\n", i, cook->dish_tickets.lenght);
    }

    fprintf(stdout, "Current availability of kitchen resources:\n");
    for (size_t i = 0; i < restaurant->kitchen.resources.length; i++) {
      KitchenResource* kitchen_resources =
          vec_at(&restaurant->kitchen.resources, i);

      fprintf(stdout, "--- %s:   %b\n", kitchen_resources->resource->name,
              atomic_load(&kitchen_resources->available));
    }
  }
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

  Restaurant restaurant;
  if (result == RESULT_OK) {
    result = restaurant_init(&restaurant, &config, &resources, &dishes);
  }

  if (result == RESULT_OK) {
    int tmp = mkdir("/tmp", 0775);
    if (tmp != 0 && errno != EEXIST) {
      result = RESULT_MKDIR_FAILED;
    }
  }

  if (result == RESULT_OK) {
    FILE* file = fopen("/tmp/restaurant.pid", "w");
    if (file == nullptr) {
      result = RESULT_FILE_OPENING_FAILED;
    } else {
      fprintf(file, "%d\n", getpid());
      fclose(file);
    }
  }

  if (result == RESULT_OK) {
    result = sigusr1_register_handler();
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

  unsigned int spawned_customers = 0;
  unsigned int next_customer_ticks = rng_next_range(&restaurant.rng, 0, 50);
  struct timespec next_customer_at = now;
  timespec_add(&next_customer_at, next_customer_ticks, config.game_speed);

  while (result == RESULT_OK && !restaurant_has_finished(&restaurant)) {
    clock_gettime(CLOCK_MONOTONIC, &now);

    if (timespec_difference(now, next_customer_at) > 0 &&
        spawned_customers < config.total_customers) {
      result = restaurant_spawn_customer(&restaurant);

      if (result == RESULT_RESTAURANT_FULL) {
        // Do nothing and retry on the next cycle.
        result = RESULT_OK;
      } else if (result == RESULT_OK) {
        ++spawned_customers;
        next_customer_ticks = rng_next_range(&restaurant.rng, 0, 50);
        next_customer_at = now;
        timespec_add(&next_customer_at, next_customer_ticks, config.game_speed);
      }
    }

    if (sigusr1_get_raised()) {
      status_print(&config, &restaurant, true);
      // Set to false to avoid checking again on next loop.
      sigusr1_set_raised(false);
    }

    if (timespec_difference(now, next_status_at) > 0) {
      status_print(&config, &restaurant, false);
      next_status_at = now;
      next_status_at.tv_sec += PRINT_STATUS_INTERVAL;
    }

    // Loop every hundredth of a second.
    usleep((unsigned int)1e4);
  }

  // Cleanup.

  restaurant_drop(&restaurant);
  vec_drop(&dishes, dish_drop);
  vec_drop(&resources, resource_drop);
  config_drop(&config);

  return (int)result;
}
