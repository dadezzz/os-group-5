#ifndef TIMER_H
#define TIMER_H

#include <pthread.h>
#include <stdatomic.h>

typedef struct {
  atomic_int value;
  pthread_mutex_t mtx;
  pthread_cond_t signal;
} Timer;

void timer_init(Timer* timer);

void timer_tick(Timer* timer);

int timer_get(Timer* timer);

void timer_wait(Timer* timer, int ticks);

void timer_drop(Timer* timer);

#endif
