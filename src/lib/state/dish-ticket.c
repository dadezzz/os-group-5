#include "dish-ticket.h"

#include <pthread.h>
#include <stdlib.h>

#include "../data/dishes/dishes.h"

void dish_ticket_init(DishTicket* dish_ticket,
                      Dish* dish,
                      pthread_t customer_tid,
                      pthread_t waiter_tid) {
  dish_ticket->dish = dish;
  dish_ticket->customer_tid = customer_tid;
  dish_ticket->waiter_tid = waiter_tid;
}

void dish_ticket_drop(void* arg) {
  if (arg == nullptr) {
    return;
  }

  DishTicket* dish_ticket = arg;
  free(dish_ticket->dish);
}
