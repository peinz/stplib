#pragma once

#include "allocator.h"
#include <string.h>
#include <assert.h>

// [Dynamic Arrays with Allocator Support]
#define DA_INIT_CAP 256

// Generic dynamic array type that includes allocator
#define DA_TYPE(Type) \
  struct { \
    Type *items; \
    size_t count; \
    size_t capacity; \
    Allocator alc; \
  }

#define da_init(da, allocator) \
  do { \
    (da)->items = NULL; \
    (da)->count = 0; \
    (da)->capacity = 0; \
    (da)->alc = allocator; \
  } while (0)

#define da_reserve(da, expected_capacity)                                                 \
  do {                                                                                  \
    if ((expected_capacity) > (da)->capacity) {                                       \
      size_t old_capacity = (da)->capacity;                                         \
      if ((da)->capacity == 0) {                                                    \
        (da)->capacity = DA_INIT_CAP;                                             \
      }                                                                             \
      while ((expected_capacity) > (da)->capacity) {                                \
        (da)->capacity *= 2;                                                      \
      }                                                                             \
      void* old_items = (da)->items;                                                \
      (da)->items = (typeof((da)->items)) alloc((da)->alc, (da)->capacity * sizeof(*(da)->items));       \
      assert((da)->items != NULL && "Buy more RAM lol");                            \
      if (old_items && old_capacity > 0) {                                          \
        memcpy((da)->items, old_items, (da)->count * sizeof(*(da)->items));       \
      }                                                                             \
    }                                                                                 \
  } while (0)

#define da_append(da, item)                           \
  do {                                              \
    da_reserve((da), (da)->count + 1);            \
    (da)->items[(da)->count++] = (item);          \
  } while (0)

