#include "dish-ticket.h"

#include <stdlib.h>

void dish_ticket_init(DishTicket* dish_ticket, Dish* dish) {
  dish_ticket->dish = dish;
}

void dish_ticket_drop(void* arg) {
  if (arg == nullptr) {
    return;
  }

  DishTicket* dish_ticket = arg;
  free(dish_ticket->dish);
}
