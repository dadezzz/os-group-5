#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <pthread.h>

#include "../result.h"
#include "../rng.h"

typedef struct {
  pthread_t tid;
  RNGState* rng;
} Customer;

Result customer_init(Customer* customer, RNGState* rng);

Result customer_drop(Customer* customer);

#endif
