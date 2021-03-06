#include <unistd.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <sys/time.h>
#include <assert.h>
#include <time.h>
#include "syscall.h"

void _fork();
void _wait();

extern char end;
void *brk_old = &end;
void *brk_new = NULL;
void *brk_ret = NULL;

#if defined(__ISA_X86__)
intptr_t _syscall_(int type, intptr_t a0, intptr_t a1, intptr_t a2){
  int ret = -1;
  asm volatile("int $0x80": "=a"(ret): "a"(type), "b"(a0), "c"(a1), "d"(a2));
  return ret;
}
#elif defined(__ISA_AM_NATIVE__)
intptr_t _syscall_(int type, intptr_t a0, intptr_t a1, intptr_t a2){
  intptr_t ret = 0;
  asm volatile("call *0x100000": "=a"(ret): "a"(type), "S"(a0), "d"(a1), "c"(a2));
  return ret;
}
#else
#error _syscall_ is not implemented
#endif

void _exit(int status) {
  _syscall_(SYS_exit, status, 0, 0);
  while (1);
}

int _open(const char *path, int flags, mode_t mode) {
  int ret = _syscall_(SYS_open, (uintptr_t) path, flags, (uintptr_t) mode);
  return ret;
}

int _write(int fd, void *buf, size_t count){
  int ret = _syscall_(SYS_write, fd, (uintptr_t) buf, count);
  return ret;
}

void *_sbrk(intptr_t increment){
  brk_ret = brk_old;
  brk_new = brk_old + increment;
  int ret = _syscall_(SYS_brk, (uintptr_t) brk_new, 0, 0);
  if (ret == 0) {
    brk_old = brk_new;
    return brk_ret;
  } else {
    return (void *) -1;
  }
}

int _read(int fd, void *buf, size_t count) {
  int ret = _syscall_(SYS_read, fd, (uintptr_t) buf, count);
  return ret;
}

int _close(int fd) {
  int ret = _syscall_(SYS_close, fd, 0, 0);
  return ret;
}

off_t _lseek(int fd, off_t offset, int whence) {
  int ret = _syscall_(SYS_lseek, fd, (uintptr_t) offset, whence);
  return ret;
}

int _execve(const char *fname, char * const argv[], char *const envp[]) {
  int ret = _syscall_(SYS_execve, (uintptr_t) fname, (uintptr_t) argv, (uintptr_t) envp);
  return ret;
}

// The code below is not used by Nanos-lite.
// But to pass linking, they are defined as dummy functions

int _fstat(int fd, struct stat *buf) {
  return 0;
}

int _kill(int pid, int sig) {
  _exit(-SYS_kill);
  return -1;
}

pid_t _getpid() {
  _exit(-SYS_getpid);
  return -1; 
}
