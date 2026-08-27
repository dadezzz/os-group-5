#include "sink.h"

#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>

#include "kitchen.h"
#include "restaurant.h"

// Washes and also makes available all the dirty resources.
void sink_wash_all(Sink* sink, Vec* dirty_resources) {
  pthread_mutex_lock(&sink->mtx);
  sink->waiting += dirty_resources->length;
  while (sink->locked) {
    pthread_cond_wait(&sink->signal, &sink->mtx);
  }
  sink->locked = true;
  pthread_mutex_unlock(&sink->mtx);

  for (size_t i = 0; i < dirty_resources->length; ++i) {
    KitchenResource** resource_ref = vec_at(dirty_resources, i);
    KitchenResource* resource = *resource_ref;
    restaurant_time_wait(sink->restaurant, resource->resource->clean_time);
    atomic_store(&resource->dirtiness, 0);
    atomic_store(&resource->available, true);
  }

  pthread_mutex_lock(&sink->mtx);
  sink->waiting -= dirty_resources->length;
  sink->locked = false;
  pthread_cond_signal(&sink->signal);
  pthread_mutex_unlock(&sink->mtx);
}

void sink_init(Sink* sink, Restaurant* restaurant) {
  sink->restaurant = restaurant;
  sink->waiting = 0;
  sink->locked = false;
  pthread_mutex_init(&sink->mtx, nullptr);
  pthread_cond_init(&sink->signal, nullptr);
}

void sink_drop(Sink* sink) {
  if (sink == nullptr) {
    return;
  }

  pthread_cond_destroy(&sink->signal);
  pthread_mutex_destroy(&sink->mtx);
}
