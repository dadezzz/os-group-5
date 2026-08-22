#include "dish-ticket.h"

#include "../data/dishes/dishes.h"
#include "../threads/waiter.h"
#include "order.h"

void dish_ticket_init(DishTicket* dish_ticket,
                      Dish* dish,
                      Order* order,
                      Waiter* waiter) {
  dish_ticket->dish = dish;
  dish_ticket->order = order;
  dish_ticket->waiter = waiter;
}
