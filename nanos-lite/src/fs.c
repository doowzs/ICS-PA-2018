#include "fs.h"

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

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
  
  // set NR_FILES as an variable
  file_table_size = NR_FILES;
  Log("Initializing filesystem... %d files loaded.", file_table_size);
}
