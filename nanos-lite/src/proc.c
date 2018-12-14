#include "proc.h"

#define MAX_NR_PROC 4

void naive_uload(PCB *, const char *, char* const[], char* const[]);
void context_kload(PCB *pcb, void *entry);
void context_uload(PCB *pcb, const char *filename);

static PCB pcb[MAX_NR_PROC] __attribute__((used));
PCB pcb_boot;
PCB *current;

bool pcb_valid(PCB* pcb) {
  return !(pcb->cp == NULL || pcb->cp->prot == NULL);
}

void switch_boot_pcb() {
  current = &pcb_boot;
  current->cp->prot = &pcb_boot.as;
#ifdef SYS_DEBUG
  Log("switching to BOOT PCB, will start at %p and set CR3 to 0x%08x", current->cp->eip, current->cp->prot->ptr);
#endif
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

  context_uload(&pcb_boot, "/bin/init");
  printf("eip of pcb_boot is 0x%08x\n", pcb_boot.cp->eip);
  switch_boot_pcb();
}

int last_id = 0;
_Context* schedule(_Context *prev) {
  Log("prev's   prot is 0x%08x", prev->prot);
  Log("PCB[0]'s prot is 0x%08x", pcb[0].cp->prot);
  if (prev->prot != NULL) {
    current->cp = prev;
  }

  int i = 0;
  PCB *next_PCB = NULL;

  for ( ; i < MAX_NR_PROC; ++i ) {
    next_PCB = &pcb[(last_id + i + 1) % MAX_NR_PROC];
    if (pcb_valid(next_PCB)) {
      printf("switching to pcb no. %d\n", (last_id + i + 1) % MAX_NR_PROC);
      current = next_PCB;
      break;
    }
  }

  if (i == MAX_NR_PROC) {
    printf("SWITCHING TO BOOT PCB\n");
    switch_boot_pcb();
    return current->cp;
  }

#ifdef SYS_DEBUG
  Log("switching to context at %p, will start at %p and set CR3 to 0x%08x", current->cp, current->cp->eip, current->cp->prot->ptr);
#endif
  current->cp->prot = &next_PCB->as;
  return current->cp;
}

PCB* get_free_pcb() {
  for (int i = 0; i < MAX_NR_PROC; ++i) {
    if (!pcb_valid(&pcb[i])) {
      printf("new process will run on pcb no. %d\n", i);
      return &pcb[i];
    }
  }
  return NULL;
}
