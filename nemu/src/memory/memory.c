#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024) // 128MB
#define PAGE_SIZE (4 * 1024) // 4KB for each page

#define GET_CR0_PG   ((cpu.CR[0] >> 31) & 0x1) // MSB of CR0
#define GET_FRAME_ADDR(entry) ((entry >> 12) & 0xfffff) // 12-31
#define ASSERT_PRESENT(entry, level) \
  Assert(entry & 0x1, "Entry %x of %s is not present in page translation!", entry, level)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
  })

uint8_t pmem[PMEM_SIZE];
paddr_t page_translate(vaddr_t, int);
paddr_t do_page_translate(int, int, int);

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  if (addr > PMEM_SIZE) return vaddr_read(addr, len);
  int mmio_id = is_mmio(addr);
  if (mmio_id != -1) {
    return mmio_read(addr, len, mmio_id);
  } else {
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  }
}

void paddr_write(paddr_t addr, uint32_t data, int len) {
  if (addr > PMEM_SIZE) {
    printf("addr 0x%08x->0x%08x\n", addr, page_translate(addr, len));
    addr = page_translate(addr, len);
    assert(0);
  }
  int mmio_id = is_mmio(addr);
  if (mmio_id != -1) {
    mmio_write(addr, len, data, mmio_id);
  } else {
    memcpy(guest_to_host(addr), &data, len);
  }
}

/**
 * NOTE: the work of checking legality of CR and page,
 * and whether the address exceeds the page boundary,
 * should be done by page_translate!
 */
uint32_t vaddr_read(vaddr_t vaddr, int len) {
  if (vaddr > 0x4000000) Log("addr translate %08x -> %08x", vaddr, page_translate(vaddr, len));
  return paddr_read(page_translate(vaddr, len), len);
}

void vaddr_write(vaddr_t vaddr, uint32_t data, int len) {
  paddr_write(page_translate(vaddr, len), data, len);
}

/**
 * This translate a virtual addr to a real addr.
 * On success (
 *   1 - CR0.PG is invalid (paging is off)
 *   2 - the address does not exceed page boundary
 * ), return paddr. Otherwise throws an error and abort.
 *
 * See i386 Manual Page 98 for debugging memo.
 */
paddr_t page_translate(vaddr_t vaddr, int len) {
  printf("PG status is %d\n", GET_CR0_PG);
  if (GET_CR0_PG) {
    /* Paging is on. */
    int dir    = (vaddr >> 22) & 0x3ff; // 22-31: dir
    int page   = (vaddr >> 12) & 0x3ff; // 12-21: page
    int offset =  vaddr        & 0xfff; // 00-11: offset  
    if (offset + len > PAGE_SIZE) {
      panic("Address exceeds page boundary! dir=%d, page=%d, offset=%d, len=%d", dir, page, offset, len);
    } else {
      return do_page_translate(dir, page, offset);
    }
  } else {
    /* Paging is off. */
    return vaddr;
  }
}

/**
 * All frame addresses start at 12-th bit!
 */
paddr_t do_page_translate(int dir, int page, int offset) {
  printf("in translator!\n");
  paddr_t dir_entry, pg_entry, paddr;
  dir_entry = paddr_read(GET_FRAME_ADDR(cpu.CR[3]), 4) + dir;
  ASSERT_PRESENT(dir_entry, "DIRECTORY");
  pg_entry  = paddr_read(GET_FRAME_ADDR(dir_entry), 4) + page;
  ASSERT_PRESENT(pg_entry, "PAGE TABLE");
  paddr     = paddr_read(GET_FRAME_ADDR(pg_entry),  4) + offset;
  Log("dir=%d -> %x", dir, dir_entry);
  Log("page=%d -> %x", page, pg_entry);
  Log("offset=%d -> %x", offset, paddr);
  return paddr;
}
