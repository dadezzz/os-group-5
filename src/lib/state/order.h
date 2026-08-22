#ifndef ORDER_H
#define ORDER_H

#include "../vec.h"

// Forward declare these to avoid circular header dependencies.
typedef struct Customer Customer;

typedef struct {
  Vec* dishes;
  Customer* customer;
  size_t dishes_served;
} Order;

void order_init(Order* order, Vec* dishes, Customer* customer);

void order_drop(Order* order);

#endif
