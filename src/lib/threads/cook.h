#ifndef COOK_H
#define COOK_H

#include <pthread.h>
#include <semaphore.h>

#include "../fifo-queue.h"
#include "../result.h"
#include "../rng.h"
#include "../state/dish-ticket.h"

typedef struct Cook {
  pthread_t tid;
  FIFOQueue task_q;
  int queued_time;
  RNGState rng;
  pthread_mutex_t mtx;
  sem_t sem;
  bool should_terminate;
} Cook;

Result cook_assign(DishTicket* dish_ticket, Cook* cook);

Result cook_init(Cook* cook, RNGState* rng_main_state);

Result cook_drop(Cook* cook);

#endif
