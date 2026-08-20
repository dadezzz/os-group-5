#ifndef WAITER_H
#define WAITER_H

#include <pthread.h>

#include "../fifo-queue.h"
#include "../state/dish-ticket.h"

typedef struct {
  pthread_t tid;
  FIFOQueue ready_q;
  int queued_time;
} Waiter;

Result waiter_assign(Waiter* waiter ,DishTicket* dish_ticket);

Result waiter_init(Waiter* waiter);

Result waiter_drop(Waiter* waiter);

#endif
