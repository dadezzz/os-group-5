#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <pthread.h>

#include "../result.h"

typedef struct {
  pthread_t tid;
} Customer;

Result customer_init(Customer* customer);

Result customer_drop(Customer* customer);

#endif
