#include "kitchen.h"

#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../data/dishes/requirements.h"
#include "../data/resources.h"
#include "../vec.h"

Result kitchen_get_resources(Kitchen* kitchen, Vec* requirements) {
  bool available = true;
  Vec* kitchen_resources = kitchen->kitchen_resources;

  pthread_mutex_lock(&kitchen->cook_mtx);

  // Make sure all required resources are available
  for (size_t i = 0; i < requirements->length; i++) {
    Requirement* requirement = vec_at(requirements, i);
    Resource* required_resource = requirement->resource;

    int count_available = 0;
    for (size_t k = 0; k < kitchen_resources->length; k++) {
      KitchenResource* kitchen_resource = vec_at(kitchen_resources, k);

      if (strcmp(kitchen_resource->resource->name, required_resource->name) ==
              0 &&
          kitchen_resource->available) {
        count_available++;
      }
    }

    if (count_available < requirement->quantity) {
      available = false;
      break;
    }
  }

  if (!available) {
    pthread_mutex_unlock(&kitchen->cook_mtx);
    return RESULT_REQUIREMENTS_UNAVAILABLE;
  }

  for (size_t i = 0; i < requirements->length; i++) {
    Requirement* requirement = vec_at(requirements, i);
    Resource* required_resource = requirement->resource;

    int allocated = 0;
    for (size_t k = 0;
         k < kitchen_resources->length && allocated < requirement->quantity;
         k++) {
      KitchenResource* kitchen_resource = vec_at(kitchen_resources, k);

      if (strcmp(kitchen_resource->resource->name, required_resource->name) ==
              0 &&
          kitchen_resource->available) {
        kitchen_resource->available = false;
        allocated++;
      }
    }
  }

  pthread_mutex_unlock(&kitchen->cook_mtx);
  return RESULT_OK;
}

void kitchen_drop_resources(Kitchen* kitchen, Vec* requirements) {
  Vec* kitchen_resources = kitchen->kitchen_resources;

  pthread_mutex_lock(&kitchen->cook_mtx);

  for (size_t i = 0; i < requirements->length; i++) {
    Requirement* requirement = vec_at(requirements, i);
    Resource* resource = requirement->resource;

    int released = 0;
    for (size_t k = 0;
          k < kitchen_resources->length && released < requirement->quantity;
          k++) {
        KitchenResource* kitchen_resource = vec_at(kitchen_resources, k);

        if (strcmp(kitchen_resource->resource->name, resource->name) == 0 &&
            !kitchen_resource->available) {
            kitchen_resource->available = true;
            kitchen_resource->dirtiness++;
            released++;
        }
    }
  }

  pthread_mutex_unlock(&kitchen->cook_mtx);
}

Result kitchen_init(Kitchen* kitchen, Vec* resources) {
  pthread_mutex_init(&kitchen->cook_mtx, nullptr);

  for (size_t i = 0; i < resources->length; i++) {
    Resource* resource = vec_at(resources, i);

    for (int j = 0; j < resource->quantity; j++) {
      KitchenResource kitchen_resource;
      kitchen_resource.resource = resource;
      kitchen_resource.available = true;
      kitchen_resource.dirtiness = 0;

      Result result = vec_push(kitchen->kitchen_resources, &kitchen_resource);

      if (result != RESULT_OK) {
        return result;
      }
    }
  }

  return RESULT_OK;
}

void kitchen_resources_drop(void* arg) {
  if (arg == nullptr) {
    return;
  }

  KitchenResource* kitchen_resource = arg;
  free(kitchen_resource->resource);
}

void kitchen_drop(Kitchen* kitchen) {
  if (kitchen == nullptr) {
    return;
  }

  pthread_mutex_destroy(&kitchen->cook_mtx);
  vec_drop(kitchen->kitchen_resources, kitchen_resources_drop);
}
