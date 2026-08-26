#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <pthread.h>

#include "../result.h"
#include "../rng.h"
#include "../state/restaurant.h"

typedef struct Customer {
  RNGState rng;
  pthread_t tid;
  pthread_mutex_t mtx;
  Vec order_dishes;
  size_t dishes_served;
  int time_waiting;
  double patience;
  bool wants_to_order;
  bool has_left;
  Restaurant* restaurant;
} Customer;

void customer_serve(Customer* customer);

Result customer_init(Customer* customer, Restaurant* restaurant);

int customer_order_total_price(Customer* customer);

Result customer_drop(Customer* customer);

#endif
