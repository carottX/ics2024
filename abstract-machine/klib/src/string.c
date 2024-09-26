#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t i = 0;
  while(s[i]!='\0')i++;
  return i;
}

char *strcpy(char *dst, const char *src) {
  size_t i = 0;
  while(src[i] != '\0') {
    dst[i] = src[i];
    i++;
  }
  dst[i] = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t i = 0;
  while(i<n && src[i] != '\0') {
    dst[i] = src[i];
    i++;
  }
  for(; i<n ;++i){
    dst[i] = '\0';
  }
  return dst;
}

char *strcat(char *dst, const char *src) {
  size_t _offset = 0;
  for(; dst[_offset] != '\0'; ++_offset);
  size_t i = 0;
  for(; src[i] != '\0'; ++i) dst[_offset+i] = src[i];
  dst[_offset+i] = '\0';
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  size_t i = 0;
  for(; s1[i] != '\0' && s2[i] != '\0'; ++i){
    if(s1[i]!=s2[i]) {
      if((unsigned char)s1[i] < (unsigned char)s2[i]) return -1;
      else return 1;
    }
  }
  if(s1[i]=='\0' && s2[i] == '\0') return 0;
  if(s1[i]!='\0') return 1;
  else return -1;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  size_t i = 0;
  for(; s1[i] != '\0' && s2[i] != '\0' && i<n; ++i){
    if(s1[i]!=s2[i]) {
      if((unsigned char)s1[i] < (unsigned char)s2[i]) return -1;
      else return 1;
    }
  }
  if(i==n) return 0;
  if(s1[i] == '\0' && s2[i] != '\0') return -1;
  else if (s1[i] != '\0' && s2[i] == '\0') return 1;
  else{
    assert(0);
    return 0;
  }
}

void *memset(void *s, int c, size_t n) {
  panic("Not implemented");
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
  panic("Not implemented");
}

#endif
