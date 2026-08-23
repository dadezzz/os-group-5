#include "restaurant.h"

#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>

#include "../result.h"
#include "../rng.h"
#include "../threads/cook.h"
#include "../threads/customer.h"
#include "../threads/waiter.h"
#include "../timer.h"
#include "../vec.h"
#include "dish-ticket.h"
#include "kitchen.h"
#include "order.h"

void restaurant_init(Restaurant* restaurant,
                     Timer* timer,
                     unsigned int rng_seed,
                     unsigned int num_seats,
                     Vec* resources,  // Vec<Resource>
                     Vec* dishes      // Vec<Dish>
) {
  restaurant->score = 0;
  restaurant->timer = timer;
  restaurant->dishes = dishes;
  atomic_init(&restaurant->is_closing, false);
  kitchen_init(&restaurant->kitchen, resources);

  rng_init_main(&restaurant->rng, rng_seed);

  vec_init(&restaurant->cooks, sizeof(Cook));
  vec_init(&restaurant->waiters, sizeof(Waiter));
  vec_init(&restaurant->customers, sizeof(Customer));

  restaurant->num_seats = num_seats;
  vec_reserve(&restaurant->customers, num_seats);

  pthread_mutex_init(&restaurant->mtx, nullptr);
}

Result restaurant_spawn_cooks(Restaurant* restaurant, size_t quantity) {
  Result result = vec_reserve(&restaurant->cooks, quantity);
  if (result != RESULT_OK) {
    return result;
  }

  restaurant->cooks.length = quantity;

  for (size_t i = 0; result == RESULT_OK && i < restaurant->cooks.length; ++i) {
    result = cook_init(vec_at(&restaurant->cooks, i), restaurant);
  }

  return result;
}

Result restaurant_spawn_waiters(Restaurant* restaurant, size_t quantity) {
  Result result = vec_reserve(&restaurant->waiters, quantity);
  if (result != RESULT_OK) {
    return result;
  }

  restaurant->waiters.length = quantity;

  for (size_t i = 0; result == RESULT_OK && i < restaurant->waiters.length;
       ++i) {
    result = waiter_init(vec_at(&restaurant->waiters, i), restaurant);
  }

  return result;
}

Result restaurant_spawn_customer(Restaurant* restaurant) {
  Customer* customer = nullptr;
  Result result = RESULT_OK;

  if (restaurant->customers.length == restaurant->num_seats) {
    // Try to find a free place in the seats.
    for (size_t i = 0; i < restaurant->customers.length; ++i) {
      Customer* old_customer = vec_at(&restaurant->customers, i);

      if (old_customer->has_left) {
        result = customer_drop(old_customer);
        if (result != RESULT_OK) {
          return result;
        }

        customer = old_customer;
        break;
      }
    }

    if (customer == nullptr) {
      return RESULT_RESTAURANT_FULL;
    }
  } else {
    customer = vec_at(&restaurant->customers, restaurant->customers.length);
    ++restaurant->customers.length;
  }

  result = customer_init(customer, restaurant);
  return result;
}

bool restaurant_is_closing(Restaurant* restaurant) {
  return atomic_load(&restaurant->is_closing);
}

bool restaurant_is_empty(Restaurant* restaurant) {
  for (size_t i = 0; i < restaurant->customers.length; ++i) {
    Customer* customer = vec_at(&restaurant->customers, i);
    if (!customer->has_left) {
      return false;
    }
  }

  return true;
}

void restaurant_drop(Restaurant* restaurant) {
  atomic_store(&restaurant->is_closing, true);

  Result result = RESULT_OK;

  // Drop cooks before waiters because the cook might still try to push to the
  // waiter's ready queue.
  for (size_t i = 0; i < restaurant->cooks.length; ++i) {
    Result local_result = cook_drop(vec_at(&restaurant->cooks, i));
    if (result == RESULT_OK) {
      result = local_result;
    }
  }
  vec_drop(&restaurant->cooks, nullptr);

  for (size_t i = 0; i < restaurant->waiters.length; ++i) {
    Result local_result = waiter_drop(vec_at(&restaurant->waiters, i));
    if (result == RESULT_OK) {
      result = local_result;
    }
  }
  vec_drop(&restaurant->waiters, nullptr);

  for (size_t i = 0; i < restaurant->customers.length; ++i) {
    Result local_result = customer_drop(vec_at(&restaurant->customers, i));

    // Don't overwrite previous failures;
    if (result == RESULT_OK) {
      result = local_result;
    }
  }
  vec_drop(&restaurant->customers, nullptr);

  pthread_mutex_destroy(&restaurant->mtx);
  kitchen_drop(&restaurant->kitchen);
}
