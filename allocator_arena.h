#pragma once

#include "allocator.h"
#include "log.h"
#include "../../dep/arena.h"

// [+arena-allocator-wrapper]
void* __arena_allocator_alloc(size_t size, void *ctx) {
  Arena* arena = (Arena*)ctx;
  return arena_alloc(arena, size);
}

void __arena_allocator_free(void *ptr, void *ctx) {
  log_w("trying to free arena is not implemented; do nothing");
}

void __arena_allocator_clear(void *ctx) {
  Arena* arena = (Arena*)ctx;
  arena_clear(arena);
}

Allocator arena_allocator(Arena* arena) {
  return (Allocator){
    .alloc = __arena_allocator_alloc,
    .free = __arena_allocator_free,
    .clear = __arena_allocator_clear,
    .ctx = arena
  };
}

