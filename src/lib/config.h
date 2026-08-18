#ifndef CONFIG_H
#define CONFIG_H

#include "result.h"

typedef struct {
  int num_cooks;
  int num_waiters;
  int max_customers;
  int total_customers;
  const char* menu_file;
  const char* resources_file;
  double game_speed;
  int random_seed;
} Config;

Config* config_new();

Result config_load(Config* config);

void config_drop(Config* config);

#endif
