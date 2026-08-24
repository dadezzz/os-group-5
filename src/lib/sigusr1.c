#include "sigusr1.h"

#include <signal.h>
#include <stdatomic.h>

#include "result.h"

static atomic_bool signal_raised;

static void signal_handler(int /*unused*/) {
  sigusr1_set_raised(true);
}

Result sigusr1_register_handler() {
  atomic_init(&signal_raised, false);

  struct sigaction sa = {};
  sa.sa_handler = signal_handler;

  int result = sigaction(SIGUSR1, &sa, nullptr);
  if (result != 0) {
    return RESULT_SIGUSR1_REGISTRATION_FAILED;
  }

  return RESULT_OK;
}

bool sigusr1_get_raised() {
  return atomic_load(&signal_raised);
}

void sigusr1_set_raised(bool value) {
  atomic_store(&signal_raised, value);
}
