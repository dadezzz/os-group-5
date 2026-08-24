#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <pthread.h>
#include <semaphore.h>

#include "../result.h"
#include "../rng.h"
#include "../state/restaurant.h"

typedef struct Customer {
  RNGState rng;
  pthread_t tid;
  pthread_mutex_t mtx;
  sem_t* seats;
  Vec order_dishes;
  size_t dishes_served;
  int time_to_serve;
  int patience;
  bool wants_to_order;
  bool has_left;
  Restaurant* restaurant;
} Customer;

void customer_serve(Customer* customer);

Result customer_init(Customer* customer, Restaurant* restaurant);

Result customer_drop(Customer* customer);

#endif
