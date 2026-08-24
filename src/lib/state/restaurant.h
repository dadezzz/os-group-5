#ifndef RESTAURANT_H
#define RESTAURANT_H

#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stddef.h>

#include "../result.h"
#include "../rng.h"
#include "../timer.h"
#include "../vec.h"

typedef struct Restaurant {
  double score;
  pthread_mutex_t mtx;
  RNGState rng;
  unsigned int num_seats;
  Vec customers;  // Vec<Cook>
  Vec waiters;    // Vec<Waiter>
  Vec cooks;      // Vec<Customer>
  Timer* timer;
  atomic_bool is_closing;
} Restaurant;

void restaurant_init(Restaurant* restaurant,
                     Timer* timer,
                     unsigned int rng_seed,
                     unsigned int num_seats);

Result restaurant_spawn_cooks(Restaurant* restaurant, size_t quantity);

Result restaurant_spawn_waiters(Restaurant* restaurant, size_t quantity);

Result restaurant_spawn_customer(Restaurant* restaurant);

bool restaurant_is_closing(Restaurant* restaurant);

bool restaurant_is_empty(Restaurant* restaurant);

void restaurant_drop(Restaurant* restaurant);

#endif
