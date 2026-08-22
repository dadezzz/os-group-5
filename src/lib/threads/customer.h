#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <pthread.h>
#include <semaphore.h>

#include "../result.h"
#include "../rng.h"
#include "../state/order.h"

typedef struct Customer {
  pthread_t tid;
  RNGState rng;
  pthread_mutex_t mtx;
  sem_t* seats;
  Order* order;
  bool order_placed;
  // Used to prevent use-after-free on the DishTicket.
  bool has_left;
} Customer;

void customer_serve(Customer* customer);

Result customer_init(Customer* customer,
                     RNGState* rng_main_state,
                     sem_t* seats);

Result customer_drop(Customer* customer);

#endif
