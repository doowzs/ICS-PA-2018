#ifndef __ARCH_H__
#define __ARCH_H__

#include <am.h>

#define PMEM_SIZE (128 * 1024 * 1024) // 128MB
#define PAGE_SIZE (4 * 1024)          // 4KB

typedef uint32_t size_t;

struct _Context {
  struct _Protect *prot;
  uintptr_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  int       irq;
  uintptr_t err, eip, cs, eflags;
};

#define GPR1 eax
#define GPR2 esi
#define GPR3 edx
#define GPR4 ecx
#define GPRx eax

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif
