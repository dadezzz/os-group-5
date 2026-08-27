#include "sink.h"

#include <semaphore.h>
#include <stdatomic.h>
#include <stddef.h>

#include "kitchen.h"
#include "restaurant.h"

// Washes and also makes available all the dirty resources.
void sink_wash_all(Sink* sink, Vec* dirty_resources) {
  atomic_fetch_add(&sink->waiting, dirty_resources->length);
  sem_wait(&sink->locked);

  for (size_t i = 0; i < dirty_resources->length; ++i) {
    KitchenResource** resource_ref = vec_at(dirty_resources, i);
    KitchenResource* resource = *resource_ref;
    restaurant_time_wait(sink->restaurant, resource->resource->clean_time);
    atomic_store(&resource->dirtiness, 0);
    atomic_store(&resource->available, true);
  }

  atomic_fetch_sub(&sink->waiting, dirty_resources->length);
  sem_post(&sink->locked);
}

void sink_init(Sink* sink, Restaurant* restaurant) {
  sink->restaurant = restaurant;
  atomic_init(&sink->waiting, 0);
  sem_init(&sink->locked, 0, 1);
}

void sink_drop(Sink* sink) {
  if (sink == nullptr) {
    return;
  }

  sem_destroy(&sink->locked);
}
