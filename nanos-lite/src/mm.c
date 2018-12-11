#include "proc.h"
#include "memory.h"

static void *pf = NULL;

void* new_page(size_t nr_page) {
  void *p = pf;
  pf += PGSIZE * nr_page;
  assert(pf < (void *)_heap.end);
  Log("new page allocated at %p", p);
  return p;
}

void free_page(void *p) {
  panic("not implement yet, fix in nanos/src/mm.c");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t new_brk) {
  if (new_brk > current->max_brk) {
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
