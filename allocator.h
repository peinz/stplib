#pragma once

#include <stddef.h>

// [allocator]
typedef struct {
  void *(*alloc)(size_t size, void *ctx);
  void (*free)(void * ptr, void *ctx);
  void (*clear)(void *ctx);
  void *ctx;
} Allocator;

void* alloc (Allocator allocator, size_t size) {
  return allocator.alloc(size, allocator.ctx);
}

