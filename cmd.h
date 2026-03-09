#pragma once

#include "allocator.h"
#include "stringbuilder.h"
#include "error.h"
#include <string.h>
#include <stdlib.h>

// [cmd]

// Command structure without nob.h dependencies
typedef struct {
  char **items;
  size_t count;
  size_t capacity;
  Allocator alc;
} Cmd;

// Options for cmd_run_opt() function
typedef struct {
  const char *stdout_path;
  const char *stderr_path;
} Cmd_Opt;

#define cmd_run(err, cmd) cmd_run_opt((err), (cmd), (Cmd_Opt){0})

void cmd_append(Cmd *cmd, const char *item) {
  if (cmd->count >= cmd->capacity) {
    size_t new_capacity = cmd->capacity == 0 ? 8 : cmd->capacity * 2;
    char **new_items = (char**) alloc(cmd->alc, new_capacity * sizeof(char*));
    if (cmd->items) {
      memcpy(new_items, cmd->items, cmd->count * sizeof(char*));
    }
    cmd->items = new_items;
    cmd->capacity = new_capacity;
  }

  size_t item_len = strlen(item);
  char *item_copy = (char*) alloc(cmd->alc, item_len + 1);
  memcpy(item_copy, item, item_len + 1);
  cmd->items[cmd->count++] = item_copy;
}

enum ERR_CMDOPT {
  ERR_CMDOPT_COMMAND_FAILED = 1,
  ERR_CMDOPT_EMPTY_COMMAND,
};

void cmd_run_opt(Error* err, Cmd *cmd, Cmd_Opt opt)
{
  if (!cmd || cmd->count == 0) {
    err_set(err, ERR_CMDOPT_EMPTY_COMMAND, "Command is empty or invalid");
    return;
  }

  // Build command string for system() call
  StringBuilder sb = sb_create(cmd->alc);
  for (size_t i = 0; i < cmd->count; ++i) {
    if (i > 0) sb_append_cstr(&sb, " ");

    // Simple quoting - wrap in quotes if contains spaces
    bool needs_quotes = strchr(cmd->items[i], ' ') != NULL;
    if (needs_quotes) sb_append_cstr(&sb, "\"");
    sb_append_cstr(&sb, cmd->items[i]);
    if (needs_quotes) sb_append_cstr(&sb, "\"");
  }

  // Handle output redirection
  if (opt.stdout_path) {
    sb_append_cstr(&sb, " > ");
    sb_append_cstr(&sb, opt.stdout_path);
  }
  if (opt.stderr_path) {
    sb_append_cstr(&sb, " 2> ");
    sb_append_cstr(&sb, opt.stderr_path);
  }

  // Null terminate the command string
  sb_append_cstr(&sb, "");

  char* command = sb_concat_cstr(cmd->alc, sb);
  log_v("Executing command: %s", command);

  int result = system(command);

  if (result != 0) {
    err_set(err, ERR_CMDOPT_COMMAND_FAILED, "Command failed with exit code %d: %s", result, command);
  }
}

