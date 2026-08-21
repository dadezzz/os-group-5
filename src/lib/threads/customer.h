#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <pthread.h>
#include <semaphore.h>

#include "../result.h"
#include "../rng.h"

typedef struct Customer {
  pthread_t tid;
  RNGState rng;
  sem_t* seats;
  // Used to prevent use-after-free on the DishTicket.
  bool has_left;
} Customer;

Result customer_init(Customer* customer,
                     RNGState* rng_main_state,
                     sem_t* seats);

Result customer_drop(Customer* customer);

#endif
