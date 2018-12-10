#ifndef __COMMON_H__
#define __COMMON_H__

#define SYS_DEBUG

#if _SHARE
// do not enable these features while building a reference design
#undef SYS_DEBUG
#endif

/* Uncomment these macros to enable corresponding functionality. */
#define HAS_CTE
#define HAS_VME

#include <am.h>
#include <klib.h>
#include "debug.h"

typedef char bool;
#define true 1
#define false 0

#endif
