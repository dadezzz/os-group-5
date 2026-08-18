// Some implementations of libc don't support strdup in c23 unless this macro is
// defined as in https://en.cppreference.com/c/experimental/dynamic/strdup.
#define _POSIX_C_SOURCE 200809L

#include "config.h"

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "result.h"

Config* config_new() {
  return malloc(sizeof(Config));
}

static Result get_uint32_t_env(const char* name, uintmax_t* value) {
  const char* value_str = getenv(name);

  char* endptr = nullptr;
  *value = strtoumax(value_str, &endptr, 10);
  if (errno == ERANGE || *value > UINT32_MAX || *endptr != '\0') {
    return RESULT_CONFIG_INVALID_VALUE;
  }

  return RESULT_OK;
}

static Result get_double_env(const char* name, double* value) {
  const char* value_str = getenv(name);

  char* endptr = nullptr;
  *value = strtod(value_str, &endptr);
  if (errno == ERANGE || *endptr != '\0') {
    return RESULT_CONFIG_INVALID_VALUE;
  }

  return RESULT_OK;
}

static Result get_string_env(const char* name, char** value) {
  const char* value_str = getenv(name);
  if (value == nullptr) {
    return RESULT_CONFIG_MISSING_VALUE;
  }

  *value = strdup(value_str);

  return RESULT_OK;
}

Result config_load(Config* config) {
  Result result;

  uintmax_t num_cooks;
  result = get_uint32_t_env("NUM_COOKS", &num_cooks);
  if (result != RESULT_OK) {
    return result;
  }
  config->num_cooks = (uint32_t)num_cooks;

  uintmax_t num_waiters;
  result = get_uint32_t_env("NUM_WAITERS", &num_waiters);
  if (result != RESULT_OK) {
    return result;
  }
  config->num_waiters = (uint32_t)num_waiters;

  uintmax_t max_customers;
  result = get_uint32_t_env("MAX_CUSTOMERS", &max_customers);
  if (result != RESULT_OK) {
    return result;
  }
  config->max_customers = (uint32_t)max_customers;

  uintmax_t total_customers;
  result = get_uint32_t_env("TOTAL_CUSTOMERS", &total_customers);
  if (result != RESULT_OK) {
    return result;
  }
  config->total_customers = (uint32_t)total_customers;

  char* menu_file;
  result = get_string_env("MENU_FILE", &menu_file);
  if (result != RESULT_OK) {
    return result;
  }
  config->menu_file = menu_file;

  char* resources_file;
  result = get_string_env("RESOURCES_FILE", &resources_file);
  if (result != RESULT_OK) {
    return result;
  }
  config->resources_file = resources_file;

  double game_speed;
  result = get_double_env("GAME_SPEED", &game_speed);
  if (result != RESULT_OK) {
    return result;
  }
  config->game_speed = game_speed;

  uintmax_t random_seed;
  result = get_uint32_t_env("RANDOM_SEED", &random_seed);
  if (result != RESULT_OK) {
    return result;
  }
  config->random_seed = (uint32_t)random_seed;

  return RESULT_OK;
}

void config_drop(Config* config) {
  free(config->menu_file);
  free(config->resources_file);
  free(config);
}
