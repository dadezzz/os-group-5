#ifndef DISH_TICKET_H
#define DISH_TICKET_H

#include "../dishes/dishes.h"

typedef struct {
  Dish* dish;
} DishTicket;

void dish_ticket_init(DishTicket* dish_ticket, Dish* dish);

void dish_ticket_drop(DishTicket* dish_ticket);

#endif
