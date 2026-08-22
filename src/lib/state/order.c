#include "order.h"

#include "../data/dishes/dishes.h"
#include "../threads/customer.h"
#include "../vec.h"

void order_init(Order* order, Customer* customer) {
  order->customer = customer;
  order->dishes_served = 0;
  vec_init(&order->dishes, sizeof(Dish));
}

void order_drop(Order* order) {
  if (order == nullptr) {
    return;
  }

  vec_drop(&order->dishes, nullptr);
}
