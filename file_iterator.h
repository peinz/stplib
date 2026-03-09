#pragma once

#include "string.h"
#include "error.h"
#include "file.h"
#include <dirent.h>
#include <string.h>

// [file iterator]
typedef enum {
  FILETYPE_UNINITIALIZED = 0,
  FILETYPE_FILE,
  FILETYPE_DIR,
  FILETYPE_UNKNOWN
} FileType;

typedef struct FileEntry {
  String name;
  String ext;
  String path;
  FileType type;
} FileEntry;

typedef struct FileIterator {
  Allocator alc;
  String dir_path;
  DIR* dir;
  FileEntry* cur;
} FileIterator;

FileIterator* file_dir_iter_create(Error* err, Allocator alc, String dir_path) {
  String dir_path_absolute_str = file_get_absolute_path(err, alc, dir_path);
  if (err_isset(err)) return NULL;

  char* dir_path_absolute = string_alloca_cstring(dir_path_absolute_str);
  DIR* dir = opendir(dir_path_absolute);

  if (!dir) {
    err_set(err, 6, "Could not open directory %s", dir_path_absolute);
    return NULL;
  }

  FileIterator* iter = (FileIterator*) alloc(alc, sizeof(FileIterator));
  iter->alc = alc;
  iter->dir_path = dir_path_absolute_str;
  iter->dir = dir;

  FileEntry* cur_file_entry = (FileEntry*) alloc(alc, sizeof(FileEntry));
  cur_file_entry->type = FILETYPE_UNINITIALIZED;
  cur_file_entry->name = (String){.chars = NULL, .length = 0};
  cur_file_entry->ext = (String){.chars = NULL, .length = 0};
  cur_file_entry->path = (String){.chars = NULL, .length = 0};
  iter->cur = cur_file_entry;

  return iter;
}

bool file_dir_iter_next(FileIterator* iter) {
  if (!iter || !iter->dir) return false;

  struct dirent* entry = readdir(iter->dir);
  if (!entry) return false;

  // Skip . and .. entries
  if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
    return file_dir_iter_next(iter);
  }

  // Create FileEntry for current item
  FileEntry* file_entry = iter->cur;

  // Determine file type
  if      (entry->d_type == DT_REG) file_entry->type = FILETYPE_FILE;
  else if (entry->d_type == DT_DIR) file_entry->type = FILETYPE_DIR;
  else                              file_entry->type = FILETYPE_UNKNOWN;

  // set name
  file_entry->name = str_from_cstr(iter->alc, entry->d_name);

  // Build full path
  file_entry->path = str_format(
    iter->alc, "%.*s/%.*s",
    iter->dir_path.length, iter->dir_path.chars,
    file_entry->name.length, file_entry->name.chars
  );

  // extension
  char* ext = strrchr(entry->d_name, '.');
  if (ext) file_entry->ext = str_from_cstr(iter->alc, ext);
  else     file_entry->ext = (String){.chars = NULL, .length = 0};

  return true;
}

void file_dir_iter_close(FileIterator* iter) {
  if (iter && iter->dir) closedir(iter->dir);
}

