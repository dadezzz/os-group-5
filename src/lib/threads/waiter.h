#ifndef WAITER_H
#define WAITER_H

#include <pthread.h>

#include "../fifo-queue.h"
#include "../state/dish-ticket.h"
#include "../rng.h"

typedef struct {
  pthread_t tid;
  FIFOQueue ready_q;
  int queued_time;
  RNGState* rng;
} Waiter;

Result waiter_assign(Waiter* waiter ,DishTicket* dish_ticket);

Result waiter_init(Waiter* waiter, RNGState* rng);

Result waiter_drop(Waiter* waiter);

#endif
