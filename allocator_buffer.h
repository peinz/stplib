#pragma once

#include "allocator.h"
#include <stddef.h>
#include <assert.h>

// [+buffer-allocator]
typedef struct BufferAllocator {
  size_t capacity;
  size_t cur;
  char* buffer;
} BufferAllocator;

void* __buffer_allocator_alloc(size_t size, void *ctx) {
  BufferAllocator* allocator = (BufferAllocator*)ctx;
  assert(allocator->cur + size <= allocator->capacity && "BufferAllocator out of bounds");
  char* result = allocator->buffer + allocator->cur;
  allocator->cur += size;
  return result;
}

void __buffer_allocator_free(void *ptr, void *ctx) {
  assert(0 && "Freeing from BufferAllocator is not implemented");
}

void __buffer_allocator_clear(void *ctx) {
  BufferAllocator* allocator = (BufferAllocator*)ctx;
  allocator->cur = 0;
}

Allocator buffer_allocator(void* buffer, size_t capacity) {
  assert(capacity > sizeof(BufferAllocator) && "Buffer has to be larger then BufferAllocator");
  BufferAllocator* allocator = (BufferAllocator*)buffer;
  allocator->capacity = capacity;
  allocator->cur = 0;
  allocator->buffer = (char*)buffer + sizeof(BufferAllocator);

  return (Allocator){
    .alloc = __buffer_allocator_alloc,
    .free = __buffer_allocator_free,
    .clear = __buffer_allocator_clear,
    .ctx = buffer,
  };
}

