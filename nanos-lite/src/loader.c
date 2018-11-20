#include "fs.h"
#include "proc.h"

#define DEFAULT_ENTRY 0x4000000

size_t ramdisk_read(void *, size_t, size_t);
size_t get_ramdisk_size();

static uintptr_t loader(PCB *pcb, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  void *buf = (void *) DEFAULT_ENTRY;
  size_t sz = fs_filesz(fd);
#ifdef SYS_DEBUG
  Log("loading file %s to %p, fd=%d, sz=%d", filename, buf, fd, sz);
#endif
  fs_read(fd, buf, sz);
  return (intptr_t) buf;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Start running at address 0x%08x", entry);
  ((void(*)())entry) ();
}

void context_kload(PCB *pcb, void *entry) {
  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->tf = _kcontext(stack, entry, NULL);
}

void context_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);

  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->tf = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
}
