#include "dish-ticket.h"

#include <pthread.h>
#include <stdlib.h>

#include "../data/dishes/dishes.h"
#include "../threads/customer.h"
#include "../threads/waiter.h"

void dish_ticket_init(DishTicket* dish_ticket,
                      Dish* dish,
                      Customer* customer,
                      Waiter* waiter) {
  dish_ticket->dish = dish;
  dish_ticket->customer = customer;
  dish_ticket->waiter = waiter;
}

void dish_ticket_drop(void* arg) {
  if (arg == nullptr) {
    return;
  }

  DishTicket* dish_ticket = arg;
  free(dish_ticket->dish);
}
