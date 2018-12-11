#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024) // 128MB
#define PAGE_SIZE (4 * 1024) // 4KB for each page

//-----------------------------------------------
// PAGE TABLE HANDY HELPERS!!!
// F*CK USELESS i386 MANUAL!!!
// F*CK NONSENSE 4K ALIGNMENT!
// SEE MANUAL PAGE 92 FOR INFO
#define GET_CR0_PG \
  ((cpu.CR[0] >> 31) & 0x1) // MSB of CR0
#define NEXT_PG(entry, offset) \
  ((entry >> 12) << 12) + (offset << 2)
  // clear lower 12 bits and add offset * SIZE
#define ASSERT_PRESENT(entry, level) \
  Assert(entry & 0x1, "%s is not present in page translation!", level)
//-----------------------------------------------

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
  })

uint8_t pmem[PMEM_SIZE];
paddr_t page_translate(vaddr_t, int);
paddr_t do_page_translate(int, int, int);

// Left/Right 0xff array for separate reading/writing
int lff[5] = { 0x0000000, 0xff000000, 0xffff0000, 0xffffff00, 0xffffffff };
int rff[5] = { 0x0000000, 0x000000ff, 0x0000ffff, 0x00ffffff, 0xffffffff };

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

/**
 * NOTE: the work of checking legality of CR and page,
 * and whether the address exceeds the page boundary,
 * should be done by page_translate!
 *
 * NOTE2: if the address+len exceeds the page bounary,
 * we need to separate the r/w operations so that we 
 * can avoid page fault easily.
 */
uint32_t vaddr_read(vaddr_t vaddr, int len) {
  if ((vaddr & 0xfff) + len <= 4096) {
    return paddr_read(page_translate(vaddr, len), len);
  } else {
    int align = vaddr & 0x3;
    uint32_t upper = vaddr_read((vaddr >> 2) << 2, 4);
    uint32_t lower = vaddr_read(((vaddr >> 2) << 2) + 4, 4);
    uint32_t ret = ((upper & lff[4 - align]) >> (align       << 3))
                 | ((lower & rff[align]    ) << ((4 - align) << 3));
    return ret & rff[len]; // take last len bits
  }
}

void vaddr_write(vaddr_t vaddr, uint32_t data, int len) {
  if ((vaddr & 0xfff) + len <= 4096) {
    paddr_write(page_translate(vaddr, len), data, len);
  } else {
    Log("WARNING, SPLIT WRITE IS NOT TESTED!!!");
    int align = vaddr & 0x3;
    int len1 = 4 - align;  // we must have len + align > 4
    int len2 = len - len1;
    vaddr_write(vaddr, data & rff[len1], len1);
    vaddr_write(((vaddr >> 2) << 2) + 4, data & lff[len2] >> ((4 - len2) << 3), len2); 
  }
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
  if (GET_CR0_PG) {
    /* Paging is on. */
    int dir    = (vaddr >> 22) & 0x3ff; // 22-31: dir
    int page   = (vaddr >> 12) & 0x3ff; // 12-21: page
    int offset =  vaddr        & 0xfff; // 00-11: offset  
    if (offset + len > PAGE_SIZE) {
      panic("Address exceeds page boundary! dir=%d, page=%d, offset=%d, len=%d", dir, page, offset, len);
    } else {
      //printf("translate address 0x%08x\n", vaddr);
      //printf("-> CR3=0x%08x, dir=%d, page=%d, offset=%d\n", cpu.CR[3], dir, page, offset);
      paddr_t paddr = do_page_translate(dir, page, offset);
      //printf("-> result is 0x%08x\n", paddr);
      return paddr;
    }
  } else {
    /* Paging is off. */
    return vaddr;
  }
}

paddr_t do_page_translate(int dir, int page, int offset) {
  paddr_t PDE, PTE;
  PDE = paddr_read(NEXT_PG(cpu.CR[3], dir), 4);
  //printf("-> PDE at 0x%08x, is 0x%08x\n", 
  //    NEXT_PG(cpu.CR[3], dir), PDE);
  if ((PDE & 1) == 0) { printf("error reading address: 0x%08x\n", (dir << 22) + (page << 12) + offset); }
  ASSERT_PRESENT(PDE, "PDE(level 1)");

  PTE = paddr_read(NEXT_PG(PDE, page), 4);
  //printf("-> PTE at 0x%08x, is 0x%08x\n",
  //    NEXT_PG(PDE, page), PTE);
  if ((PTE & 1) == 0) { printf("error reading address: 0x%08x\n", (dir << 22) + (page << 12) + offset); }
  ASSERT_PRESENT(PTE, "PTE(level 2)");

  return ((PTE >> 12) << 12) + offset;
}
