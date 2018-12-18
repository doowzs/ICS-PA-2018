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
  Log("initializing process of %s...", filename);

  context_uload(&pcb_boot, "/bin/init");
  switch_boot_pcb();
}

int schedule_target = -2;
void key_schedule(int target) {
  //Log("Key schedule to target %d handled.", target);
  schedule_target = target;
  _yield();
}

int last_id = 0;
_Context* last_cp[MAX_NR_PROC];
_Context* schedule(_Context *prev, bool kill) {
#ifdef SYS_DEBUG
  Log("prev's   prot is 0x%08x", prev->prot);
  Log("PCB[0]'s prot is 0x%08x", &pcb[0].as);
  if (kill) Log("KILL THE PROCESS");
#endif

  if (prev->prot != NULL) {
    current->cp = prev;
  }

  /* Handle key event */
  if (schedule_target > -2) {
    Log("Handling key overrided schedule to target %d", schedule_target);
    if (schedule_target == -1) {
      current = &pcb_boot;
      current->cp->prot = &(pcb_boot.as);
    } else {
      assert(schedule_target < MAX_NR_PROC);
      if (pcb_valid(&pcb[schedule_target])) {
        current = &pcb[schedule_target];
        current->cp->prot = &(pcb[schedule_target].as);
      } else {
        Log("Target PCB No.%d is not in use!", schedule_target);
        Log("Press F1 to go back to boot menu and start new processes!");
        Log("Refer to the friendly '171860508.pdf' for detailed infomation!");
      }
    }
    schedule_target = -2;
    return current->cp;
  }

  int i = 0;
  PCB *next_PCB = NULL;

  /* New process detect and boot PCB reservation */
  for (i = 0; i < MAX_NR_PROC; ++i) {
    if (current == &pcb[i]) continue;
    if (last_cp[i] == NULL && pcb[i].cp != NULL) {
      Log("pcb of %d has changed!", i);
      current = &pcb[i];
      current->cp = pcb[i].cp;
      last_cp[i] = pcb[i].cp;
      return current->cp;
    } else if (pcb[i].cp == NULL) last_cp[i] = NULL;
  }
  if (current == &pcb_boot) return current->cp;

  for (i = 0; i < MAX_NR_PROC; ++i) {
    next_PCB = &pcb[(last_id + i + 1) % MAX_NR_PROC];
    if (pcb_valid(next_PCB)) {
      last_id = (last_id + i + 1) % MAX_NR_PROC;
      current = next_PCB;
      break;
    }
  }

  /* Kill the process */
  if (i == MAX_NR_PROC || kill) {
    if (kill) {
      for (i = 0; i < MAX_NR_PROC; ++i) {
        if (prev->prot == &pcb[i].as) {
          current->cp = NULL;
          pcb[i].cp = NULL;
        }
      }
    }
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
#ifdef SYS_DEBUG
      Log("new process will run on PCB No.%d", i);
#endif
      return &pcb[i];
    }
  }
  return NULL;
}
