#define _POSIX_C_SOURCE 200809L

#include "menu.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "result.h"

#define MAX_LINE 50
#define MAX_NAME 25
#define MAX_REQS 30

Menu* menu_new() {
  return malloc(sizeof(Menu));
}

Result menu_load(const char* file_path, Menu* menu) {
  menu->dishes = nullptr;
  menu->n_dishes = 0;

  FILE* file = fopen(file_path, "r");

  if (file == nullptr) {
    return RESULT_MENU_FILE_NOT_OPENED;
  }

  char* buf = nullptr;
  size_t buf_len = 0;

  // Skip reading the first line (CSV header).
  getline(&buf, &buf_len, file);

  while ((getline(&buf, &buf_len, file)) != -1) {
    char name[MAX_NAME];
    char reqs[MAX_REQS];
    int price;
    int time;

    if (sscanf(buf, "%s[^,],%d,%d,%s", name, &price, &time, reqs) == 4) {
      menu->dishes = realloc(menu->dishes, (menu->n_dishes + 1) * sizeof(Dish));

      Dish* d = &menu->dishes[menu->n_dishes];
      d->name = strdup(name);
      d->price = price;
      d->cook_time = time;

      Requirements* temp = parse_reqs(reqs);
      if (temp == nullptr) {
        fclose(file);
        return RESULT_MENU_INVALID_RESOURCES;
      }
      d->reqs = temp;

      menu->n_dishes++;
    }
  }

  free(buf);
  fclose(file);

  if (menu->n_dishes == 0) {
    return RESULT_MENU_FILE_EMPTY;
  }

  return RESULT_OK;
}

void menu_drop(Menu* menu) {
  for (size_t i = 0; i < menu->n_dishes; i++) {
    free(menu->dishes[i].name);
    free(menu->dishes[i].reqs);
  }
  free(menu->dishes);
  free(menu);
}

Requirements* parse_reqs(char* reqs_str) {
  if (!reqs_str || strlen(reqs_str) == 0) {
    return nullptr;
  }

  size_t count = 0;
  Requirements* reqs = NULL;

  char* token = strtok(reqs_str, ";");
  while (token != NULL) {
    char name[MAX_NAME];
    int qty = 1;

    if (sscanf(token, "%s:%d", name, &qty) == EOF) {
      strcpy(name, token);
    }

    count++;
    Requirements* temp = realloc(reqs, count * sizeof(Requirements));
    if (!temp) {
      free(reqs);
      return nullptr;
    }
    reqs = temp;

    reqs[count].type_id = strdup(name);
    reqs[count].quantity = qty;
  }

  return reqs;
}
