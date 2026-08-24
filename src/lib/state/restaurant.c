#include "restaurant.h"

#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdlib.h>

#include "../fifo-queue.h"
#include "../result.h"
#include "../rng.h"
#include "../threads/cook.h"
#include "../threads/customer.h"
#include "../threads/waiter.h"
#include "../timer.h"
#include "../vec.h"
#include "dish-ticket.h"
#include "kitchen.h"
#include "sink.h"

Result restaurant_init(Restaurant* restaurant,
                       Timer* timer,
                       unsigned int rng_seed,
                       unsigned int num_seats,
                       Vec* resources,  // Vec<Resource>
                       Vec* dishes      // Vec<Dish>
) {
  Result result = RESULT_OK;

  restaurant->score = 0;
  restaurant->timer = timer;
  restaurant->dishes = dishes;
  restaurant->num_seats = num_seats;
  atomic_init(&restaurant->is_closing, false);

  pthread_mutex_init(&restaurant->mtx, nullptr);

  result = kitchen_init(&restaurant->kitchen, resources);
  if (result != RESULT_OK) {
    return result;
  }

  sink_init(&restaurant->sink, timer);

  rng_init_main(&restaurant->rng, rng_seed);

  vec_init(&restaurant->cooks, sizeof(Cook));
  vec_init(&restaurant->waiters, sizeof(Waiter));
  queue_init(&restaurant->customers, sizeof(Customer));

  return RESULT_OK;
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
  unsigned int occupied_seats = 0;

  pthread_mutex_lock(&restaurant->mtx);
  for (FIFOQueueNode* node = restaurant->customers.head; node != nullptr;
       node = node->next) {
    Customer* old_customer = node->value;

    if (!old_customer->has_left) {
      ++occupied_seats;
    }
  }

  if (occupied_seats == restaurant->num_seats) {
    pthread_mutex_unlock(&restaurant->mtx);
    return RESULT_RESTAURANT_FULL;
  }

  Result result = RESULT_OK;

  // Has to be pre-allocated because the customer passed to customer_init has
  // to be in the same location as the one on the queue.
  Customer* customer = malloc(sizeof(Customer));
  if (customer == nullptr) {
    result = RESULT_OUT_OF_MEMORY;
  }

  if (result == RESULT_OK) {
    result = queue_push_allocated(&restaurant->customers, customer);
  }
  if (result != RESULT_OK) {
    free(customer);
  }

  if (result == RESULT_OK) {
    result = customer_init(customer, restaurant);
  }

  pthread_mutex_unlock(&restaurant->mtx);
  return result;
}

bool restaurant_is_closing(Restaurant* restaurant) {
  return atomic_load(&restaurant->is_closing);
}

bool restaurant_is_empty(Restaurant* restaurant, unsigned int expected_people) {
  unsigned int seated_people = 0;
  bool present_people = false;

  pthread_mutex_lock(&restaurant->mtx);

  for (FIFOQueueNode* node = restaurant->customers.head; node != nullptr;
       node = node->next) {
    Customer* customer = node->value;

    ++seated_people;

    pthread_mutex_lock(&customer->mtx);
    if (!customer->has_left) {
      present_people = true;
    }
    pthread_mutex_unlock(&customer->mtx);
  }

  pthread_mutex_unlock(&restaurant->mtx);
  return seated_people == expected_people && !present_people;
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

  for (FIFOQueueNode* node = restaurant->customers.head; node != nullptr;
       node = node->next) {
    Result local_result = customer_drop(node->value);

    // Don't overwrite previous failures;
    if (result == RESULT_OK) {
      result = local_result;
    }
  }
  queue_drop(&restaurant->customers, nullptr);

  sink_drop(&restaurant->sink);
  kitchen_drop(&restaurant->kitchen);
  pthread_mutex_destroy(&restaurant->mtx);
}
