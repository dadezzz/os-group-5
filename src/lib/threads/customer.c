#include "customer.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "../result.h"

static Result customer_run(Customer* customer) {
  // TODO

  return RESULT_OK;
}

static void* customer_thread(void* arg) {
  Result* result = malloc(sizeof(Result));
  if (result == nullptr) {
    return nullptr;
  }

  Result run_result = customer_run(arg);
  memcpy(result, &run_result, sizeof(Result));

  return result;
}

Result customer_init(Customer* customer) {
  int result =
      pthread_create(&customer->tid, nullptr, customer_thread, customer);
  if (result != 0) {
    return RESULT_THREAD_CREATION_FAILED;
  }

  return RESULT_OK;
}

Result customer_drop(Customer* customer) {
  if (customer == nullptr) {
    return RESULT_OK;
  }

  Result* run_result;
  int join_result = pthread_join(customer->tid, (void**)&run_result);

  if (join_result != 0) {
    return RESULT_THREAD_JOIN_FAILED;
  }

  if (run_result == nullptr) {
    return RESULT_OUT_OF_MEMORY;
  }

  return *run_result;
}
