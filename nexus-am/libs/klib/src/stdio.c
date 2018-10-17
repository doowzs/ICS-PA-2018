#include "klib.h"
#include <stdarg.h>
#include <stdbool.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
#define VBUF_MAX_SIZE 128
#define PBUF_MAX_SIZE 1024

/* arg register for arguments */
// TODO: add more types of arguments!
union arg {
  int intarg;
  char *pchararg;
} uarg;

char vbuf[VBUF_MAX_SIZE];
char pbuf[PBUF_MAX_SIZE];

/* print a int to vbuffer zone 
 * and return its start bias */
int vprintf_int(int src, int len, char phchar) {
  vbuf[VBUF_MAX_SIZE - 1] = '\0';
  if (src == 0) {
    vbuf[VBUF_MAX_SIZE - 2] = '0';
    return VBUF_MAX_SIZE - 2;
  } else {
    int pos = VBUF_MAX_SIZE - 1;
    while (src && pos > 0) {
      vbuf[pos] = (src % 10) + '0';
      src /= 10;
      pos--;
      len--;
    }
    while (len > 0 && pos > 0) {
      vbuf[pos] = phchar;
      pos--;
      len--;
    }
    return pos + 1;
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
  int  ret    = 0;     // character counter
  int  len    = 0;     // length of a signle token
  int  bias   = 0;     // bias of vbuf array
  char phchar = ' ';   // place holder character
  int  width  = 0;     // width from format
  //int  prec   = 0;     // precision from format
  bool done   = false; // done scannning an token 

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
      width = 0;
      phchar = ' ';
      done = false;

      while (!done) {
        done = true; // default is one-character long
        switch (*pfmt) {
          case 's':
            uarg.pchararg = va_arg(ap, char*);
            len = strlen(uarg.pchararg);
            strcat(pout, uarg.pchararg);
            break;
          case '0':
            done = false;
            if (width == 0) {
              phchar = '0';
              break;
            }
            // no break if width != 0
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            done = false;
            width = width * 10 + (*pfmt - '0');
            break;
          case 'd':
            uarg.intarg = va_arg(ap, int);
            if (uarg.intarg < 0) {
              strcat(pout, "-");
              ret++;
              pout++;
              uarg.intarg = -uarg.intarg;
            }
            bias = vprintf_int(uarg.intarg, width, phchar);
            len = VBUF_MAX_SIZE - bias - 1;
            strcat(pout, vbuf + bias);
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

  assert(ret < PBUF_MAX_SIZE);
  pbuf[ret] = '\0';
  for (char *s = pbuf; *s; ++s) {
    _putc(*s);
  }
  return ret;
}

#endif
