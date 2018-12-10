#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];
paddr_t page_translate(vaddr_t);

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int mmio_id = is_mmio(addr);
  if (mmio_id != -1) {
    return mmio_read(addr, len, mmio_id);
  } else {
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  }
}

void paddr_write(paddr_t addr, uint32_t data, int len) {
  int mmio_id = is_mmio(addr);
  if (mmio_id != -1) {
    mmio_write(addr, len, data, mmio_id);
  } else {
    memcpy(guest_to_host(addr), &data, len);
  }
}

uint32_t vaddr_read(vaddr_t vaddr, int len) {
  if (false) {// FIXME: how to judge the address exceeds the page boundary???
    //
  } else {
    return paddr_read(page_translate(vaddr), len);
  }
}

void vaddr_write(vaddr_t vaddr, uint32_t data, int len) {
  if (false) { // FIXME: same as above.
    //
  } else {
    paddr_write(page_translate(vaddr), data, len);
  }
}

paddr_t page_translate(vaddr_t vaddr) {
  //FIXME: How to do this???
  return vaddr;
}
