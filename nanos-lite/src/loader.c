#include "fs.h"
#include "proc.h"

#define DEFAULT_ENTRY 0x8048000

size_t ramdisk_read(void *, size_t, size_t);
size_t get_ramdisk_size();

static uintptr_t loader(PCB *pcb, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  void *buf = (void *) DEFAULT_ENTRY;
  size_t sz = fs_filesz(fd);
#ifdef SYS_DEBUG
  Log("loading file %s to %p, fd=%d, sz=%d", filename, buf, fd, sz);
#endif
  void *pa = new_page(1); // allocate 1 new page
  _map(&pcb->as, buf, pa, 0); // map va to pa
  Log("vaddr %p mapped to %p", buf, pa);
  fs_read(fd, buf, sz);
  Log("file %s loaded!", filename);
  return (intptr_t) buf;
}

void naive_uload(PCB *pcb, const char *filename, char* const argv[], char* const envp[]) {
  uintptr_t entry = loader(pcb, filename);
  Log("Start running at address 0x%08x", entry);

  int argc = 0;
  while(strcmp(argv[argc], "\0") != 0) argc++;
  Log("Count of argumnts: %d", argc);
  for (int i = 0; i < argc; ++i) Log("arg[%d]: %s", i, argv[i]);
  ((void(*)(int, char* const[]))entry) (argc, argv);
}

void context_kload(PCB *pcb, void *entry) {
  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _kcontext(stack, entry, NULL);
}

void context_uload(PCB *pcb, const char *filename) {
  _protect(&pcb->as); // allocate a memory address
  Log("protected a new page directory!");
  uintptr_t entry = loader(pcb, filename);

  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
}
