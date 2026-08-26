#ifndef DISHES_H
#define DISHES_H

#include "../../result.h"
#include "../../vec.h"

typedef struct Dish {
  char* name;
  unsigned int price;
  unsigned int cook_time;
  Vec requirements;  // Vec<Requirement>
} Dish;

Result dishes_load(const char* file_path,
                   Vec* dishes,    // Vec<Dish>
                   Vec* resources  // Vec<Resource>
);

void dish_drop(void* arg);

#endif
