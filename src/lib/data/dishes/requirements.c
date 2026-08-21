#include "requirements.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../../result.h"
#include "../../str.h"
#include "../../vec.h"

static Result parse_requirement(const char* requirement_str,
                                Requirement* requirement,
                                Vec* resources) {
  char* req_name;
  size_t name_bytes = read_str_until_char(requirement_str, &req_name, ':');

  // Return error if the first character was ':' or there was no name.
  if (name_bytes == 0) {
    return RESULT_DISHES_FILE_INVALID;
  }

  // Link required resource
  for (size_t i = 0; i < resources->length; i++) {
    Resource* resource = vec_at(resources, i);
    if (strcmp(req_name, resource->name) == 0) {
      requirement->resource = resource;
    }
  }

  // Make sure link exists
  if (requirement->resource == nullptr) {
    return RESULT_DISHES_RESOURCE_NOT_FOUND;
  }

  // Try to read the quantity value if a colon was found.
  if (requirement_str[name_bytes] == ':') {
    char* quantity_str = nullptr;
    size_t quantity_bytes = read_str_until_char(
        &requirement_str[name_bytes] + 1, &quantity_str, '\0');

    // Return error if the string ended with a colon or was empty.
    if (quantity_bytes == 0) {
      free(quantity_str);
      return RESULT_DISHES_FILE_INVALID;
    }

    requirement->quantity = atoi(quantity_str);
    free(quantity_str);
  } else {
    requirement->quantity = 1;
  }

  return RESULT_OK;
}

Result requirements_load(const char* requirements_str,
                         Vec* requirements,
                         Vec* resources) {
  // This is like a cursor to advance parsing with sscanf.
  size_t total_bytes_read = 0;

  // Loop until the end of the string.
  while (requirements_str[total_bytes_read] != '\0') {
    char* requirement_str = nullptr;

    size_t bytes_read = read_str_until_char(&requirements_str[total_bytes_read],
                                            &requirement_str, ';');

    // Return error if there were 2 semicolons together or the string ended with
    // a semicolon.
    if (bytes_read == 0) {
      free(requirement_str);
      return RESULT_DISHES_FILE_INVALID;
    }

    total_bytes_read +=
        requirements_str[total_bytes_read + bytes_read] == ';'
            // If there are more requirements then continue to loop.
            ? bytes_read + 1
            // Otherwise end this cycle and then break.
            : bytes_read;

    Requirement new_requirement = {};
    Result result =
        parse_requirement(requirement_str, &new_requirement, resources);
    free(requirement_str);
    if (result != RESULT_OK) {
      requirement_drop(&new_requirement);
      return result;
    }

    result = vec_push(requirements, &new_requirement);
    if (result != RESULT_OK) {
      requirement_drop(&new_requirement);
      return result;
    }
  }

  if (requirements->length == 0) {
    return RESULT_DISHES_FILE_INVALID;
  }

  return RESULT_OK;
}

void requirement_drop(void* arg) {
  if (arg == nullptr) {
    return;
  }

  Requirement* requirement = arg;
  resource_drop(requirement->resource);
}
