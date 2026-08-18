#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#include "result.h"

typedef struct {
  uint32_t num_cooks;
  uint32_t num_waiters;
  uint32_t max_customers;
  uint32_t total_customers;
  const char* menu_file;
  const char* resources_file;
  double game_speed;
  uint32_t random_seed;
} Config;

Config* config_new();

Result config_load(Config* config);

void config_drop(Config* config);

#endif
