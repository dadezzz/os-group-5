#include "order.h"

#include "../threads/customer.h"
#include "../vec.h"

void order_init(Order* order, Vec* dishes, Customer* customer) {
  order->dishes = dishes;
  order->dishes_served = 0;
  order->customer = customer;
}

void order_drop(Order* order) {
  if (order == nullptr) {
    return;
  }

  vec_drop(order->dishes, nullptr);
}
