#include "sink.h"

#include <pthread.h>
#include <stdatomic.h>

#include "kitchen.h"
#include "restaurant.h"

void sink_wash(Sink* sink, KitchenResource* resource) {
  atomic_fetch_add(&sink->waiting, 1);
  pthread_mutex_lock(&sink->mtx);

  restaurant_time_wait(sink->restaurant, resource->resource->clean_time);
  atomic_fetch_sub(&sink->waiting, 1);

  pthread_mutex_lock(&sink->restaurant->kitchen.mtx);
  resource->dirtiness = 0;
  pthread_mutex_unlock(&sink->restaurant->kitchen.mtx);

  pthread_mutex_unlock(&sink->mtx);
}

void sink_init(Sink* sink, Restaurant* restaurant) {
  sink->restaurant = restaurant;
  atomic_init(&sink->waiting, 0);
  pthread_mutex_init(&sink->mtx, nullptr);
}

void sink_drop(Sink* sink) {
  if (sink == nullptr) {
    return;
  }

  pthread_mutex_destroy(&sink->mtx);
}
