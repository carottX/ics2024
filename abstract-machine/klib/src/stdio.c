#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <stdio.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static char print_buf[1024];

int printf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vsprintf(print_buf, fmt, args);
  va_end(args);
  for(int i=0; print_buf[i]; ++i){
    putch(print_buf[i]);
  }
  return 0;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  char c;
  size_t i=0;
  while((c=*(fmt++))){
    if(c=='%'){
      c=*(fmt++);
      switch(c){
        case 'd':
          int tmp = va_arg(ap, int);
          int cnt = 0, ttmp = tmp;
          while(ttmp){
            cnt++, ttmp/=10;
          }
          int cnt2 = 0;
          while(tmp){
            ++cnt2;
            out[i+cnt-cnt2] = tmp%10 + '0';
            tmp/=10;
          }
          i+=cnt;
          break;
        case 's':
          char* s = va_arg(ap, char*);
          strcpy(out+i, s);
          i += strlen(s);
          break;
        default:
          return i; //Error!
      }
    }
    else{
      out[i] = c;
      ++i;
    }
  }
  out[i] = '\0';
  return i;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list argptr;
  va_start(argptr, fmt);
  int ret=vsprintf(out, fmt, argptr);
  va_end(argptr);
  return ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
