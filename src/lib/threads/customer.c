#include "customer.h"

#include <math.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>

#include "../data/dishes/dishes.h"
#include "../result.h"
#include "../rng.h"
#include "../state/restaurant.h"
#include "../timer.h"
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

  if (customer->order_dishes.length == customer->dishes_served) {
    return total_price * (1 - (customer->time_waiting / customer->patience));
  }

  return -1 * total_price *
         log2(1 + (customer->patience / (1 + customer->dishes_served)));
}

static Result customer_thread(void* void_customer) {
  Customer* customer = void_customer;
  fprintf(stderr, "customer %lu started\n", customer->tid);

  // TODO: make this random.
  int ticks_to_wait = 5;
  timer_wait(customer->restaurant->timer, ticks_to_wait);
  pthread_mutex_lock(&customer->mtx);
  customer->wants_to_order = true;
  pthread_mutex_unlock(&customer->mtx);
  fprintf(stderr, "customer %lu started waiting\n", customer->tid);

  // Check patience every tick because waiters might modify it by entertaining
  // the customer.
  while (true) {
    timer_wait(customer->restaurant->timer, 1);

    pthread_mutex_lock(&customer->mtx);

    ++customer->time_waiting;
    fprintf(stderr, "customer %lu time_waiting: %d\n", customer->tid,
            customer->time_waiting);

    if (customer->time_waiting > customer->patience ||
        customer->order_dishes.length == customer->dishes_served) {
      pthread_mutex_unlock(&customer->mtx);
      break;
    }

    pthread_mutex_unlock(&customer->mtx);
  }

  pthread_mutex_lock(&customer->mtx);
  customer->has_left = true;
  pthread_mutex_unlock(&customer->mtx);

  double score = customer_order_calculate_score(customer);
  pthread_mutex_lock(&customer->restaurant->mtx);
  customer->restaurant->score += score;
  fprintf(stderr, "new restaurant score: %f, delta: %f\n",
          customer->restaurant->score, score);
  pthread_mutex_unlock(&customer->restaurant->mtx);
  fprintf(stderr, "customer %lu left\n", customer->tid);

  return RESULT_OK;
}

Result customer_init(Customer* customer, Restaurant* restaurant) {
  customer->restaurant = restaurant;
  rng_init_thread(&restaurant->rng, &customer->rng);

  customer->time_waiting = 0;
  // TODO: make this random.
  customer->patience = 10;
  customer->has_left = false;
  customer->wants_to_order = false;

  customer->dishes_served = 0;
  vec_init(&customer->order_dishes, sizeof(Dish));
  // TODO: add dishes to customer.order.

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
