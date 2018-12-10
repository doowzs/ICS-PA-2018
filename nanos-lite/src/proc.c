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
  Log("special init proc for testing kContext!");
  context_kload(&pcb[0], (void *)hello_fun);
  context_uload(&pcb[1], "/bin/init");
  switch_boot_pcb();
}

_Context* schedule(_Context *prev) {
  current->cp = prev;
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
#ifdef SYS_DEBUG
  Log("switching to context at %p", current->cp);
#endif
  return current->cp;
}
