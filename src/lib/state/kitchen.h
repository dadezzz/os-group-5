#ifndef KITCHEN_H
#define KITCHEN_H

#include <pthread.h>
#include <stdatomic.h>

#include "../data/resources.h"
#include "../vec.h"

typedef struct {
  Resource* resource;
  atomic_bool available;
  atomic_int dirtiness;
} KitchenResource;

typedef struct {
  pthread_mutex_t mtx;
  Vec resources;  // Vec<KitchenResource>
} Kitchen;

Result kitchen_get_resources(Kitchen* kitchen,
                             Vec* requirements,  // Vec<Requirements>
                             Vec* acquired       // Vec<KitchenResource*>
);

void kitchen_drop_resources_dirty(Kitchen* kitchen,
                                  Vec* acquired  // Vec<KitchenResource*>
);

Result kitchen_init(Kitchen* kitchen, Vec* resources);

void kitchen_drop(Kitchen* kitchen);

#endif
