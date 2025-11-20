#pragma once

#include "allocator.h"
#include "string.h"
#include "log.h"
#include <string.h>
#include <stdio.h>

// [StringBuilder]
typedef struct StringBuilder {
  Allocator alc;
  char* chars;
  size_t count;
  size_t capacity;
} StringBuilder;

#define STRINGBUILDER_INITIAL_CAPACITY 256
StringBuilder sb_create(Allocator alc) {
  return (StringBuilder) {
    .alc = alc,
      .chars = alloc(alc, STRINGBUILDER_INITIAL_CAPACITY * sizeof(char)),
      .count = 0,
      .capacity = STRINGBUILDER_INITIAL_CAPACITY,
  };
}

void sb_realloc(StringBuilder sb, size_t new_capacity) {
  char* new_chars = alloc(sb.alc, new_capacity * sizeof(char));
  memcpy(new_chars, sb.chars, sb.count);
  sb.capacity = new_capacity;
  sb.chars = new_chars;
}

void sb_increase_capacity(StringBuilder* sb, size_t missing_capacity) {
  log_w("sb_increase_capacity: missing_capacity %zu; current_capacity %zu", missing_capacity, sb->capacity);
  int capacity_to_add = sb->capacity;
  if (capacity_to_add == 0) capacity_to_add = missing_capacity;
  while (capacity_to_add < missing_capacity) {
    capacity_to_add *= 2;
  }
  size_t new_capacity = sb->capacity + capacity_to_add;
  log_w("sb_increase_capacity: new_capacity %zu", new_capacity);

  char* new_chars = alloc(sb->alc, new_capacity * sizeof(char));
  memcpy(new_chars, sb->chars, sb->count);
  sb->capacity = new_capacity;
  sb->chars = new_chars;
}

void sb_append_cstr(StringBuilder* sb, const char* new_part) {
  log_v("sb_append_cstr: %s\n", new_part);
  int new_part_len = strlen(new_part);

  if (sb->count + new_part_len >= sb->capacity) {
    int missing_capacity = sb->count + new_part_len - sb->capacity;
    sb_increase_capacity(sb, missing_capacity);
  }

  memcpy(sb->chars + sb->count, new_part, new_part_len);
  sb->count += new_part_len;
}

void sb_append_buf(StringBuilder* sb, const char* new_part, size_t new_part_len) {
  log_v("sb_append_buf: %s\n", new_part);
  if (sb->count + new_part_len >= sb->capacity) {
    int missing_capacity = sb->count + new_part_len - sb->capacity;
    sb_increase_capacity(sb, missing_capacity);
  }

  memcpy(sb->chars + sb->count, new_part, new_part_len);
  sb->count += new_part_len;
}

void sb_append(StringBuilder* sb, String new_part) {
  log_v("sb_append: %.*s\n", (int)new_part.length, new_part.chars);
  sb_append_buf(sb, new_part.chars, new_part.length);
}

#define sb_appendf(sb, fmt, ...) ({                                  \
    log_v("sb_appendf: %s\n", fmt);                                    \
    char buffer[1024];                                                 \
    int len = snprintf(buffer, sizeof(buffer), fmt, ##__VA_ARGS__);    \
    sb_append_buf(sb, buffer, len);                                    \
    })

String* sb_concat(Allocator alc, StringBuilder sb) {
  char* str_chars = alloc(alc, sb.count);
  memcpy(str_chars, sb.chars, sb.count);

  // Create String using the buffer
  String* str = alloc(alc, sizeof(String));
  str->length = sb.count;
  str->chars = str_chars;

  return str;
}

char* sb_concat_cstr(Allocator alc, StringBuilder sb) {
  char* str_chars = alloc(alc, sb.count + 1);
  memcpy(str_chars, sb.chars, sb.count);
  str_chars[sb.count] = '\0';
  return str_chars;
}

