#ifndef MENU_H
#define MENU_H

#include <stddef.h>
#include <stdio.h>
#include "result.h"

typedef struct {
  char* type_id;
  int quantity;
} Requirements;

typedef struct {
  char* name;
  int price;
  int cook_time;
  Requirements* reqs;
} Dish;

typedef struct {
  Dish* dishes;
  size_t n_dishes;
} Menu;

Menu* menu_new();
Result menu_load(const char* file_path, Menu* menu);
void menu_drop(Menu* menu);
Requirements* parse_reqs(char* reqs_str);

#endif
