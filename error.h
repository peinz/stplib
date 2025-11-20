#pragma once

#include "allocator.h"
#include "string.h"
#include "log.h"
#include <stdbool.h>
#include <stddef.h>

// [Error Handling]
typedef struct Error {
  Allocator alc;
  size_t code;
  String message;
} Error;

Error err_create(Allocator alc) {
  return (Error){
    .alc = alc,
      .code = 0,
      .message = {0},
  };
}

#define err_set(err, _code, fmt, ...) ({                       \
    if (!err) unreachable("err is null");                       \
    err->code = _code;                                           \
    char* message = cstr_format(err->alc, fmt, ##__VA_ARGS__); \
    err->message = str_wrap_cstr(message);                      \
    })

bool err_isset(Error* err) {
  return err && err->code != 0;
}

String err_serialize(Allocator alc, Error* err) {
  if (!err_isset(err)) return (String){.chars = NULL, .length = 0};
  return err->message;
}

