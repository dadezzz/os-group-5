#ifndef WAITER_H
#define WAITER_H

#include <pthread.h>
#include <semaphore.h>

#include "../fifo-queue.h"
#include "../result.h"
#include "../rng.h"
#include "../state/dish-ticket.h"

typedef struct Waiter {
  pthread_t tid;
  FIFOQueue ready_dish_tickets;  // FIFOQueue<DishTicket>
  RNGState rng;
  pthread_mutex_t mtx;
  sem_t sem;
  bool should_terminate;
} Waiter;

Result waiter_assign(Waiter* waiter, DishTicket* dish_ticket);

Result waiter_init(Waiter* waiter, RNGState* rng_main_state);

Result waiter_drop(Waiter* waiter);

#endif
