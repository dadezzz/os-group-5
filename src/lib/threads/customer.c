#include "customer.h"

#include <math.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>

#include "../data/dishes/dishes.h"
#include "../result.h"
#include "../rng.h"
#include "../state/restaurant.h"
#include "../vec.h"
#include "wrapper.h"

void customer_serve(Customer* customer) {
  pthread_mutex_lock(&customer->mtx);

  if (customer->has_left) {
    pthread_mutex_unlock(&customer->mtx);
    return;
  }

  ++customer->dishes_served;

  pthread_mutex_unlock(&customer->mtx);
}

static double customer_order_calculate_score(Customer* customer) {
  double total_price = customer_order_total_price(customer);

  double score;

  if (customer->order_dishes.length == customer->dishes_served) {
    score = total_price * (1 - (customer->time_waiting / customer->patience));
    fprintf(stderr, "customer %lu left with complete order, delta: %f\n",
            customer->tid, score);
  } else {
    score = -1 * total_price *
            log2(1 + (customer->patience / (1 + customer->dishes_served)));
    fprintf(stderr,
            "customer %lu sadly left with incomplete order, delta: %f\n",
            customer->tid, score);
  }

  return score;
}

static Result customer_thread(void* void_customer) {
  Customer* customer = void_customer;
  fprintf(stderr, "customer %lu started\n", customer->tid);

  unsigned int ticks_to_wait = rng_next_range(&customer->rng, 5, 25);
  restaurant_time_wait(customer->restaurant, ticks_to_wait);
  pthread_mutex_lock(&customer->mtx);
  customer->wants_to_order = true;
  pthread_mutex_unlock(&customer->mtx);
  fprintf(stderr, "customer %lu wants to order\n", customer->tid);

  // Check patience every tick because waiters might modify it by entertaining
  // the customer.
  while (true) {
    restaurant_time_wait(customer->restaurant, ticks_to_wait);

    pthread_mutex_lock(&customer->mtx);

    ++customer->time_waiting;

    if (customer->time_waiting > customer->patience ||
        customer->order_dishes.length == customer->dishes_served) {
      pthread_mutex_unlock(&customer->mtx);
      break;
    }

    pthread_mutex_unlock(&customer->mtx);
  }

  pthread_mutex_lock(&customer->mtx);
  customer->has_left = true;
  double score = customer_order_calculate_score(customer);
  pthread_mutex_unlock(&customer->mtx);

  pthread_mutex_lock(&customer->restaurant->mtx);
  customer->restaurant->score += score;
  pthread_mutex_unlock(&customer->restaurant->mtx);

  return RESULT_OK;
}

Result customer_init(Customer* customer, Restaurant* restaurant) {
  customer->restaurant = restaurant;
  rng_init_thread(&restaurant->rng, &customer->rng);

  customer->time_waiting = 0;
  customer->has_left = false;
  customer->wants_to_order = false;

  customer->dishes_served = 0;
  vec_init(&customer->order_dishes, sizeof(Dish));
  int n_dishes = rng_next_range(&customer->rng, 1, 10);
  int cook_time = 0;
  for (int i = 0; i < n_dishes; i++) {
    Dish* dish = vec_at(
        restaurant->dishes,
        rng_next_range(&customer->rng, 0, restaurant->dishes->length - 1));
    cook_time += dish->cook_time;

    Result result = vec_push(&customer->order_dishes, dish);
    if (result != RESULT_OK) {
      return result;
    }
  }

  customer->patience =
      rng_next_range(&customer->rng, cook_time + 1, cook_time + 100);

  pthread_mutex_init(&customer->mtx, nullptr);
  return thread_init(&customer->tid, customer_thread, customer);
}

int customer_order_total_price(Customer* customer) {
  int acc = 0;

  for (size_t i = 0; i < customer->order_dishes.length; ++i) {
    Dish* dish = vec_at(&customer->order_dishes, i);
    acc += dish->price;
  }

  return acc;
}

Result customer_drop(Customer* customer) {
  if (customer == nullptr) {
    return RESULT_OK;
  }

  Result result = thread_drop(customer->tid);

  pthread_mutex_destroy(&customer->mtx);
  vec_drop(&customer->order_dishes, nullptr);

  return result;
}
