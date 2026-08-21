#ifndef COOK_H
#define COOK_H

#include <pthread.h>

#include "../fifo-queue.h"
#include "../result.h"
#include "../rng.h"
#include "../state/dish-ticket.h"

typedef struct Cook {
  pthread_t tid;
  FIFOQueue task_q;
  int queued_time;
  RNGState* rng;
} Cook;

Result cook_assign(DishTicket* dish_ticket, Cook* cook);

Result cook_init(Cook* cook, RNGState* rng);

Result cook_drop(Cook* cook);

#endif
