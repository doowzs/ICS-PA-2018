#include "klib.h"
#include <stdarg.h>
#include <stdbool.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
#define BUF_MAX_SIZE 1024

/* arg register for arguments */
// TODO: add more types of arguments!
union arg {
  int intarg;
  char *pchararg;
} uarg;

char vbuf[64];
char pbuf[1024];

/* print a int to vbuffer zone 
 * and return its length */
int vprintf_int(int src) {
  if (src == 0) {
    vbuf[0] = '0';
    vbuf[1] = '\0';
    return 1;
  } else {
    int i = 0, pos = 63, len = 0;
    while (src) {
      vbuf[pos] = (src % 10) + '0';
      src /= 10;
      pos--;
      len++;
    }
    for (i = 0; i < len; ++i) {
      vbuf[i] = vbuf[pos + i + 1];
    }
    vbuf[i] = '\0';
    return len;
  }
}

int printf(const char *fmt, ...) {
  int ret = 0;
  va_list ap;

  va_start(ap, fmt);
  ret = vsprintf(pbuf, fmt, ap);
  va_end(ap);

  for (char *s = pbuf; *s; ++s) {
    _putc(*s);
  }
  return ret;
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
    for ( ; *pfmt != '\0' && *pfmt != '%'; ++pfmt, ++ret, ++pout) {
      *pout = *pfmt;
    }
    *pout = '\0'; // mark the end of normal string

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
            strcat(pout, uarg.pchararg);
            break;
          case 'd':
            uarg.intarg = va_arg(ap, int);
            len = vprintf_int(uarg.intarg);
            strcat(pout, vbuf);
            break;
          default:
            len = 30;
            strcat(pout, "implement me at vsprintf \0");
            break;
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
  int ret = 0;
  va_list ap;

  va_start(ap, fmt);
  ret = vsprintf(pbuf, fmt, ap);
  va_end(ap);

  assert(ret < BUF_MAX_SIZE);
  pbuf[ret] = '\0';
  for (char *s = pbuf; *s; ++s) {
    _putc(*s);
  }
  return ret;
}

#endif
