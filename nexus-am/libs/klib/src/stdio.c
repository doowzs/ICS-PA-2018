#include "klib.h"
#include <stdarg.h>
#include <stdbool.h>

#ifndef __ISA_NATIVE__

/* arg register for arguments */
// TODO: add more types of arguments!
union arg {
  int intarg;
  char *pchararg;
} uarg;

/* print a int and return its length */
int print_int(char *pout, int src) {
  int ret = 0;      // length counter
  char s[20] = {};  // tmp char array
  while (src) {
    s[ret] = (src % 10) - '0';
    src /= 10;
    ret++;
  }
  for (int i = ret - 1; i >= 0; --i) {
    *pout = s[i];
    pout++;
  }
  return ret;
}

int printf(const char *fmt, ...) {
  assert(0);
  return 0;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  int  ret   = 0;     // character counter
  int  len   = 0;     // length of a signle token
  //int  width = 0;     // width from format
  //int  prec  = 0;     // precision from format
  //char sign  = 0;     // sign prefix
  bool done  = false; // done scannning an token 

  char *pfmt = (char *) fmt, *pout = out; // pointers
  while (*pfmt != '\0') {
    for ( ; *pfmt != '\0' && *pfmt != '%'; ++pfmt) {
      *pout = *pfmt;
      ret++;
    }
    if (*pfmt == '\0') {
      break; // done
    } else {
      pfmt++; // omit '%'
      done = false;

      while (!done) {
        done = true; // default is one-character long
        switch (*pfmt) {
          case 's':
            uarg.pchararg = va_arg(ap, char*);
            len = strlen(uarg.pchararg);
            strcpy(pout, uarg.pchararg);
            break;
          case 'd':
            uarg.intarg = va_arg(ap, int);
            len = print_int(pout, uarg.intarg);
            break;
          default:
            len = 3;
            strcpy(pout, "WTF");
        }
        pfmt++;
        ret += len;
        pout += len;
      }
    }
  }

  return ret;
}

int sprintf(char *out, const char *fmt, ...) {
  int ret = 0;
  va_list ap;

  va_start(ap, fmt);
  ret = vsprintf(out, fmt, ap);
  va_end(ap);
  return ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  assert(0);
  return 0;
}

#endif
