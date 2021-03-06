#include "common.h"
#include "fs.h"
#include "proc.h"
#include "ramdisk.h"
#include "syscall.h"

void init_proc(const char *, char* const [], char* const []);
void naive_uload(PCB *pcb, const char *filename, char* const argv[], char* const envp[]);
void context_uload(PCB *pcb, const char *filename);
void switch_boot_pcb();
_Context *schedule(_Context *, bool);
PCB* get_free_pcb();
int mm_brk(uintptr_t);

void syscall_ret(_Context *c, int val) {
  c->GPRx = val;
}

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];

  a[0] = c->GPR1;
  a[1] = c->ebx;
  a[2] = c->ecx;
  a[3] = c->edx;

  switch (a[0]) {
    /**
     * SYS_exit(0):
     *   exit program.
     * @params int
     */
    case SYS_exit:
#ifdef SYS_DEBUG
      Log("SYS_exit(code=%d)", a[1]);
#endif
      if (a[1] == 0) {
        syscall_ret(c, 0);
        return schedule(c, true);
      } else {
        _halt(a[1]);
      }
      break;

    /**
     * SYS_yield(1):
     *   trigger yield event.
     * @return int 0
     */
    case SYS_yield:
#ifdef SYS_DEBUG
      Log("SYS_yield");
#endif
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
      Log("SYS_open(name=%s, flags=%d, mode=%d)", a[1], a[2], a[3]);
#endif
      syscall_ret(c, fs_open((const char *) a[1], a[2], a[3]));
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
      Log("SYS_read(fd=%d, *buf=%p, len=%d)", a[1], a[2], a[3]);
#endif
      syscall_ret(c, fs_read(a[1], (void *) a[2], a[3]));
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
      Log("SYS_write(fd=%d, *buf=%p, len=%d)", a[1], a[2], a[3]);
#endif
      syscall_ret(c, fs_write(a[1], (const void *) a[2], a[3]));
      break;
    
    /**
     * SYS_getpid(6):
     *   return the process ID.
     * @return pid_t
     */
    case SYS_getpid:
#ifdef SYS_DEBUG
      Log("SYS_getpid");
#endif
      // FIXME: how to do this???
      syscall_ret(c, 0);
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
      Log("SYS_close(fd=%d)", a[1]);
#endif
      syscall_ret(c, fs_close(a[1]));
      break;

    /**
     * SYS_lseek(8):
     *   reposition the offset of a specific file.
     *   on success, return the resulting offset.
     * @params int fd
     * @params size_t offset
     * @params int whence
     *   SEEK_SET: set to offset bytes
     *   SEEK_CUR: current location + offset
     *   SEEK_END: size of the file + offset
     * @return size_t
     */
    case SYS_lseek:
#ifdef SYS_DEBUG
      Log("SYS_lseek(fd=%d, offset=%d, whence=%d)", a[1], a[2], a[3]);
#endif
      syscall_ret(c, fs_lseek(a[1], (size_t) a[2], a[3]));
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
      syscall_ret(c, mm_brk(a[1]));
      break;

    /**
     * SYS_execve(13)
     *  execute program
     * @params const char *
     * @params char *const
     * @params char *const
     * @return int
     */
    case SYS_execve:
#ifdef SYS_DEBUG
      Log("SYS_execve(file=%s)", (const char *) a[1]);
#endif
      context_uload(get_free_pcb(), (const char *) a[1]);
      break;

    default: panic("Unhandled syscall ID = %d, fix in nanos/src/syscall.c", a[0]);
  }

  return NULL;
}
