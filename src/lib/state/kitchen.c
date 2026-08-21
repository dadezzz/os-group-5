#include "kitchen.h"

#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../data/dishes/requirements.h"
#include "../data/resources.h"
#include "../vec.h"

// acquired is used remember exactly which resources were acquired by the cook.
Result kitchen_get_resources(Kitchen* kitchen,
                             Vec* requirements,  // Vec<Requirement>
                             Vec* acquired       // Vec<KitchenResource*>
) {
  vec_init(acquired, sizeof(KitchenResource*));

  bool all_available = true;
  Result result = RESULT_OK;
  Vec* resources = kitchen->resources;

  pthread_mutex_lock(&kitchen->resources_mtx);

  for (size_t r = 0; r < requirements->length; ++r) {
    Requirement* requirement = vec_at(requirements, r);
    int available_count = 0;

    for (size_t k = 0;
         available_count < requirement->quantity && k < resources->length;
         k++) {
      KitchenResource* kitchen_resource = vec_at(resources, k);

      if (kitchen_resource->resource == requirement->resource &&
          kitchen_resource->available) {
        // Store a ref to the kitchen resource, so that we can modify its value
        // from inside acquired.
        vec_push(acquired, &kitchen_resource);
        available_count++;
      }
    }

    if (available_count < requirement->quantity) {
      all_available = false;
      break;
    }
  }

  if (!all_available) {
    vec_drop(acquired, nullptr);
    result = RESULT_REQUIREMENTS_UNAVAILABLE;
  } else {
    // Mark all taken resources as unavailable.
    for (size_t i = 0; i < acquired->length; ++i) {
      KitchenResource** kitchen_resource_ref = vec_at(acquired, i);
      KitchenResource* kitchen_resource = *kitchen_resource_ref;
      kitchen_resource->available = false;
    }
  }

  pthread_mutex_unlock(&kitchen->resources_mtx);

  return result;
}

void kitchen_drop_resources(Kitchen* kitchen,
                            Vec* acquired  // Vec<KitcherResource*>
) {
  pthread_mutex_lock(&kitchen->resources_mtx);

  for (size_t i = 0; i < acquired->length; i++) {
    KitchenResource** kitchen_resource_ref = vec_at(acquired, i);
    KitchenResource* kitchen_resource = *kitchen_resource_ref;
    kitchen_resource->available = true;
    kitchen_resource->dirtiness++;
  }

  pthread_mutex_unlock(&kitchen->resources_mtx);

  vec_drop(acquired, nullptr);
}

Result kitchen_init(Kitchen* kitchen, Vec* resources  // Vec<Resource>
) {
  pthread_mutex_init(&kitchen->resources_mtx, nullptr);

  for (size_t i = 0; i < resources->length; i++) {
    Resource* resource = vec_at(resources, i);

    for (int j = 0; j < resource->quantity; j++) {
      KitchenResource kitchen_resource;
      kitchen_resource.resource = resource;
      kitchen_resource.available = true;
      kitchen_resource.dirtiness = 0;

      Result result = vec_push(kitchen->resources, &kitchen_resource);

      if (result != RESULT_OK) {
        return result;
      }
    }
  }

  return RESULT_OK;
}

void kitchen_drop(Kitchen* kitchen) {
  if (kitchen == nullptr) {
    return;
  }

  pthread_mutex_destroy(&kitchen->resources_mtx);
  vec_drop(kitchen->resources, nullptr);
}
