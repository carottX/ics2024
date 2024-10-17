#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

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

// char* 

int vsprintf(char *out, const char *fmt, va_list ap) {
  // putstr(fmt);
  // assert(0);
  char c;
  size_t i=0;
  bool entered = false;
  char padding = ' ';
  int width = -1;
  
  while((c=*(fmt++))){
    if(c=='%' || entered){
      if(!entered)c=*(fmt++);
      entered = true;
      switch(c){
        case 'd':
          int tmp = va_arg(ap, int);
          int cnt = 0, ttmp = tmp;
          while(ttmp){
            putch(ttmp%10);
            cnt++, ttmp/=10;
          }
          putch('\n');
          if(tmp==0) cnt=1;
          int cnt2 = 0;
          cnt = (width==-1?cnt:(cnt>width?cnt:width));
          for(int ii=0;ii<cnt;++ii) out[i+ii] = padding;
          while(cnt2<cnt){
            ++cnt2;
            out[i+cnt-cnt2] = tmp%10 + '0';
            tmp/=10;
          }
          i+=cnt;
          entered = false;
          padding = ' ';
          width = -1;
          break;
        case 'u':
          uint32_t tmpu = va_arg(ap, uint32_t);
          uint32_t cntu = 0, ttmpu = tmpu;
          while(ttmpu){
            cntu++, ttmpu/=10;
          }
          if(tmpu==0) cntu=1;
          uint32_t cnt2u = 0;
          cntu = (width==-1?cntu:(cntu>width?cntu:width));
          for(int ii=0;ii<cntu;++ii) out[i+ii] = padding;
          while(cnt2u<cntu){
            ++cnt2u;
            out[i+cntu-cnt2u] = tmpu%10 + '0';
            tmpu/=10;
          }
          i+=cntu;
          entered = false;
          padding = ' ';
          width = -1;
          break;
        case 's':
          char* s = va_arg(ap, char*);
          int diff = width-strlen(s);
          for(int ii=0;ii<diff;++ii)out[i+ii] = padding;
          if(diff>0) i+=diff;
          strcpy(out+i, s);
          i += strlen(s);
          entered = false;
          padding = ' ';
          width = -1;
          break;
        case 'c':
          char cc = va_arg(ap, int);
          out[i++] = cc;
          entered = false;
          padding = ' ';
          width = -1;
          break;
        case '%':
          out[i++] = c;
          entered = false;
          padding = ' ';
          width = -1;
          break;
        default:
          if(c=='0') {padding = '0';c=*(fmt++);}
          if(c>='0'&&c<='9'){
            width = 0;
            while(c>='0'&&c<='9'){
              width = width*10+c-'0';
              c=*(fmt++);
            }
            fmt--;
          }
          break;
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
