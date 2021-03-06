#include <dlfcn.h>

#include "nemu.h"
#include "monitor/monitor.h"
#include "diff-test.h"

static void (*ref_difftest_memcpy_from_dut)(paddr_t dest, void *src, size_t n);
static void (*ref_difftest_getregs)(void *c);
static void (*ref_difftest_setregs)(const void *c);
static void (*ref_difftest_exec)(uint64_t n);

static bool is_skip_ref = false;
static bool is_skip_dut = false;
static bool is_skip_flg = false;
static bool difftest_on =  true;

void difftest_skip_ref() { is_skip_ref = true; }
void difftest_skip_dut() { is_skip_dut = true; }
void difftest_skip_flg() { is_skip_flg = true; }

void init_difftest(char *ref_so_file, long img_size) {
#ifndef DIFF_TEST
  return;
#endif

  assert(ref_so_file != NULL);

  void *handle;
  handle = dlopen(ref_so_file, RTLD_LAZY | RTLD_DEEPBIND);
  assert(handle);

  ref_difftest_memcpy_from_dut = dlsym(handle, "difftest_memcpy_from_dut");
  assert(ref_difftest_memcpy_from_dut);

  ref_difftest_getregs = dlsym(handle, "difftest_getregs");
  assert(ref_difftest_getregs);

  ref_difftest_setregs = dlsym(handle, "difftest_setregs");
  assert(ref_difftest_setregs);

  ref_difftest_exec = dlsym(handle, "difftest_exec");
  assert(ref_difftest_exec);

  void (*ref_difftest_init)(void) = dlsym(handle, "difftest_init");
  assert(ref_difftest_init);

  Log("Differential testing: \33[1;32m%s\33[0m", "ON");
  Log("The result of every instruction will be compared with %s. "
      "This will help you a lot for debugging, but also significantly reduce the performance. "
      "If it is not necessary, you can turn it off in include/common.h.", ref_so_file);

  ref_difftest_init();
  ref_difftest_memcpy_from_dut(ENTRY_START, guest_to_host(ENTRY_START), img_size);
  ref_difftest_setregs(&cpu);
}

void difftest_detach() {
  difftest_on = false;
  Log("Differential testing is turned off.");
}

void difftest_attach() {
  Log("Reloading difftest mem/reg...");

  ref_difftest_memcpy_from_dut(0, guest_to_host(0), PMEM_SIZE + 0x100000);
  ref_difftest_setregs(&cpu);

  difftest_on = true;
  is_skip_ref = false;
  is_skip_dut = false;
  is_skip_flg = false;
  Log("Differential testing is turned on.");
}

void difftest_step(uint32_t eip) {
  if (!difftest_on) return;

  CPU_state ref_r;

  if (is_skip_dut) {
    is_skip_dut = false;
    return;
  }

  if (is_skip_ref) {
    // to skip the checking of an instruction, just copy the reg state to reference design
    ref_difftest_setregs(&cpu);
    is_skip_ref = false;
    return;
  }

  ref_difftest_exec(1);
  ref_difftest_getregs(&ref_r);

  // Check the registers state with the reference design.
  // Set `nemu_state` to `NEMU_ABORT` if they are not the same.
  bool OK = true;
  for (int i = 0; i < 8; ++i) {
    if (cpu.gpr[i]._32 != ref_r.gpr[i]._32) {
      printf("[\33[1;36mDIFF\33[0m] \33[1;31mFAIL\33[0m on %s: NEMU 0x%08x REFR 0x%08x\n", regsl[i], cpu.gpr[i]._32, ref_r.gpr[i]._32);
      OK = false;
    }  
  }
  if (cpu.eip != ref_r.eip) {
    printf("[\33[1;36mDIFF\33[0m] \33[1;31mFAIL\33[0m on %s: NEMU 0x%08x REFR 0x%08x\n", "eip", cpu.eip, ref_r.eip);
    OK = false;
  }

  if (!is_skip_flg) {
    for (int i = 0; i < EFLAGS_SIZE; ++i) {
      uint32_t cpuef = (cpu.eflags32   >> regse_index[i]) & 1;
      uint32_t refef = (ref_r.eflags32 >> regse_index[i]) & 1;
      if (cpuef != refef) {
        printf("[\33[1;36mDIFF\33[0m] \33[1;31mFAIL\33[0m on %s: NEMU 0x%01x REFR 0x%01x\n", regse_upper[i], cpuef, refef);
        OK = false;
      }
    }
    is_skip_flg = false;
  }
  
  if (!OK) {
    nemu_state = NEMU_ABORT;
  }
}
