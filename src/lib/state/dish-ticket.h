#ifndef DISH_TICKET_H
#define DISH_TICKET_H

#include <pthread.h>

#include "../data/dishes/dishes.h"

// Forward declare these to avoid circular header dependencies.
typedef struct Customer Customer;
typedef struct Waiter Waiter;

typedef struct {
  Dish* dish;
  Customer* customer;
  Waiter* waiter;
} DishTicket;

void dish_ticket_init(DishTicket* dish_ticket,
                      Dish* dish,
                      Customer* customer,
                      Waiter* waiter);

void dish_ticket_drop(void* arg);

#endif
