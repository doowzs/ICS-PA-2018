#ifndef __COMMON_H__
#define __COMMON_H__

//#define DEBUG
//#define EXPR_DEBUG
//#define DIFF_TEST

#if _SHARE
// do not enable these features while building a reference design
#undef DIFF_TEST
#undef DEBUG
#undef EXPR_DEBUG
#endif

/* You will define this macro in PA2 */
#define HAS_IOE

#include <stdint.h>
#include <assert.h>
#include <string.h>

typedef uint8_t bool;

typedef uint32_t rtlreg_t;

typedef uint32_t paddr_t;
typedef uint32_t vaddr_t;

typedef uint16_t ioaddr_t;

volatile static int dummy = 0;

#define false 0
#define true 1

#include "debug.h"
#include "macro.h"

#endif
