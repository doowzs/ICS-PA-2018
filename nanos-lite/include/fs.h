#ifndef __FS_H__
#define __FS_H__

#include "common.h"

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

static size_t file_table_size;
static Finfo file_table[] __attribute__((used));

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

#ifndef SEEK_SET
enum {SEEK_SET, SEEK_CUR, SEEK_END};
#endif

#endif
