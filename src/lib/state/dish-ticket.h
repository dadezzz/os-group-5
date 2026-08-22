#ifndef DISH_TICKET_H
#define DISH_TICKET_H

#include <pthread.h>

#include "../data/dishes/dishes.h"
#include "order.h"

// Forward declare these to avoid circular header dependencies.
typedef struct Waiter Waiter;

typedef struct {
  Dish* dish;
  Order* order;
  Waiter* waiter;
} DishTicket;

void dish_ticket_init(DishTicket* dish_ticket,
                      Dish* dish,
                      Order* order,
                      Waiter* waiter);

#endif
