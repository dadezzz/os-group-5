#ifndef COOK_H
#define COOK_H

#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>

#include "../fifo-queue.h"
#include "../result.h"
#include "../rng.h"
#include "../state/dish-ticket.h"
#include "../state/restaurant.h"

typedef struct Cook {
  RNGState rng;
  pthread_t tid;
  FIFOQueue dish_tickets;  // FIFOQueue<DishTicket>
  int queued_time;
  pthread_mutex_t mtx;
  sem_t sem;
  Restaurant* restaurant;
} Cook;

Result cook_init(Cook* cook, Restaurant* restaurant);

Result cook_assign(Cook* cook, DishTicket* dish_ticket);

Result cook_drop(Cook* cook);

#endif
