#include "restaurant.h"

#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

#include "../config.h"
#include "../fifo-queue.h"
#include "../result.h"
#include "../rng.h"
#include "../threads/cook.h"
#include "../threads/customer.h"
#include "../threads/waiter.h"
#include "../vec.h"
#include "dish-ticket.h"
#include "kitchen.h"
#include "sink.h"

Result restaurant_init(Restaurant* restaurant,
                       Config* config,
                       Vec* resources,  // Vec<Resource>
                       Vec* dishes      // Vec<Dish>
) {
  restaurant->score = 0;
  restaurant->config = config;
  restaurant->dishes = dishes;
  atomic_init(&restaurant->is_closing, false);

  pthread_mutex_init(&restaurant->mtx, nullptr);

  sink_init(&restaurant->sink, restaurant);
  rng_init_main(&restaurant->rng, config->random_seed);

  vec_init(&restaurant->cooks, sizeof(Cook));
  vec_init(&restaurant->waiters, sizeof(Waiter));
  queue_init(&restaurant->customers, sizeof(Customer));
  restaurant->present_customers = 0;
  restaurant->spawned_customers = 0;
  restaurant->left_unserved_customers = 0;

  return kitchen_init(&restaurant->kitchen, resources);
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

// Tries to spawn a customer unless the restaurant is full.
Result restaurant_spawn_customer(Restaurant* restaurant) {
  pthread_mutex_lock(&restaurant->mtx);
  if (restaurant->present_customers == restaurant->config->max_customers) {
    pthread_mutex_unlock(&restaurant->mtx);
    return RESULT_RESTAURANT_FULL;
  }

  Result result = queue_push_last(&restaurant->customers, nullptr);

  if (result == RESULT_OK) {
    Customer* customer = restaurant->customers.tail->value;
    result = customer_init(customer, restaurant);
  }
  if (result == RESULT_OK) {
    ++restaurant->present_customers;
    ++restaurant->spawned_customers;
  }

  pthread_mutex_unlock(&restaurant->mtx);
  return result;
}

bool restaurant_is_closing(Restaurant* restaurant) {
  return atomic_load(&restaurant->is_closing);
}

// Waits until all customers have arrived and for everyone to leave.
bool restaurant_has_finished(Restaurant* restaurant) {
  pthread_mutex_lock(&restaurant->mtx);
  unsigned int spawned_customers = restaurant->spawned_customers;
  unsigned int present_customers = restaurant->present_customers;
  pthread_mutex_unlock(&restaurant->mtx);

  return spawned_customers == restaurant->config->total_customers &&
         present_customers == 0;
}

void restaurant_time_wait(Restaurant* restaurant, unsigned int units) {
  usleep((unsigned int)(1e6 * units / restaurant->config->game_speed));
}

// Tries to drop the first customer with the objective to shorten the queue when
// total_customers is high.
Result restaurant_try_drop_first_customer(Restaurant* restaurant) {
  pthread_mutex_lock(&restaurant->mtx);
  FIFOQueueNode* node = restaurant->customers.head;
  // No customers present.
  if (node == nullptr) {
    pthread_mutex_unlock(&restaurant->mtx);
    return RESULT_OK;
  }

  Result result = RESULT_OK;
  Customer* customer = node->value;
  pthread_mutex_lock(&customer->mtx);

  // Check dishes_served, otherwise waiters might write to freed memory.
  if (customer->has_left &&
      customer->dishes_served == customer->order_dishes.length) {
    queue_pop_first(&restaurant->customers);
    pthread_mutex_unlock(&customer->mtx);
    result = customer_drop(customer);
    free(customer);
  } else {
    pthread_mutex_unlock(&customer->mtx);
  }

  pthread_mutex_unlock(&restaurant->mtx);

  return result;
}

Result restaurant_drop(Restaurant* restaurant) {
  if (restaurant == nullptr) {
    return RESULT_OK;
  }

  atomic_store(&restaurant->is_closing, true);

  Result result = RESULT_OK;

  // Drop cooks before waiters because the cook might still try to push to the
  // waiter's ready queue.
  for (size_t i = 0; i < restaurant->cooks.length; ++i) {
    Result local_result = cook_drop(vec_at(&restaurant->cooks, i));
    // Don't overwrite previous failures;
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
    if (result == RESULT_OK) {
      result = local_result;
    }
  }
  queue_drop(&restaurant->customers, nullptr);

  sink_drop(&restaurant->sink);
  kitchen_drop(&restaurant->kitchen);
  pthread_mutex_destroy(&restaurant->mtx);

  return result;
}
