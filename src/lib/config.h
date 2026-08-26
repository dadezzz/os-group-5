#ifndef CONFIG_H
#define CONFIG_H

#include "result.h"

typedef struct Config {
  unsigned int num_cooks;
  unsigned int num_waiters;
  unsigned int max_customers;
  unsigned int total_customers;
  char* menu_file;
  char* resources_file;
  double game_speed;
  unsigned int random_seed;
} Config;

Result config_load(Config* config);

void config_drop(Config* config);

#endif
