#pragma once

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <alloca.h>
#include "allocator.h"
#include "error.h"

int str_to_int(Error* err, String str) {
  if (str.length == 0) {
    err_set(err, 1, "Failed to parse: string is empty");
    return -1;
  }

  int result = 0;
  for (int i = 0; i < (int)str.length; i++) {
    int digit = str.chars[i] - '0';

    // Validate character
    if (digit < 0 || digit > 9) {
      err_set(err, 2, "Failed to parse string as integer: %.*s", (int)str.length, str.chars);
      return -1;
    }

    // Check for overflow
    if (result > (INT_MAX - digit) / 10) {
      err_set(err, 3, "Integer overflow parsing: %.*s", (int)str.length, str.chars);
      return -1;
    }

    result = result * 10 + digit;
  }

  return result;
}

