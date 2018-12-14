#include "proc.h"
#include "memory.h"

static void *pf = NULL;

void* new_page(size_t nr_page) {
  void *p = pf;
  pf += PGSIZE * nr_page;
  assert(pf < (void *)_heap.end);

#ifdef SYS_DEBUG
  Log("new page allocated at %p", p);
#endif

  return p;
}

void free_page(void *p) {
  panic("not implement yet, fix in nanos/src/mm.c");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t new_brk) {
#ifdef SYS_DEBUG
  Log("setting brk from 0x%08x to 0x%08x", current->cur_brk, new_brk);
#endif

  if (new_brk > current->max_brk) {
#ifdef SYS_DEBUG
    Log("new memory is needed!");
#endif

    /* new memory, call newpage and map it */
    int nr_pg = (new_brk - current->max_brk) / PGSIZE + 1;
    void *pa = new_page(nr_pg);
    _map(&current->as, (void *) current->max_brk, pa, nr_pg);

    current->max_brk = new_brk;
  }
  current->cur_brk = new_brk;
  return 0;
}

void init_mm() {
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
  Log("free physical pages starting from %p", pf);

  _vme_init(new_page, free_page);
}
