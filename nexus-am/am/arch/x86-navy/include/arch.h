#ifndef __ARCH_H__
#define __ARCH_H__

#include <unistd.h>
#include <sys/types.h>

#define false 0
#define true 1

struct _Context {
  struct _Protect *prot;
  uintptr_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  int       irq;
  uintptr_t err, eip, cs, eflags;
};

#endif
