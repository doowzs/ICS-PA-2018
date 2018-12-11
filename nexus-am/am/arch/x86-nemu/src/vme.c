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

  printf("OK!\n");
  set_cr3(kpdirs);
  printf("OKOK!\n");
  set_cr0(get_cr0() | CR0_PG);
  printf("OKOKOK!\n");
  printf("CR0 is now 0x%08x\n", get_cr0());

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
  return 0;
}

void _unprotect(_Protect *p) {
}

static _Protect *cur_as = NULL;
void get_cur_as(_Context *c) {
  c->prot = cur_as;
}

void _switch(_Context *c) {
  set_cr3(c->prot->ptr);
  cur_as = c->prot;
}

/**
 * NOTE: The 4-th argument size
 * was originally mode. I changed
 * it so that it MAY work.
 */
int _map(_Protect *p, void *va, void *pa, int nr_pg) {
  // make PDE present
  kpdirs[((int) va >> 22) & 0x3ff] = (int) p->ptr | PTE_P;

  // check page does not exceed PDE limit 
  int page = ((int) va >> 12) & 0x3ff;
  if (page + nr_pg >= 4096) {
    printf("PDE boundary exceeded!\n");  
  }

  // fill the PTEs
  int *PTBR = (int *) p->ptr;
  int *PTB = PTBR + page;
  int *PTB_END = PTBR + page + nr_pg;
  int PTBE = (((int) pa >> 12) << 12) | PTE_P;
  for ( ; PTB < PTB_END; PTB++, PTBE += PGSIZE ) {
    *(PTB) = PTBE;
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
  return c;
}
