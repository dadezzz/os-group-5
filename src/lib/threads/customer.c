#include "customer.h"

#include <pthread.h>
#include <semaphore.h>

#include "../result.h"
#include "../rng.h"
#include "../state/order.h"
#include "wrapper.h"

void customer_serve(Customer* customer) {
  if (customer->has_left) {
    return;
  }

  pthread_mutex_lock(&customer->mtx);

  ++customer->order.dishes_served;

  pthread_mutex_unlock(&customer->mtx);
}

static Result customer_thread(void* void_customer) {
  Customer* customer = void_customer;

  // wait a random amount of time and then set wants_to_order to true.

  // TODO

  customer->has_left = true;
  sem_post(customer->seats);
  return RESULT_OK;
}

Result customer_init(Customer* customer,
                     RNGState* rng_main_state,
                     sem_t* seats) {
  rng_state_init_thread(rng_main_state, &customer->rng);

  customer->wants_to_order = false;
  // TODO: create an order with random dishes
  // order_init(&customer->order, dishes, customer);

  customer->seats = seats;
  sem_wait(customer->seats);

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
