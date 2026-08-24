#include "dish-ticket.h"

#include "../data/dishes/dishes.h"
#include "../threads/waiter.h"

void dish_ticket_init(DishTicket* dish_ticket,
                      Dish* dish,
                      Customer* customer,
                      Waiter* waiter) {
  dish_ticket->dish = dish;
  dish_ticket->customer = customer;
  dish_ticket->waiter = waiter;
}
