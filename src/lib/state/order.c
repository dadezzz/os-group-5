#include "order.h"

#include "../data/dishes/dishes.h"
#include "../threads/customer.h"
#include "../vec.h"

void order_init(Order* order, Vec* dishes, Customer* customer) {
  order->dishes = dishes;
  order->pending_dishes = dishes->length;
  order->customer = customer;
}

void order_drop(Order* order) {
  if (order == nullptr) {
    return;
  }

  customer_drop(order->customer);
  vec_drop(order->dishes, dish_drop);
}
