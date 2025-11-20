#pragma once

#include "allocator.h"
#include <stdlib.h>
#include <assert.h>

// [+malloc-allocator]
void* malloc_allocator_alloc(size_t size, void *ctx) {
  (void)ctx;
  return malloc(size);
}

void malloc_allocator_free(void *ptr, void *ctx) {
  (void)ctx;
  free(ptr);
}

void malloc_allocator_clear(void *ctx) {
  assert(0 && "Clearing the malloc allocator is not supported");
}

Allocator malloc_allocator(void) {
  return (Allocator){
    .alloc = malloc_allocator_alloc,
    .free = malloc_allocator_free,
    .clear = malloc_allocator_clear,
    .ctx = NULL
  };
}

