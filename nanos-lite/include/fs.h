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

static Finfo file_table[] __attribute__((used));

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

#ifndef SEEK_SET
enum {SEEK_SET, SEEK_CUR, SEEK_END};
#endif

int fs_open(const char *, int, int);
int fs_close(int);
size_t fs_filesz(int);
size_t fs_read(int, void *, size_t);
size_t fs_write(int, const void *, size_t);
size_t fs_lseek(int, size_t, int);

#endif
