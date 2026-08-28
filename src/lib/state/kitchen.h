#ifndef KITCHEN_H
#define KITCHEN_H

#include <stdatomic.h>

#include "../data/resources.h"
#include "../vec.h"

typedef struct DishTicket DishTicket;

typedef struct {
  Resource* resource;
  atomic_bool available;
  // Doesn't need to be atomic since it's always updated and read by one cook at
  // a time while available is false.
  int dirtiness;
} KitchenResource;

typedef struct {
  Vec resources;  // Vec<KitchenResource>
} Kitchen;

Result kitchen_try_get_resources(Kitchen* kitchen,
                                 Vec* requirements,  // Vec<Requirements>
                                 Vec* acquired       // Vec<KitchenResource*>
);

double kitchen_calculate_contention_score(Kitchen* kitchen, DishTicket* ticket);

Result kitchen_init(Kitchen* kitchen, Vec* resources);

void kitchen_drop(Kitchen* kitchen);

#endif
