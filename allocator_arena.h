#pragma once

#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#include "allocator.h"
#include "log.h"

// [helper]

static inline uintptr_t align_forward(uintptr_t ptr, size_t align) {
  assert((align & (align - 1)) == 0 && "Alignment must be a power of two"); 
  return (ptr + (align - 1)) & ~(align - 1);
}

#define MAX_SYSTEM_ALIGNMENT sizeof(intmax_t)

// [arena-allocator]

typedef struct Arena {
  void* buffer;
  size_t capacity;
  size_t used;
  size_t alignment;
} Arena;

void __arena_initialize(Arena* arena, size_t initial_capacity, size_t alignment) {
  log_v("initializing arena with capacity %d", initial_capacity);
  if (initial_capacity <= 0) assert(false&&"capacity has to be > 0");
  assert((alignment & (alignment - 1)) == 0 && "Alignment must be a power of two"); 
  arena->buffer = malloc(initial_capacity);
  if (arena->buffer == NULL) assert(false&&"Failed to allocate memory for arena");
  arena->capacity = initial_capacity;
  arena->used = 0;
  arena->alignment = alignment;
}

void __arena_increase_capacity(Arena* arena) {
  if (arena->capacity > SIZE_MAX / 2) assert(false && "Arena capacity overflow on resize");
  size_t new_capacity = arena->capacity * 2;
  void* new_buffer = realloc(arena->buffer, new_capacity);
  if (new_buffer == NULL) assert(false&&"Failed to allocate memory for arena");
  arena->buffer = new_buffer;
  arena->capacity = new_capacity;
}

Arena arena_create_aligned(size_t initial_capacity, size_t alignment) {
  Arena arena = {0};
  __arena_initialize(&arena, initial_capacity, alignment);
  return arena;
}

#define arena_create(capacity) \
  arena_create_aligned(capacity, MAX_SYSTEM_ALIGNMENT)


void* arena_alloc(Arena* arena, size_t size) {
  log_v("arena allocating %zu bytes with alignment %zu", size);

  size_t alignment = arena->alignment;
  assert(alignment > 0 && "Alignment has to be > 0");

  // Ensure alignment is a power of two
  assert((alignment & (alignment - 1)) == 0 && "Alignment must be a power of two");
  
  // Calculate the next aligned pointer
  uintptr_t current_ptr = (uintptr_t)arena->buffer + arena->used;
  uintptr_t aligned_ptr = align_forward(current_ptr, alignment);
  
  size_t offset = aligned_ptr - (uintptr_t)arena->buffer;
  size_t needed_space = offset + size;

  // Check if we need to resize
  while (offset + size > arena->capacity) {
    __arena_increase_capacity(arena);
  }
  
  // Update the used marker and return the aligned pointer
  arena->used = needed_space;
  return (void*)aligned_ptr;
}

void arena_clear(Arena* arena) {
  log_v("clearing arena");
  if (arena->buffer != NULL) free(arena->buffer);
  arena->buffer = NULL;
  arena->capacity = 0;
  arena->used = 0;
}

// [arena-allocator-wrapper]
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

#define arena_allocator_create(initial_capacity) ({ \
  Arena* arena = (Arena*) alloca(sizeof(Arena)); \
  __arena_initialize(arena, initial_capacity, MAX_SYSTEM_ALIGNMENT); \
  Allocator alc = (Allocator){ \
    .alloc = __arena_allocator_alloc, \
    .free = __arena_allocator_free, \
    .clear = __arena_allocator_clear, \
    .ctx = arena, \
  }; \
  alc; \
})

