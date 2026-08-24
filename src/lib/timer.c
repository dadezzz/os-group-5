#include "timer.h"

#include <pthread.h>
#include <stdatomic.h>

void timer_init(Timer* timer) {
  atomic_init(&timer->value, 0);
  pthread_mutex_init(&timer->mtx, nullptr);
  pthread_cond_init(&timer->signal, nullptr);
}

void timer_tick(Timer* timer) {
  atomic_fetch_add(&timer->value, 1);
  pthread_mutex_lock(&timer->mtx);
  pthread_cond_broadcast(&timer->signal);
  pthread_mutex_unlock(&timer->mtx);
}

int timer_get(Timer* timer) {
  return atomic_load(&timer->value);
}

void timer_wait(Timer* timer, int ticks) {
  int target = atomic_load(&timer->value) + ticks;

  pthread_mutex_lock(&timer->mtx);

  while (atomic_load(&timer->value) < target) {
    pthread_cond_wait(&timer->signal, &timer->mtx);
  }

  pthread_mutex_unlock(&timer->mtx);
}

void timer_drop(Timer* timer) {
  if (timer == nullptr) {
    return;
  }

  pthread_cond_destroy(&timer->signal);
  pthread_mutex_destroy(&timer->mtx);
}
