#pragma once

#include "string.h"
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// [Date]
typedef struct Date {
  int year;
  int month;
  int day;
} Date;

int parse_int(const char* c, int len) {
    int val = 0;
    for (int i = 0; i < len; i++) {
        val = val * 10 + (c[i] - '0');
    }
    return val;
}

Date date_from_ymd_str(String ymd_str) {
  assert(ymd_str.length == 10);
  return (Date){
    .year  = parse_int(ymd_str.chars + 0, 4),
    .month = parse_int(ymd_str.chars + 5, 2),
    .day   = parse_int(ymd_str.chars + 8, 2),
  };
}

Date date_from_dmy_str(String dmy_str) {
  assert(dmy_str.length == 10);
  return (Date){
    .year  = parse_int(dmy_str.chars + 6, 4),
    .month = parse_int(dmy_str.chars + 3, 2),
    .day   = parse_int(dmy_str.chars + 0, 2),
  };
}

String date_to_ymd_str(Allocator alc, Date date) {
  return str_format(alc, "%04d-%02d-%02d", date.year, date.month, date.day);
}

Date date_now(void) {
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  return (Date){
    .year = tm->tm_year + 1900,
    .month = tm->tm_mon + 1,
    .day = tm->tm_mday,
  };
}

bool date_a_is_before_b(Date date_a, Date date_b) {
  if (date_a.year < date_b.year) return true;
  if (date_a.year > date_b.year) return false;
  if (date_a.month < date_b.month) return true;
  if (date_a.month > date_b.month) return false;
  if (date_a.day < date_b.day) return true;
  if (date_a.day > date_b.day) return false;
  // All fields are equal
  return false;
}

bool date_a_is_after_b(Date date_a, Date date_b) {
  if (date_a.year > date_b.year) return true;
  if (date_a.year < date_b.year) return false;
  if (date_a.month > date_b.month) return true;
  if (date_a.month < date_b.month) return false;
  if (date_a.day > date_b.day) return true;
  if (date_a.day < date_b.day) return false;
  // All fields are equal
  return false;
}

bool date_a_is_equal_to_b(Date date_a, Date date_b) {
  return date_a.year == date_b.year &&
    date_a.month == date_b.month &&
    date_a.day == date_b.day;
}

Date date_subtract(Date date_a, Date date_b) {
  struct tm tm = {0};

  tm.tm_year = date_a.year - 1900;
  tm.tm_mon = date_a.month - 1;
  tm.tm_mday = date_a.day;

  time_t date_a_seconds = mktime(&tm);

  // Check for errors (mktime returns -1 on failure)
  if (date_a_seconds == -1) assert(0 && "Error: Could not convert date_a to time\n");

  long seconds_to_subtract =
    date_b.year * 365 * 24 * 60 * 60 +
    date_b.month * 30 * 24 * 60 * 60 +
    date_b.day * 24 * 60 * 60;

  time_t result_time_seconds = date_a_seconds - seconds_to_subtract;
  struct tm* result_date = localtime(&result_time_seconds);

  return (Date){
    .year = result_date->tm_year + 1900,
    .month = result_date->tm_mon + 1,
    .day = result_date->tm_mday,
  };
}

