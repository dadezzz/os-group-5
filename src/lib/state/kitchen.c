#include "kitchen.h"

#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>

#include "../data/dishes/requirements.h"
#include "../data/resources.h"
#include "../result.h"
#include "../vec.h"

// acquired is used remember exactly which resources were acquired by the cook.
Result kitchen_get_resources(Kitchen* kitchen,
                             Vec* requirements,  // Vec<Requirement>
                             Vec* acquired       // Vec<KitchenResource*>
) {
  vec_init(acquired, sizeof(KitchenResource*));

  bool all_available = true;
  Result result = RESULT_OK;

  // Needs a lock because we need to acquire multiple resources in a
  // transaction.
  pthread_mutex_lock(&kitchen->mtx);

  for (size_t r = 0; r < requirements->length; ++r) {
    Requirement* requirement = vec_at(requirements, r);
    int available_count = 0;

    for (size_t k = 0; available_count < requirement->quantity &&
                       k < kitchen->resources.length;
         k++) {
      KitchenResource* kitchen_resource = vec_at(&kitchen->resources, k);

      if (kitchen_resource->resource == requirement->resource &&
          atomic_load(&kitchen_resource->available)) {
        // Store a ref to the kitchen resource, so that we can modify its value
        // from inside acquired.
        result = vec_push(acquired, &kitchen_resource);
        if (result != RESULT_OK) {
          vec_drop(acquired, nullptr);
          pthread_mutex_unlock(&kitchen->mtx);
          return result;
        }

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
      atomic_store(&kitchen_resource->available, false);
    }
  }

  pthread_mutex_unlock(&kitchen->mtx);
  return result;
}

void kitchen_drop_resources_dirty(Kitchen* kitchen,
                                  Vec* acquired  // Vec<KitcherResource*>
) {
  pthread_mutex_lock(&kitchen->mtx);

  for (size_t i = 0; i < acquired->length; i++) {
    KitchenResource** kitchen_resource_ref = vec_at(acquired, i);
    KitchenResource* kitchen_resource = *kitchen_resource_ref;
    atomic_store(&kitchen_resource->available, true);
    atomic_fetch_add(&kitchen_resource->dirtiness, 1);
  }

  pthread_mutex_unlock(&kitchen->mtx);

  vec_drop(acquired, nullptr);
}

Result kitchen_init(Kitchen* kitchen, Vec* resources  // Vec<Resource>
) {
  pthread_mutex_init(&kitchen->mtx, nullptr);
  vec_init(&kitchen->resources, sizeof(KitchenResource));

  for (size_t i = 0; i < resources->length; i++) {
    Resource* resource = vec_at(resources, i);

    for (int j = 0; j < resource->quantity; j++) {
      KitchenResource kitchen_resource;
      kitchen_resource.resource = resource;
      atomic_init(&kitchen_resource.available, true);
      atomic_init(&kitchen_resource.dirtiness, 0);

      Result result = vec_push(&kitchen->resources, &kitchen_resource);

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

  pthread_mutex_destroy(&kitchen->mtx);
  vec_drop(&kitchen->resources, nullptr);
}
