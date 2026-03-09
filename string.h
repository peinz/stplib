#pragma once

#include "allocator.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <alloca.h>

// [string]
typedef struct String {
  char* chars;
  size_t length;
} String;

bool string_equals(String a, String b) {
  return a.length == b.length && memcmp(a.chars, b.chars, a.length) == 0;
}

void str_print(String str) {
  printf("%.*s\n", (int)str.length, str.chars);
}

char* str_tocstr(Allocator alc, String str) {
  char* chars = (char*) alloc(alc, str.length + 1);
  memcpy(chars, str.chars, str.length);
  chars[str.length] = '\0';
  return chars;
}

char* cstr_format(Allocator alc, const char* format, ...) {
  va_list args;
  va_start(args, format);
  int n = vsnprintf(NULL, 0, format, args);
  va_end(args);

  assert(n >= 0);
  char *result = (char*) alloc(alc, n + 1);
  assert(result != NULL && "Extend the size of the temporary allocator");
  va_start(args, format);
  vsnprintf(result, n + 1, format, args);
  va_end(args);

  return result;
}

char* cstr_dup(Allocator alc, const char* cstr) {
  size_t len = strlen(cstr);
  char* chars = (char*) alloc(alc, len + 1);
  memcpy(chars, cstr, len);
  chars[len] = '\0';
  return chars;
}

String str_from_cstr(Allocator alc, const char* cstr) {
  int len = strlen(cstr);
  char* chars = (char*) alloc(alc, len);
  memcpy(chars, cstr, len);
  return (String){.chars = chars, .length = (size_t)len};
}

String str_wrap_cstr(char* cstr) {
  return (String){.chars = cstr, .length = strlen(cstr)};
}

String str_format(Allocator alc, const char* format, ...) {
  va_list args;
  va_start(args, format);
  int n = vsnprintf(NULL, 0, format, args);
  va_end(args);

  assert(n >= 0);
  char *result = (char*) alloc(alc, n + 1);
  assert(result != NULL && "Extend the size of the temporary allocator");
  va_start(args, format);
  vsnprintf(result, n + 1, format, args);
  va_end(args);

  return str_wrap_cstr(result);
}

#define string_alloca_cstring(s) ({                       \
  String _temp_s = (s);                                   \
  char* _temp_cstr = (char*)alloca(_temp_s.length + 1);   \
  if (_temp_cstr) {                                       \
    memcpy(_temp_cstr, _temp_s.chars, _temp_s.length);    \
    _temp_cstr[_temp_s.length] = '\0';                    \
  }                                                       \
  _temp_cstr;                                             \
})

