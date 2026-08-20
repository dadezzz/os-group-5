#include "dish-ticket.h"

#include "../fifo-queue.h"

void dish_ticket_init(DishTicket* dish_ticket, Dish* dish) {
  dish_ticket->dish = dish;
}

void dish_ticket_drop(DishTicket* dish_ticket) {
  if (dish_ticket == nullptr) {
    return;
  }

  free(dish_ticket->dish);
}
