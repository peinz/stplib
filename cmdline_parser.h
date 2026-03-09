#pragma once

#include "allocator.h"
#include "generic_list.h"
#include "string.h"
#include "error.h"
#include "log.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// [Commandline Argument Parser]

typedef TListItem(CmdArgParserCommandEntryItem, char* name; char* description; int* cmd_parse_target; int cmd_value);
typedef TList(CmdArgParserCommandList, CmdArgParserCommandEntryItem);

typedef TListItem(CmdArgParserArgStringEntryItem, char* name; char* description; String* cmd_parse_target);
typedef TList(CmdArgParserArgStringList, CmdArgParserArgStringEntryItem);

typedef TListItem(CmdArgParserArgBoolEntryItem, char* name; char* description; bool* cmd_parse_target);
typedef TList(CmdArgParserArgBoolList, CmdArgParserArgBoolEntryItem);

typedef TListItem(CmdArgParserSubCommandEntryItem, String value);
typedef TList(CmdArgParserSubCommandList, CmdArgParserSubCommandEntryItem);

typedef struct CmdArgParser {
  Allocator alc;
  CmdArgParserCommandList* commands;
  CmdArgParserArgStringList* arg_strings;
  CmdArgParserArgBoolList* arg_bools;
  CmdArgParserSubCommandList* subcommands;
} CmdArgParser;

CmdArgParser cmd_parser_create(Allocator alc) {
  CmdArgParser parser = {0};
  parser.alc = alc;
  parser.commands = ls_create(CmdArgParserCommandList, alc);
  parser.arg_strings = ls_create(CmdArgParserArgStringList, alc);
  parser.arg_bools = ls_create(CmdArgParserArgBoolList, alc);
  parser.subcommands = ls_create(CmdArgParserSubCommandList, alc);

  return parser;
}

void cmd_parser_add_command(
  CmdArgParser parser,
  int* cmd_parse_target,
  int cmd_value,
  const char* name,
  const char* description
) {
  // log_v("Adding command: %s", cstr_dup(parser.alc, name));
  CmdArgParserCommandEntryItem* entry = (CmdArgParserCommandEntryItem*) alloc(parser.alc, sizeof(CmdArgParserCommandEntryItem));
  entry->name = cstr_dup(parser.alc, name);
  entry->description = cstr_dup(parser.alc, description);
  entry->cmd_parse_target = cmd_parse_target;
  entry->cmd_value = cmd_value;
  ls_push(parser.commands, entry);
  log_v("Added command: %s", entry->name);
}

void cmd_parser_add_arg_string(
  CmdArgParser parser,
  String* cmd_parse_target,
  const char* name,
  const char* description
){
  CmdArgParserArgStringEntryItem* entry = (CmdArgParserArgStringEntryItem*) alloc(parser.alc, sizeof(CmdArgParserArgStringEntryItem));
  entry->name = cstr_dup(parser.alc, name);
  entry->description = cstr_dup(parser.alc, description);
  entry->cmd_parse_target = cmd_parse_target;
  ls_push(parser.arg_strings, entry);
}

void cmd_parser_add_arg_bool(
  CmdArgParser parser,
  bool* cmd_parse_target,
  const char* name,
  const char* description
){
  CmdArgParserArgBoolEntryItem* entry = (CmdArgParserArgBoolEntryItem*) alloc(parser.alc, sizeof(CmdArgParserArgBoolEntryItem));
  entry->name = cstr_dup(parser.alc, name);
  entry->description = cstr_dup(parser.alc, description);
  entry->cmd_parse_target = cmd_parse_target;
  ls_push(parser.arg_bools, entry);
}

void cmd_parser_parse(Error* err, CmdArgParser parser, int argc, char** argv) {
  log_i("Parsing arguments");
  for (int i = 1; i < argc; i++) {
    char* arg = argv[i];
    bool matched = false;

    log_v("Parsing argument: %s", arg);

    if (arg[0] == '-' && arg[1] == '-') {
      // This is a long option
      char* equal_pos = strchr(arg, '=');

      if (equal_pos) {
        // Format: --name=value
        *equal_pos = '\0';
        char* value = equal_pos + 1;

        // Check string arguments
        CmdArgParserArgStringEntryItem* str_entry = parser.arg_strings->start;
        while (str_entry) {
          log_v("Comparing %s to %s", arg, str_entry->name);
          if (strcmp(str_entry->name, arg) == 0) {
            *str_entry->cmd_parse_target = str_from_cstr(parser.alc, value);
            matched = true;
            break;
          }
          str_entry = str_entry->next;
        }

        // Check bool arguments if not matched
        if (!matched) {
          CmdArgParserArgBoolEntryItem* bool_entry = parser.arg_bools->start;
          while (bool_entry) {
            if (strcmp(bool_entry->name, arg) == 0) {
              *bool_entry->cmd_parse_target = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
              matched = true;
              break;
            }
            bool_entry = bool_entry->next;
          }
        }

        *equal_pos = '='; // Restore original string
      } else {
        // Format: --name (boolean flag)
        CmdArgParserArgBoolEntryItem* bool_entry = parser.arg_bools->start;
        while (bool_entry) {
          if (strcmp(bool_entry->name, arg) == 0) {
            *bool_entry->cmd_parse_target = true;
            matched = true;
            break;
          }
          bool_entry = bool_entry->next;
        }
      }
    } else {
      // This is a command or subcommand
      log_v("this is a command or subcommand");
      CmdArgParserCommandEntryItem* cmd_entry = parser.commands->start;
      while (cmd_entry) {
        log_v("Comparing %s to %s", arg, cmd_entry->name);
        if (strcmp(cmd_entry->name, arg) == 0) {
          log_v("command matched");
          *(cmd_entry->cmd_parse_target) = cmd_entry->cmd_value;
          matched = true;
          break;
        }
        cmd_entry = cmd_entry->next;
      }

      // If not a known command, treat it as a subcommand
      if (!matched) {
        log_v("treating as subcommand: %s", arg);
        CmdArgParserSubCommandEntryItem* subcommand_entry = (CmdArgParserSubCommandEntryItem*) alloc(parser.alc, sizeof(CmdArgParserSubCommandEntryItem));
        subcommand_entry->value = str_from_cstr(parser.alc, arg);
        ls_push(parser.subcommands, subcommand_entry);
        matched = true;
      }
    }

    if (!matched) {
      err_set(err, 2, "Unknown argument: %s", arg);
      return;
    }
  }
}

void cmd_parser_print_help(CmdArgParser parser) {
  printf("\nCommands:\n");
  CmdArgParserCommandEntryItem* cmd_entry = parser.commands->start;
  while (cmd_entry) {
    printf("  %-18s %s\n", cmd_entry->name, cmd_entry->description);
    cmd_entry = cmd_entry->next;
  }

  if (parser.arg_strings->count > 0 || parser.arg_bools->count > 0) {
    printf("\nOptions:\n");

    CmdArgParserArgStringEntryItem* str_entry = parser.arg_strings->start;
    while (str_entry) {
      printf("  %-18s %s\n", str_entry->name, str_entry->description);
      str_entry = str_entry->next;
    }

    CmdArgParserArgBoolEntryItem* bool_entry = parser.arg_bools->start;
    while (bool_entry) {
      printf("  %-18s %s\n", bool_entry->name, bool_entry->description);
      bool_entry = bool_entry->next;
    }
  }
}

