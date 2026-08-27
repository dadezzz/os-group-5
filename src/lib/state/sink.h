#ifndef SINK_H
#define SINK_H

#include <pthread.h>
#include <stdatomic.h>

#include "../vec.h"

typedef struct Restaurant Restaurant;

typedef struct Sink {
  pthread_mutex_t mtx;
  pthread_cond_t signal;
  Restaurant* restaurant;
  int waiting;
  bool locked;
} Sink;

void sink_wash_all(Sink* sink, Vec* dirty_resources);

void sink_init(Sink* sink, Restaurant* restaurant);

void sink_drop(Sink* sink);

#endif
