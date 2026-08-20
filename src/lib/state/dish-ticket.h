#ifndef DISH_TICKET_H
#define DISH_TICKET_H

#include <pthread.h>
#include "../dishes/dishes.h"

typedef struct {
  Dish* dish;
  pthread_t customer_tid;
  pthread_t waiter_tid;
} DishTicket;

void dish_ticket_init(DishTicket* dish_ticket, Dish* dish, pthread_t customer_tid, pthread_t waiter_tid);

void dish_ticket_drop(void* arg);

#endif
