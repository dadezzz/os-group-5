#ifndef WAITER_H
#define WAITER_H

#include <pthread.h>

#include "../fifo-queue.h"
#include "../result.h"
#include "../rng.h"
#include "../state/dish-ticket.h"

typedef struct Waiter {
  pthread_t tid;
  FIFOQueue ready_q;
  int queued_time;
  RNGState* rng;
  pthread_mutex_t work_mutex;
  bool terminate;
} Waiter;

Result waiter_assign(Waiter* waiter, DishTicket* dish_ticket);

Result waiter_init(Waiter* waiter, RNGState* rng);

Result waiter_drop(Waiter* waiter);

#endif
