#include "customer.h"

#include <pthread.h>

#include "../result.h"
#include "../rng.h"
#include "../state/order.h"
#include "../state/restaurant.h"
#include "wrapper.h"

void customer_serve(Customer* customer) {
  if (customer->has_left) {
    return;
  }

  pthread_mutex_lock(&customer->mtx);

  ++customer->order.dishes_served;

  // TODO: if order.dishes_served == order.dishes.length then the customer can
  // leave the restaurant.

  pthread_mutex_unlock(&customer->mtx);
}

static Result customer_thread(void* void_customer) {
  Customer* customer = void_customer;

  // wait a random amount of time and then set wants_to_order to true.

  // TODO

  // Update restaurant score.

  customer->has_left = true;
  return RESULT_OK;
}

Result customer_init(Customer* customer, Restaurant* restaurant) {
  customer->restaurant = restaurant;
  rng_init_thread(&restaurant->rng, &customer->rng);

  customer->wants_to_order = false;
  order_init(&customer->order, customer);
  // TODO: add dishes to customer.order.

  return thread_init(&customer->tid, customer_thread, customer);
}

Result customer_drop(Customer* customer) {
  if (customer == nullptr) {
    return RESULT_OK;
  }

  Result result = thread_drop(customer->tid);

  order_drop(&customer->order);

  return result;
}
