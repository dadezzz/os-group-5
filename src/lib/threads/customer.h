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
} Customer;

Result customer_init(Customer* customer,
                     RNGState* rng_main_state,
                     sem_t* seats);

Result customer_drop(Customer* customer);

#endif
