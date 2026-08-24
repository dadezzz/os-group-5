#ifndef WAITER_H
#define WAITER_H

#include <pthread.h>
#include <semaphore.h>

#include "../fifo-queue.h"
#include "../result.h"
#include "../rng.h"
#include "../state/dish-ticket.h"
#include "../state/restaurant.h"

typedef struct Waiter {
  RNGState rng;
  pthread_t tid;
  FIFOQueue ready_dish_tickets;  // FIFOQueue<DishTicket>
  pthread_mutex_t mtx;
  sem_t sem;
  Restaurant* restaurant;
} Waiter;

Result waiter_init(Waiter* waiter, Restaurant* restaurant);

Result waiter_assign(Waiter* waiter, DishTicket* dish_ticket);

Result waiter_drop(Waiter* waiter);

#endif
