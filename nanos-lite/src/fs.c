#include "fs.h"
#include "ramdisk.h"

size_t serial_write(const void *, size_t, size_t);

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
  {"stdout", 0, 0, 0, invalid_read, serial_write},
  {"stderr", 0, 0, 0, invalid_read, serial_write},
#include "files.h"
  {"/dev/fb", 0, 0, 0, ramdisk_read, ramdisk_write},
  {"/proc/dispinfo", 0, 0, 0, ramdisk_read, ramdisk_write}
};

#define NR_FILES sizeof(file_table) / sizeof(file_table[0])
#define NR_LAST  NR_FILES - 3
#define NR_FB    NR_FILES - 2
#define NR_DISP  NR_FILES - 1

void init_fs() {
  // TODO: initialize the size of /dev/fb
  size_t fb_size = 300 * 400 * 4;
  size_t fb_offset = file_table[NR_LAST].disk_offset + file_table[NR_LAST - 2].size;
  file_table[NR_FB].size = fb_size;
  file_table[NR_FB].disk_offset = fb_offset;

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
  assert(fd >= 0 && fd < NR_FILES);
  size_t offset = file_table[fd].disk_offset + file_table[fd].open_offset;
  size_t delta = file_table[fd].read(buf, offset, len);
  file_table[fd].open_offset += delta;
  return delta;
}

size_t fs_write(int fd, const void *buf, size_t len) {
  assert(fd >= 0 && fd < NR_FILES);
  size_t offset = file_table[fd].disk_offset + file_table[fd].open_offset;
  size_t delta = file_table[fd].write(buf, offset, len);
  file_table[fd].open_offset += delta;
  return delta;
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
