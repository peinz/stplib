#pragma once

#include "allocator.h"
#include "generic_list.h"
#include "string.h"
#include <setjmp.h>
#include <stdio.h>
#include <stdbool.h>

// [testing]
typedef TListItem(TestCaseResultItem,
  bool success;
  char* test_name;
  char* message
);
typedef TList(TestCaseResultList, TestCaseResultItem);

typedef struct Testrun {
  Allocator alc;
  TestCaseResultList* results;
  int total_tests;
  int passed_tests;
  jmp_buf test_exit_jump;
  char* current_test_name;
} Testrun;

Testrun* tst_testrun_create(Allocator alc) {
  Testrun* tr = (Testrun*) alloc(alc, sizeof(Testrun));
  tr->alc = alc;
  tr->results = ls_create(TestCaseResultList, alc);
  tr->total_tests = 0;
  tr->passed_tests = 0;
  tr->current_test_name = NULL;
  return tr;
}

void tst_add_result(Testrun* tr, bool success, const char* message) {
  TestCaseResultItem* item = (TestCaseResultItem*) alloc(tr->alc, sizeof(TestCaseResultItem));
  item->success = success;
  item->test_name = tr->current_test_name ? cstr_dup(tr->alc, tr->current_test_name) : cstr_dup(tr->alc, "unknown");
  item->message = cstr_dup(tr->alc, message);
  ls_push(tr->results, item);
}

void tst_print(Testrun* tr) {
  printf("\n=== Test Results ===\n");
  printf("Total: %d tests\n", tr->total_tests);
  printf("Passed: %d tests\n", tr->passed_tests);
  printf("Failed: %d tests\n", tr->total_tests - tr->passed_tests);

  if (tr->total_tests - tr->passed_tests > 0) {
    printf("\nFailures:\n");
    TestCaseResultItem* current = tr->results->start;
    while (current) {
      if (!current->success) {
        printf("  ✗ %s: %s\n", current->test_name, current->message);
      }
      current = current->next;
    }
  }

  printf("\nTest run %s!\n", (tr->passed_tests == tr->total_tests) ? "PASSED" : "FAILED");
}

void tst_run_test(Testrun* tr, const char* test_name, void (*test_func)(Testrun*, Allocator)) {
  tr->current_test_name = (char*)test_name;
  tr->total_tests++;

  printf("Running test: %s... ", test_name);
  fflush(stdout);

  if (setjmp(tr->test_exit_jump) == 0) {
    test_func(tr, tr->alc);
    tr->passed_tests++;
    printf("PASSED\n");
  } else {
    printf("FAILED\n");
  }
}

#define tst_assert(tr, condition, ...) \
  do { \
    if (!(condition)) { \
      char buffer[1024]; \
      snprintf(buffer, sizeof(buffer), "Assertion failed: " #condition " at %s:%d" __VA_OPT__(": ") __VA_ARGS__, __FILE__, __LINE__); \
      tst_add_result((tr), false, buffer); \
      longjmp((tr)->test_exit_jump, 1); \
    } \
  } while (0)

#define tst_assert_eq(tr, actual, expected, ...) \
  do { \
    if ((actual) != (expected)) { \
      char buffer[1024]; \
      snprintf(buffer, sizeof(buffer), "Expected %ld, got %ld at %s:%d" __VA_OPT__(": ") __VA_ARGS__, (long)(expected), (long)(actual), __FILE__, __LINE__); \
      tst_add_result((tr), false, buffer); \
      longjmp((tr)->test_exit_jump, 1); \
    } \
  } while (0)

#define tst_assert_cstr_eq(tr, actual, expected, ...) \
  do { \
    if (strcmp((actual), (expected)) != 0) { \
      char buffer[1024]; \
      snprintf(buffer, sizeof(buffer), "Expected \"%s\", got \"%s\" at %s:%d" __VA_OPT__(": ") __VA_ARGS__, (expected), (actual), __FILE__, __LINE__); \
      tst_add_result((tr), false, buffer); \
      longjmp((tr)->test_exit_jump, 1); \
    } \
  } while (0)

#define tst_assert_null(tr, ptr, ...) \
  do { \
    if ((ptr) != NULL) { \
      char buffer[1024]; \
      snprintf(buffer, sizeof(buffer), "Expected NULL, got %p at %s:%d" __VA_OPT__(": ") __VA_ARGS__, (void*)(ptr), __FILE__, __LINE__); \
      tst_add_result((tr), false, buffer); \
      longjmp((tr)->test_exit_jump, 1); \
    } \
  } while (0)

#define tst_assert_not_null(tr, ptr, ...) \
  do { \
    if ((ptr) == NULL) { \
      char buffer[1024]; \
      snprintf(buffer, sizeof(buffer), "Expected non-NULL, got NULL at %s:%d" __VA_OPT__(": ") __VA_ARGS__, __FILE__, __LINE__); \
      tst_add_result((tr), false, buffer); \
      longjmp((tr)->test_exit_jump, 1); \
    } \
  } while (0)

#define tst_fail(tr, ...) \
  do { \
    char buffer[1024]; \
    snprintf(buffer, sizeof(buffer), "Test failed at %s:%d" __VA_OPT__(": ") __VA_ARGS__, __FILE__, __LINE__); \
    tst_add_result((tr), false, buffer); \
    longjmp((tr)->test_exit_jump, 1); \
  } while (0)

