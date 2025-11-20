#pragma once

#include "allocator.h"
#include <stddef.h>

// [Generic List Types]
#define TListItem(Name, ...) \
  struct Name { \
    struct Name* prev; \
    struct Name* next; \
    __VA_ARGS__; \
  } Name

#define TList(Name, ItemType) \
  struct Name { \
    Allocator alc; \
    ItemType* start; \
    ItemType* end; \
    size_t count; \
  } Name

// Generic list operation macros
#define ls_create(ListType, allocator) ({ \
    ListType* list = alloc(allocator, sizeof(ListType)); \
    list->alc = allocator; \
    list->start = NULL; \
    list->end = NULL; \
    list->count = 0; \
    list; \
    })

#define ls_push(list, item) \
  do { \
    if (!(list) || !(item)) break; \
    (item)->next = NULL; \
    (item)->prev = (list)->end; \
    if ((list)->end) { \
      (list)->end->next = (item); \
    } else { \
      (list)->start = (item); \
    } \
    (list)->end = (item); \
    (list)->count++; \
  } while (0)

#define ls_insert(list, index, item) \
  do { \
    if (!(list) || !(item)) break; \
    if ((index) >= (list)->count) { \
      ls_push((list), (item)); \
      break; \
    } \
    if ((index) == 0) { \
      (item)->prev = NULL; \
      (item)->next = (list)->start; \
      if ((list)->start) { \
        (list)->start->prev = (item); \
      } else { \
        (list)->end = (item); \
      } \
      (list)->start = (item); \
    } else { \
      typeof((list)->start) current = (list)->start; \
      for (size_t i = 0; i < (index); i++) { \
        current = current->next; \
      } \
      (item)->prev = current->prev; \
      (item)->next = current; \
      if (current->prev) { \
        current->prev->next = (item); \
      } \
      current->prev = (item); \
    } \
    (list)->count++; \
  } while (0)

#define ls_pop(list) ({ \
    typeof((list)->start) item = NULL; \
    if ((list) && (list)->count > 0) { \
    item = (list)->end; \
    if (item->prev) { \
    item->prev->next = NULL; \
    (list)->end = item->prev; \
    } else { \
    (list)->start = NULL; \
    (list)->end = NULL; \
    } \
    item->prev = NULL; \
    item->next = NULL; \
    (list)->count--; \
    } \
    item; \
    })

#define ls_shift(list) ({ \
    typeof((list)->start) item = NULL; \
    if ((list) && (list)->count > 0) { \
    item = (list)->start; \
    if (item->next) { \
    item->next->prev = NULL; \
    (list)->start = item->next; \
    } else { \
    (list)->start = NULL; \
    (list)->end = NULL; \
    } \
    item->prev = NULL; \
    item->next = NULL; \
    (list)->count--; \
    } \
    item; \
    })

#define ls_at(list, index) ({ \
    typeof((list)->start) current = NULL; \
    if ((list) && (index) < (list)->count) { \
    if ((index) < (list)->count / 2) { \
    current = (list)->start; \
    for (size_t i = 0; i < (index); i++) { \
    current = current->next; \
    } \
    } else { \
    current = (list)->end; \
    for (size_t i = (list)->count - 1; i > (index); i--) { \
    current = current->prev; \
    } \
    } \
    } \
    current; \
    })

#define ls_remove(list, item) \
  do { \
    if (!(list) || !(item)) break; \
    if ((item)->prev) { \
      (item)->prev->next = (item)->next; \
    } else { \
      (list)->start = (item)->next; \
    } \
    if ((item)->next) { \
      (item)->next->prev = (item)->prev; \
    } else { \
      (list)->end = (item)->prev; \
    } \
    (item)->prev = NULL; \
    (item)->next = NULL; \
    (list)->count--; \
  } while (0)

// Generic list item structure for casting
typedef TListItem(GenericListItem, void* data);
typedef TList(GenericList, GenericListItem);

#define ls_remove_duplicates(list, compare_func) \
  do { \
    if (!(list) || (list)->count <= 1) break; \
    GenericList* generic_list = (GenericList*)(list); \
    GenericListItem* current = generic_list->start; \
    while (current) { \
      GenericListItem* next_item = current->next; \
      GenericListItem* check = current->next; \
      while (check) { \
        GenericListItem* check_next = check->next; \
        if (compare_func((void*)current, (void*)check)) { \
          ls_remove(((GenericList*)list), check); \
        } \
        check = check_next; \
      } \
      current = next_item; \
    } \
  } while (0)

// Example usage:
// TListItem(PersonListItem, String name, int age);
// TList(PersonList, PersonListItem);
//
// PersonList* people = ls_create(PersonList, malloc_allocator());
//
// PersonListItem* person = alloc(people->alc, sizeof(PersonListItem));
// person->name = str_from_cstr(people->alc, "John");
// person->age = 30;
// ls_push(people, person);

