#include "fs.h"
#include "ramdisk.h"

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here, fix in nanos/irq/fs.c\n READ: buf=%p, offset=%d, len=%d", buf, offset, len);
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here, fix in nanos/irq/fs.c\n WRITE: buf=%p, offset=%d, len=%d", buf, offset, len);
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin", 0, 0, 0, invalid_read, invalid_write},
  {"stdout", 0, 0, 0, invalid_read, invalid_write},
  {"stderr", 0, 0, 0, invalid_read, invalid_write},
#include "files.h"
};

#define NR_FILES sizeof(file_table) / sizeof(file_table[0])

void init_fs() {
  // TODO: initialize the size of /dev/fb
  
  Log("Initializing filesystem... %d files loaded.", NR_FILES);
}

int fs_open(const char *pathname, int flags, int mode){
  // flags and mode are not used in nemu
  for (int i = 0; i < NR_FILES; ++i) {
    if (strcmp(pathname, file_table[i].name) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  /* no match, open failed */
  panic("Invalid pathname %s", pathname);
  return -1;
}

int fs_close(int fd) {
  // always succeed in nemu
  return 0;
}

size_t fs_filesz(int fd) {
  assert(fd >= 0 && fd < NR_FILES);
  return file_table[fd].size;
}

size_t fs_read(int fd, void *buf, size_t len) {
  switch (fd) {
   case FD_STDIN:
    /* do nothing in nemu */
    panic("read from STDIN is not tested! see nanos/src/fs.c");
    return 0;
   case FD_STDOUT:
   case FD_STDERR:
    panic("cannot read from STDOUT/STDERR. see nanos/src/fs.c");
   default:
    assert(fd > 2 && fd < NR_FILES);
    file_table[fd].open_offset = len;
    return ramdisk_read(buf, file_table[fd].disk_offset, len);
  }
  return -1;
}

size_t fs_write(int fd, const void *buf, size_t len) {
  char *pchar;
  switch (fd) {
    case FD_STDIN:
      panic("cannot write to STDIN. see nanos/src/fs.c");
    case FD_STDOUT:
    case FD_STDERR:
      pchar = (char *) buf;
      for (int i = 0; i < len; ++i) {
        _putc(*pchar);
        ++pchar;
      }
      return len;
    default:
      assert(fd > 2 && fd < NR_FILES);
      file_table[fd].open_offset = len;
      return ramdisk_write(buf, file_table[fd].disk_offset, len);
  }
  return -1;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  assert(fd >= 0 && fd < NR_FILES);
  switch (whence) {
    case SEEK_SET:
      file_table[fd].open_offset = offset;
      break;
    case SEEK_CUR:
      file_table[fd].open_offset = file_table[fd].open_offset + offset;
      break;
    case SEEK_END:
      file_table[fd].open_offset = file_table[fd].size + offset;
      break;
    default:
      panic("%d is not a valid whence! see nanos/src/fs.c", whence);
  }
  return file_table[fd].open_offset;
}
