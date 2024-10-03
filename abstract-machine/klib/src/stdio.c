#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <stdio.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  char* tmp = malloc((strlen(fmt)+100)*sizeof(char));
  va_list args;
  va_start(args, fmt);
  sprintf(tmp, fmt, args);
  va_end(args);
  for(int i=0; tmp[i]; ++i){
    putch(tmp[i]);
  }
  return 0;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  // printf("%s\n",fmt);
  // putchar(fmt[0]);
  size_t i = 0;
  char c;
  va_list argptr;
  va_start(argptr, fmt);
  while((c=*(fmt++))){
    if(c=='%'){
      c=*(fmt++);
      switch(c){
        case 'd':
          int tmp = va_arg(argptr, int);
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
        case 'u':
          unsigned int tmpu = va_arg(argptr, unsigned int);
          int cntu = 0;
          unsigned int ttmpu = tmpu;
          while(ttmpu){
            cntu++, ttmpu/=10;
          }
          int cnt2u = 0;
          while(tmpu){
            ++cnt2u;
            out[i+cntu-cnt2u] = tmpu%10 + '0';
            tmpu/=10;
          }
          i+=cntu;
          break;
        case 's':
          char* s = va_arg(argptr, char*);
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
  va_end(argptr);
  return 0;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
