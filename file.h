#pragma once

#include "string.h"
#include "error.h"
#include "allocator_buffer.h"
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <alloca.h>

#ifdef _WIN32
#include <windows.h>
#endif

// [file]
String* file_read(Error* err, Allocator alc, String path) {
  char* _path = str_tocstr(alc, path);
  FILE* f = fopen(_path, "rb");
  long long m = 0;
  String* str = NULL;

  if (f == NULL)                 { err_set(err, 1, "Could not open file %s for reading", _path); goto defer; }
  if (fseek(f, 0, SEEK_END) < 0) { err_set(err, 2, "Could not seek to end of file %s", _path); goto defer; }
#ifndef _WIN32
  m = ftell(f);
#else
  m = _ftelli64(f);
#endif
  if (m < 0)                     { err_set(err, 3, "Could not get file size for %s", _path); goto defer; }
  if (fseek(f, 0, SEEK_SET) < 0) { err_set(err, 4, "Could not seek to beginning of file %s", _path); goto defer; }

  str = (String*) alloc(alc, sizeof(String));
  str->length = m;
  str->chars = (char*) alloc(alc, m);

  fread(str->chars, str->length, 1, f);
  if (ferror(f)) {
    err_set(err, 5, "Could not read from file %s", _path);
    goto defer;
  }

  if (f) fclose(f);
  return str;

defer:
  if (f) fclose(f);
  return NULL;
}

void file_write(Error* err, String path, String content) {
  char* scratch = (char*) alloca(sizeof(BufferAllocator) + content.length);
  Allocator alc_scratch = buffer_allocator(scratch, sizeof(BufferAllocator) + content.length);
  char* _path = str_tocstr(alc_scratch, path);
  const char *buf = NULL;
  size_t size;
  FILE *f = fopen(_path, "wb");
  if (f == NULL) {
    err_set(err, 1, "Could not open file %s for writing", _path);
    goto defer;
  }

  size = content.length;
  buf = (const char*)content.chars;
  while (size > 0) {
    size_t n = fwrite(buf, 1, size, f);
    if (ferror(f)) {
      err_set(err, 2, "Could not write into file %s", _path);
      goto defer;
    }
    size -= n;
    buf  += n;
  }

defer:
  if (f) fclose(f);
}

void file_write_cstr(Error* err, String path, char* content) {
  String str_content = str_wrap_cstr(content);
  file_write(err, path, str_content);
}

bool file_exists(String path) {
  char* file_path = string_alloca_cstring(path);

#if _WIN32
    // TODO: distinguish between "does not exists" and other errors
    DWORD dwAttrib = GetFileAttributesA(file_path);
    return dwAttrib != INVALID_FILE_ATTRIBUTES;
#else
    struct stat statbuf;
    if (stat(file_path, &statbuf) < 0) {
        if (errno == ENOENT) return 0;
        log_e("Could not check if file %s exists: %s", file_path, strerror(errno));
        return -1;
    }
    return 1;
#endif
}

void file_mv(Error* err, String from, String to) {
  char* from_path = string_alloca_cstring(from);
  char* to_path = string_alloca_cstring(to);

  if (rename(from_path, to_path) != 0) {
    err_set(err, errno, "Could not move file from %s to %s: %s",
            from_path, to_path, strerror(errno));
  }
}

void file_mkdir(Error* err, String path) {
  char* _path = string_alloca_cstring(path);
  int result = mkdir(_path, 0755);
  if (result != 0) {
    err_set(err, 1, "Could not create archive directory %s", _path);
  }
}

String file_get_absolute_path(Error* err, Allocator alc, String path) {
  char* path_relative = string_alloca_cstring(path);
  char* path_absolute = realpath(path_relative, NULL);

  if (path_absolute == NULL) {
    err_set(err, 55, "Could not get absolute path for %s", path_relative);
    return (String){.chars = NULL, .length = 0};
  }

  String path_absolute_str = str_from_cstr(alc, path_absolute);
  free(path_absolute);
  return path_absolute_str;
}

String file_cwd(Error* err, Allocator alc) {
  char* cwd = getcwd(NULL, 0);
  if (cwd == NULL) {
    err_set(err, errno, "Could not get current working directory: %s", strerror(errno));
    return (String){.chars = NULL, .length = 0};
  }

  String result = str_from_cstr(alc, cwd);
  free(cwd);
  return result;
}

