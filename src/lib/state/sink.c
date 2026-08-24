#include "sink.h"

#include <pthread.h>

void sink_wash(Sink* sink, KitchenResource* resource) {
  pthread_mutex_lock(&sink->mtx);
  timer_wait(sink->timer, resource->resource->clean_time);
  pthread_mutex_unlock(&sink->mtx);
}

void sink_init(Sink* sink, Timer* timer) {
  sink->timer = timer;
  pthread_mutex_init(&sink->mtx, nullptr);
}

void sink_drop(Sink* sink) {
  if (sink == nullptr) {
    return;
  }

  pthread_mutex_destroy(&sink->mtx);
}
