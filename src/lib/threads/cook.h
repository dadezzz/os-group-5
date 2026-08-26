#ifndef COOK_H
#define COOK_H

#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>

#include "../fifo-queue.h"
#include "../result.h"
#include "../state/dish-ticket.h"
#include "../state/restaurant.h"

typedef struct Cook {
  pthread_t tid;
  FIFOQueue dish_tickets;  // FIFOQueue<DishTicket>
  // Use these for faster lookups, without the need to iterate the queue.
  unsigned int queued_time;
  unsigned int queued_price;
  // Used to select what to cook. When receiving a new order, the cook tries to
  // loop over the queue to find something with immediately available resource.
  // If it can't find one, then it falls back to waiting for resources to
  // complete the plate of the customer that has been waiting the most.
  atomic_bool wait_first_in_queue;
  pthread_mutex_t mtx;
  sem_t sem;
  Restaurant* restaurant;
} Cook;

Result cook_init(Cook* cook, Restaurant* restaurant);

Result cook_assign(Cook* cook, DishTicket* dish_ticket);

Result cook_drop(Cook* cook);

#endif
