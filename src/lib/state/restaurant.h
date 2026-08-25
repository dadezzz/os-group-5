#ifndef RESTAURANT_H
#define RESTAURANT_H

#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stddef.h>

#include "../config.h"
#include "../fifo-queue.h"
#include "../result.h"
#include "../rng.h"
#include "../vec.h"
#include "kitchen.h"
#include "sink.h"

typedef struct Restaurant {
  Config* config;
  double score;
  pthread_mutex_t mtx;
  RNGState rng;
  // Has to be a queue because we have references to customer that left in
  // DishTicket and we cannot drop or move them until the program ends.
  FIFOQueue customers;  // FIFOQueue<Customer>
  Vec waiters;          // Vec<Waiter>
  Vec cooks;            // Vec<Cook>
  atomic_bool is_closing;
  Vec* dishes;
  Kitchen kitchen;
  Sink sink;
} Restaurant;

Result restaurant_init(Restaurant* restaurant,
                       Config* config,
                       Vec* resources,  // Vec<Resource>
                       Vec* dishes      // Vec<Dish>
);

Result restaurant_spawn_cooks(Restaurant* restaurant, size_t quantity);

Result restaurant_spawn_waiters(Restaurant* restaurant, size_t quantity);

Result restaurant_spawn_customer(Restaurant* restaurant);

bool restaurant_is_closing(Restaurant* restaurant);

bool restaurant_has_finished(Restaurant* restaurant);

void restaurant_time_wait(Restaurant* restaurant, unsigned int units);

void restaurant_drop(Restaurant* restaurant);

#endif
