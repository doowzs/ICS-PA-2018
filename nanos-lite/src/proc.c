#include "proc.h"

#define MAX_NR_PROC 4

void naive_uload(PCB *, const char *, char* const[], char* const[]);
void context_kload(PCB *pcb, void *entry);
void context_uload(PCB *pcb, const char *filename);

static PCB pcb[MAX_NR_PROC] __attribute__((used));
static PCB pcb_boot;
PCB *current;

void switch_boot_pcb() {
  current = &pcb_boot;
  current = &pcb[0];
  printf("in switch_boot_pcb, the PTR of bootpcb is 0x%08x\n", current->cp->prot->ptr);
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite for the %dth time!", j);
    j ++;
    _yield();
  }
}

void init_proc(const char *filename, char* const argv[], char* const envp[]) {
  Log("special init proc for testing SUCK PA4.2 MMAP!!!");
  context_uload(&pcb[0], "/bin/dummy");
  switch_boot_pcb();
}

_Context* schedule(_Context *prev) {
  current->cp = prev;
  current = &pcb[0];
  //current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
#ifdef SYS_DEBUG
  Log("switching to context at %p, will start at %p", current->cp, current->cp->eip);
#endif
  printf("in schedule, the PTR address of next context is 0x%08x\n", current->cp->prot->ptr);
  return current->cp;
}
