#include <x86.h>
#include <stdio.h>
#include <string.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN;
static PTE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;
static void* (*pgalloc_usr)(size_t);
static void (*pgfree_usr)(void*);

_Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

int _vme_init(void* (*pgalloc_f)(size_t), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  int i;

  // make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }

  PTE *ptab = kptabs;
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE
      kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

      // fill PTE
      PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
      PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;
      for (; pte < pte_end; pte += PGSIZE) {
        *ptab = pte;
        ptab ++;
      }
    }
  }

  set_cr3(kpdirs);
  set_cr0(get_cr0() | CR0_PG);

  return 0;
}

/**
 * Allocate a new page directory entry.
 */
int _protect(_Protect *p) {
  PDE *updir = (PDE*)(pgalloc_usr(1));
  p->pgsize = 4096;
  p->ptr = updir;
  // map kernel space
  for (int i = 0; i < NR_PDE; i ++) {
    updir[i] = kpdirs[i];
  }

  p->area.start = (void*)0x8000000;
  p->area.end = (void*)0xc0000000;

  set_cr3(p->ptr);
  return 0;
}

void _unprotect(_Protect *p) {
}

static _Protect *cur_as = NULL;
void get_cur_as(_Context *c) {
  c->prot = cur_as;
  printf("the address of c->ptr is 0x%08x\n", c->prot->ptr);
}

void _switch(_Context *c) {
  set_cr3(c->prot->ptr);
  cur_as = c->prot;
  printf("CR3 set to 0x%08x\n", c->prot->ptr); 
}

/**
 * NOTE: The 4-th argument size
 * was originally mode. I changed
 * it so that it MAY work.
 */
int _map(_Protect *p, void *va, void *pa, int nr_pg) {
  int dir  = ((int) va >> 22) & 0x3ff;
  int page = ((int) va >> 12) & 0x3ff;
  // check page does not exceed PDE limit 
  if (page + nr_pg >= PGSIZE) {
    printf("PDE boundary exceeded!\n");  
  }

  // map PDE
  int *PDBR = (int *) p->ptr;
  int *PDE = PDBR + dir;
  if (!(*(PDE) & PTE_P)) {
    /** 
     * target page is not present
     * allocate a new page now
     */
    *(PDE) = (((int) pgalloc_usr(1) >> 12) << 12) | PTE_P;
  }
  // map PTE
  int *PTBR = (int *) (*(PDE));
  int *PTB = PTBR + page;
  int *PTB_END = PTBR + page + nr_pg;
  int PTBE = (((int) pa >> 12) << 12) | PTE_P;
  for ( ; PTB < PTB_END; PTBE += PGSIZE ) {
    printf("set 0x%08x to 0x%08x\n", PTB, PTBE);
    *(PTB) = PTBE;
    PTB++;
  }
  return 0;
}

_Context *_ucontext(_Protect *p, _Area ustack, _Area kstack, void *entry, void *args) {
  int argc = 0;
  char *argv = args;
  char **argx = NULL;
  while (strcmp(argv + argc, "\0") != 0) argc++;
  while (argc--) {
    ustack.end -= sizeof(char **);
    argx = (char **) ustack.end;
    *argx = argv + argc;
  }
  printf("the next user entry is %p\n", entry);

  _Context *c = _kcontext(kstack, entry, args);
  c->prot = p;
  printf("the CR3 of this ucontext is 0x%08x\n", p->ptr);
  return c;
}
