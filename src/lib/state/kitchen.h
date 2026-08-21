#ifndef KITCHEN_H
#define KITCHEN_H

#include <pthread.h>

#include "../vec.h"
#include "../data/resources.h"

typedef struct {
  Resource* resource;
  bool available;
  int dirtiness;
} KitchenResource;

typedef struct {
  pthread_mutex_t cook_mtx;
  Vec* kitchen_resources;
} Kitchen;

Result kitchen_get_resources(Kitchen* kitchen, Vec* requirements);
void kitchen_drop_resources(Kitchen* kitchen, Vec* requirements);

Result kitchen_init(Kitchen* kitchen, Vec* resources);
void kitchen_resources_drop(void* arg);
void kitchen_drop(Kitchen* kitchen);

#endif
