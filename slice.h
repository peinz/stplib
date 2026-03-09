#pragma once

#include "allocator.h"
#include <stddef.h>
#include <string.h>

// [slice]
typedef struct {
  void*  target;
  size_t start;
  size_t length;
} Slice;

Slice slice_make(void* target, size_t start, size_t length) {
  return (Slice){.target = target, .start = start, .length = length};
}

size_t slice_end(const Slice* slice) {
  return slice->start + slice->length - 1;
}

char* slice_to_string(Allocator alc, const Slice* slice) {
  char* str = (char*) alloc(alc, (slice->length + 1) * sizeof(char));
  memcpy(str, (char*)slice->target + slice->start, slice->length);
  str[slice->length] = '\0';
  return str;
}

