#ifndef WRAPPER_H
#define WRAPPER_H

#include <pthread.h>

#include "../result.h"

Result thread_init(pthread_t* tid, Result (*cb)(void*), void* user_arg);

Result thread_drop(pthread_t tid);

#endif
