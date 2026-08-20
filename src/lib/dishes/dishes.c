#include "dishes.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../result.h"
#include "../str.h"
#include "requirements.h"

static Result parse_dish(const char* str, Dish* dish) {
  size_t name_bytes = read_str_until_char(str, &dish->name, ',');

  if (name_bytes == 0 || str[name_bytes] =='\0') {
    return RESULT_DISHES_FILE_INVALID;
  }

  str += name_bytes + 1;

  char* price_str = nullptr;
  size_t price_bytes = read_str_until_char(str, &price_str, ',');

  if (price_bytes == 0 || str[price_bytes] =='\0') {
    free(price_str);
    return RESULT_DISHES_FILE_INVALID;
  }

  str += price_bytes + 1;

  dish->price = atoi(price_str);
  free(price_str);

  char* cook_time_str = nullptr;
  size_t cook_time_bytes = read_str_until_char(str, &cook_time_str, ',');

  if (cook_time_bytes == 0 || str[cook_time_bytes] =='\0') {
    free(cook_time_str);
    return RESULT_DISHES_FILE_INVALID;
  }

  str += cook_time_bytes + 1;

  dish->cook_time = atoi(cook_time_str);
  free(cook_time_str);

  char* requirements_str = nullptr;
  size_t requirements_bytes = read_str_until_char(str, &requirements_str, '\n');

  if (requirements_bytes == 0) {
    free(requirements_str);
    return RESULT_DISHES_FILE_INVALID;
  }

  Result result = requirements_load(requirements_str, &dish->requirements);
  free(requirements_str);
  if (result != RESULT_OK) {
    return result;
  }

  return RESULT_OK;
}

static Result parse_file(FILE* file, Vec* dishes) {
  // Skip reading the first line (CSV header).
  read_file_until_char(file, nullptr, '\n');

  // Parse each subsequent line.
  while (true) {
    char* line_str = nullptr;
    size_t line_bytes = read_file_until_char(file, &line_str, '\n');

    if (line_bytes == 0 && feof(file)) {
      free(line_str);
      break;
    }

    // At least 7 bytes require to make a valid 4 field CSV line.
    if (line_bytes < 7) {
      free(line_str);
      return RESULT_DISHES_FILE_INVALID;
    }

    Dish new_dish = {};
    Result result = parse_dish(line_str, &new_dish);
    free(line_str);
    if (result != RESULT_OK) {
      free(new_dish.name);
      requirements_drop(&new_dish.requirements);
      return result;
    }

    result = vec_push(dishes, &new_dish);
    if (result != RESULT_OK) {
      free(new_dish.name);
      requirements_drop(&new_dish.requirements);
      return result;
    }
  }

  return RESULT_OK;
}

Result dishes_load(const char* file_path, Vec* dishes) {
  vec_init(dishes, sizeof(Dish));

  FILE* file = fopen(file_path, "r");

  if (file == nullptr) {
    return RESULT_DISHES_FILE_NOT_OPENED;
  }

  Result result = parse_file(file, dishes);
  fclose(file);
  if (result != RESULT_OK) {
    return result;
  }

  if (dishes->length == 0) {
    return RESULT_DISHES_FILE_EMPTY;
  }

  return RESULT_OK;
}

void dishes_drop(Vec* dishes) {
  if (dishes == nullptr) {
    return;
  }

  for (size_t i = 0; i < dishes->length; ++i) {
    Dish* dish = vec_at(dishes, i);
    free(dish->name);
    requirements_drop(&dish->requirements);
  }

  vec_drop(dishes);
}
