#include "common.h"
#include "fs.h"
#include "ramdisk.h"
#include "syscall.h"

#define SYS_DEBUG

extern void *_end;

void syscall_ret(_Context *c, int val) {
  c->GPRx = val;
}

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];

  int i;
  char *pchar;

  a[0] = c->GPR1;
  a[1] = c->ebx;
  a[2] = c->ecx;
  a[3] = c->edx;

  switch (a[0]) {
    /**
     * SYS_exit(0):
     *   exit program.
     */
    case SYS_exit:
      _halt(0);
      break;

    /**
     * SYS_yield(1):
     *   trigger yield event.
     * @return int 0
     */
    case SYS_yield:
      _yield();
      syscall_ret(c, 0);
      break;
      
    /**
     * SYS_open(2):
     *   open a specific file.
     *   on success, return fd.
     *   flags and mode are not used in nemu.
     * @params const char *pathname
     * @params int flags
     * @params int mode
     * @return int fd
     * @throws exception (not found)
     */
    case SYS_open:  
#ifdef SYS_DEBUG
      Log("SYS_open(name=%s, mode=%d, fd=%d)", a[0], a[1], a[2]);
#endif
      pchar = (char *) a[0];
      for (i = 0; i < file_table_size; ++i) {
        if (strcmp(pchar, file_table[i].name) == 0) {
          file_table[i].open_offset = 0;
          syscall_ret(c, i);
          break;
        }
      }
      if (i == file_table_size) {
        /* no match, trigger panic */
        panic("File to be opened not found.");
      }
      break;

    /**
     * SYS_read(3):
     *   read ramdisk into buffer.
     * @params int fd
     * @params void* buf
     * @params size_t len
     * @return size_t
     */
    case SYS_read:
#ifdef SYS_DEBUG
      Log("SYS_read(fd=%d, *buf=%p, len=%d)", a[0], a[1], a[2]);
#endif
      switch (a[1]) {
        case FD_STDIN:  // 0
          /* do nothing in nemu */
          break;
        case FD_STDOUT: // 1
        case FD_STDERR: // 2
          panic("cannot write to STDIN. see nanos/src/syscall.c");
        default:
          assert(a[0] >= 0 && a[0] < file_table_size);
          syscall_ret(c, ramdisk_read((void *)a[1], file_table[a[0]].disk_offset, a[2]));
          break;
      }
      break;


    /**
     * SYS_write(4):
     *   write buffer into ramdisk.
     * @params int fd
     * @params const void* buf
     * @params size_t len
     * @return size_t
     */
    case SYS_write:
#ifdef SYS_DEBUG
      Log("SYS_write(fd=%d, *buf=%p, len=%d)", a[0], a[1], a[2]);
#endif
      switch (a[1]) {
        case FD_STDIN:  // 0
          panic("cannot write to STDIN. see nanos/src/syscall.c");
        case FD_STDOUT: // 1
        case FD_STDERR: // 2
          pchar = (char *) a[2];
          for (int i = 0; i < a[3]; ++i) {
            _putc(*pchar);
            ++pchar;
          }
          syscall_ret(c, a[3]);
          break;
        default:
          assert(a[0] >= 0 && a[0] < file_table_size);
          syscall_ret(c, ramdisk_write((const void*) a[1], file_table[a[0]].disk_offset, a[2]));
          break;
      }
      break;

    /**
     * SYS_close(7):
     *   close a file indicated by fd.
     *   always succeed in nemu.
     * @params int
     * @return int
     */
    case SYS_close:
#ifdef SYS_DEBUG
      Log("SYS_close(fd=%d)", a[0]);
#endif
      syscall_ret(c, 0);
      break;

    /**
     * SYS_lseek(8):
     *   reposition the offset of a specific file.
     *   on success, return the resulting offset.
     * @params int fd
     * @params size_t offset
     * @params int whence
     * @return size_t
     */
    case SYS_lseek:
#ifdef SYS_DEBUG
      Log("SYS_lseek(fd=%d, offset=%d, whence=%d)", a[0], a[1], a[2]);
#endif
      assert(a[0] >= 0 && a[0] < file_table_size);
      file_table[a[0]].open_offset = (size_t) a[1];
      syscall_ret(c, a[1]);
      break;

    /**
     * SYS_brk(9):
     *   set program break.
     *   on success, return 0.
     *   always succeed in nemu.
     * @params const void*
     * @return int
     */
    case SYS_brk:
      _end = (void *) a[1];
      syscall_ret(c, 0);
      break;

    default: panic("Unhandled syscall ID = %d, fix in nanos/src/syscall.c", a[0]);
  }

  return NULL;
}
