#include "proc.h"

#define MAX_NR_PROC 4

void naive_uload(PCB *, const char *, char* const[], char* const[]);
void context_kload(PCB *pcb, void *entry);
void context_uload(PCB *pcb, const char *filename);

static PCB pcb[MAX_NR_PROC] __attribute__((used));
static PCB pcb_boot;
PCB *current;

bool pcb_valid(PCB* pcb) {
  return !(pcb->cp == NULL || pcb->cp->prot == NULL);
}

void switch_boot_pcb() {
  current = &pcb_boot;
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

  context_uload(&pcb[0], "/bin/init");
}

_Context* schedule(_Context *prev) {
  current->cp = prev;
  int i = 0, j = 0;
  PCB *next_PCB = NULL;
  for ( ; i < MAX_NR_PROC; ++i ) {
    if (pcb[i].cp == prev) break;
  }
  for ( ; j < MAX_NR_PROC; ++j ) {
    next_PCB = &pcb[(i + j + 1) % MAX_NR_PROC];
    if (pcb_valid(next_PCB)) {
      current = next_PCB;
      break;
    }
  }
#ifdef SYS_DEBUG
  Log("switching to context at %p, will start at %p and set CR3 to 0x%08x", current->cp, current->cp->eip, current->cp->prot->ptr);
#endif
  return current->cp;
}

PCB* get_free_pcb() {
  for (int i = 0; i < MAX_NR_PROC; ++i) {
    if (!pcb_valid(&pcb[i])) {
      return &pcb[i];
    }
  }
  return NULL;
}
