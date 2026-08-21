#include "customer.h"

#include <semaphore.h>

#include "../result.h"
#include "../rng.h"
#include "wrapper.h"

static Result customer_run(void* void_customer) {
  Customer* customer = void_customer;

  // TODO

  sem_post(customer->seats);
  return RESULT_OK;
}

Result customer_init(Customer* customer,
                     RNGState* rng_main_state,
                     sem_t* seats) {
  rng_state_init_thread(rng_main_state, &customer->rng);
  customer->seats = seats;
  sem_wait(customer->seats);

  return thread_init(&customer->tid, customer_run, customer);
}

Result customer_drop(Customer* customer) {
  if (customer == nullptr) {
    return RESULT_OK;
  }

  Result result = thread_drop(customer->tid);

  return result;
}
