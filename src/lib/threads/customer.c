#include "customer.h"

#include <pthread.h>

#include "../data/dishes/dishes.h"
#include "../result.h"
#include "../rng.h"
#include "../state/restaurant.h"
#include "../timer.h"
#include "wrapper.h"

void order_init(Customer* customer) {
  customer->dishes_served = 0;
  vec_init(&customer->order_dishes, sizeof(Dish));
}

void order_drop(Vec* order_dishes) {
  if (order_dishes == nullptr) {
    return;
  }

  vec_drop(order_dishes, nullptr);
}

void customer_serve(Customer* customer) {
  pthread_mutex_lock(&customer->mtx);

  if (customer->has_left) {
    return;
  }

  ++customer->dishes_served;

  // TODO: if order.dishes_served == order.dishes.length then the customer can
  // leave the restaurant.

  pthread_mutex_unlock(&customer->mtx);
}

static Result customer_thread(void* void_customer) {
  Customer* customer = void_customer;

  // TODO: make this random.
  int ticks_to_wait = 5;
  timer_wait(customer->restaurant->timer, ticks_to_wait);
  pthread_mutex_lock(&customer->mtx);
  customer->wants_to_order = true;
  pthread_mutex_unlock(&customer->mtx);

  // TODO: loop each tick, waiting for patience to finish. Check it every time
  // because waiters might modify it by entertaining the customer.

  pthread_mutex_lock(&customer->mtx);
  customer->has_left = true;
  pthread_mutex_unlock(&customer->mtx);

  // TODO: Update restaurant score.

  return RESULT_OK;
}

Result customer_init(Customer* customer, Restaurant* restaurant) {
  customer->restaurant = restaurant;
  rng_init_thread(&restaurant->rng, &customer->rng);

  customer->has_left = false;
  customer->wants_to_order = false;

  order_init(customer);
  // TODO: add dishes to customer.order.

  pthread_mutex_init(&customer->mtx, nullptr);
  return thread_init(&customer->tid, customer_thread, customer);
}

Result customer_drop(Customer* customer) {
  if (customer == nullptr) {
    return RESULT_OK;
  }

  Result result = thread_drop(customer->tid);

  pthread_mutex_destroy(&customer->mtx);
  order_drop(&customer->order_dishes);

  return result;
}
