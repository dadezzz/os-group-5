#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "result.h"

static Result get_int_env(const char* name, int* value) {
  const char* value_str = getenv(name);
  if (value_str == nullptr) {
    return RESULT_CONFIG_MISSING_VALUE;
  }

  *value = atoi(value_str);
  return RESULT_OK;
}

static Result get_double_env(const char* name, double* value) {
  const char* value_str = getenv(name);
  if (value_str == nullptr) {
    return RESULT_CONFIG_MISSING_VALUE;
  }

  *value = atof(value_str);
  return RESULT_OK;
}

static Result get_string_env(const char* name, char** value) {
  const char* value_str = getenv(name);
  if (value_str == nullptr) {
    return RESULT_CONFIG_MISSING_VALUE;
  }

  // Duplicate the string, so that we can safely call free on it.
  *value = strdup(value_str);
  return RESULT_OK;
}

Result config_load(Config* config) {
  Result result;

  // int num_cooks;
  // result = get_int_env("NUM_COOKS", &num_cooks);
  // if (result != RESULT_OK) {
  //   return result;
  // }
  // config->num_cooks = num_cooks;

  // int num_waiters;
  // result = get_int_env("NUM_WAITERS", &num_waiters);
  // if (result != RESULT_OK) {
  //   return result;
  // }
  // config->num_waiters = num_waiters;

  // int max_customers;
  // result = get_int_env("MAX_CUSTOMERS", &max_customers);
  // if (result != RESULT_OK) {
  //   return result;
  // }
  // config->max_customers = max_customers;

  // int total_customers;
  // result = get_int_env("TOTAL_CUSTOMERS", &total_customers);
  // if (result != RESULT_OK) {
  //   return result;
  // }
  // config->total_customers = total_customers;

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

  // double game_speed;
  // result = get_double_env("GAME_SPEED", &game_speed);
  // if (result != RESULT_OK) {
  //   return result;
  // }
  // config->game_speed = game_speed;

  int random_seed;
  result = get_int_env("RANDOM_SEED", &random_seed);
  if (result != RESULT_OK) {
    return result;
  }
  config->random_seed = random_seed;

  return RESULT_OK;
}

void config_drop(Config* config) {
  if (config == nullptr) {
    return;
  }

  free(config->menu_file);
  free(config->resources_file);
}
