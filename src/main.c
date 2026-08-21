#include <stddef.h>
#include <stdint.h>

#include "lib/config.h"
#include "lib/data/dishes/dishes.h"
#include "lib/data/resources.h"
#include "lib/result.h"
#include "lib/rng.h"
#include "lib/state/restaurant.h"
#include "lib/threads/cook.h"
#include "lib/threads/customer.h"
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
    result = dishes_load(config.menu_file, &dishes, &resources);
  }

  Restaurant restaurant;
  restaurant_init(&restaurant, config.max_customers);

  RNGState rng_state;
  if (result == RESULT_OK) {
    rng_state_init_main(&rng_state, (uint64_t)config.random_seed);
  }

  Vec waiters;
  if (result == RESULT_OK) {
    vec_init(&waiters, sizeof(Waiter));
    // Reserve enough space for all waiters to make sure that the array doesn't
    // get moved in another memory location by a realloc.
    waiters.length = config.num_waiters;
    result = vec_reserve(&waiters, waiters.length);
  }
  for (size_t i = 0; result == RESULT_OK && i < waiters.length; ++i) {
    // Initialize all waiters in place.
    Waiter* waiter = vec_at(&waiters, i);
    result = waiter_init(waiter, &rng_state);
  }

  Vec cooks;
  if (result == RESULT_OK) {
    vec_init(&cooks, sizeof(Cook));
    cooks.length = config.num_cooks;
    result = vec_reserve(&cooks, cooks.length);
  }
  for (size_t i = 0; result == RESULT_OK && i < cooks.length; ++i) {
    Cook* cook = vec_at(&cooks, i);
    result = cook_init(cook, &rng_state);
  }

  Vec customers;
  if (result == RESULT_OK) {
    vec_init(&customers, sizeof(Customer));
    customers.length = config.total_customers;
    result = vec_reserve(&customers, customers.length);
  }
  for (size_t i = 0; result == RESULT_OK && i < customers.length; ++i) {
    Customer* customer = vec_at(&customers, i);
    result = customer_init(customer, &rng_state, &restaurant.seats);
  }

  // Cleanup.

  for (size_t i = 0; i < customers.length; ++i) {
    Result local_result = customer_drop(vec_at(&customers, i));
    // Don't override the previous value that made the program fail.
    // Subsequent failures may be a consequence of the first.
    if (result == RESULT_OK) {
      result = local_result;
    }
  }
  vec_drop(&customers, nullptr);

  for (size_t i = 0; i < cooks.length; ++i) {
    Result local_result = cook_drop(vec_at(&cooks, i));
    if (result == RESULT_OK) {
      result = local_result;
    }
  }
  vec_drop(&cooks, nullptr);

  // Waiters must be dropped after cooks have finished because a cook might
  // still hold references to the waiter in the DishTicket.
  for (size_t i = 0; i < waiters.length; ++i) {
    Result local_result = waiter_drop(vec_at(&waiters, i));
    if (result == RESULT_OK) {
      result = local_result;
    }
  }
  vec_drop(&waiters, nullptr);

  restaurant_drop(&restaurant);
  vec_drop(&dishes, dish_drop);
  vec_drop(&resources, resource_drop);
  config_drop(&config);

  return result;
}
